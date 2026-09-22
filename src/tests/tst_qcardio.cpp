#include <QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QDataStream>
#include <QFile>
#include <QDir>

#include "Models/directoryvalidator.h"
#include "Models/csv.h"
#include "Models/global_qcardio.h"
#include "Models/resultmatrix.h"
#include "Models/beatkpi.h"
#include "Models/sample.h"
#include "Models/uiconfigs.h"
#include "Services/analyseservice.h"
#include "Services/exportservice.h"
#include "Services/logservice.h"
#include "wfdb/ecgcodes.h"

class TstQCardio : public QObject
{
    Q_OBJECT

private:
    static bool writeTextFile(const QString &path, const QString &contents)
    {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            return false;
        }
        QTextStream out(&file);
        out << contents;
        return out.status() == QTextStream::Ok;
    }

private slots:
    void directoryValidator_rejectsMissingPath()
    {
        QVERIFY(!DirectoryValidator::validateDirectory(QString()));
        QVERIFY(!DirectoryValidator::validateDirectory(QStringLiteral("Z:/qcardio-missing-dir-xyz")));
    }

    void directoryValidator_acceptsExistingDir()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(DirectoryValidator::validateDirectory(dir.path()));
        QVERIFY(DirectoryValidator::validateDirectory(dir.path(), Exists | Readable | IsDirectory | IsAbsolute));
    }

    void directoryValidator_rejectsFileAsDirectory()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString filePath = dir.filePath(QStringLiteral("not-a-dir.txt"));
        QVERIFY(writeTextFile(filePath, QStringLiteral("x")));
        QVERIFY(!DirectoryValidator::validateDirectory(filePath, Exists | IsDirectory));
    }

    void directoryValidator_createsMissingPath()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString nested = dir.filePath(QStringLiteral("a/b/c"));
        QVERIFY(!DirectoryValidator::validateDirectory(nested));
        QVERIFY(DirectoryValidator::validateOrCreateDirectory(nested));
        QVERIFY(DirectoryValidator::validateDirectory(nested));
    }

    void predicting_calcParams_sensitivityAndPrecision()
    {
        Predicting p;
        p.tp = 8;
        p.fn = 2;
        p.fp = 1;
        p.tn = 9;
        p.calcParams();

        QCOMPARE(p.se, 80.0f);
        QCOMPARE(p.p, 100.0f * 8.0f / 9.0f);
        QCOMPARE(p.fpr, 10.0f);
        QCOMPARE(p.rfn, 20.0f);
        QCOMPARE(p.rtp, 80.0f);
    }

    void predicting_calcParams_zeroDenominatorsStayZero()
    {
        Predicting p;
        p.calcParams();
        QCOMPARE(p.se, 0.0f);
        QCOMPARE(p.p, 0.0f);
        QCOMPARE(p.fpr, 0.0f);
    }

    void sheetResult_getDif_andBounds()
    {
        SheetResult res;
        res.sampleIndexList1 = {100, 200};
        res.sampleIndexList2 = {110, 250};
        QCOMPARE(res.getDif(0, 0), 10);
        QCOMPARE(res.getDif(1, 1), 50);
        QCOMPARE(res.getDif(-1, 0), -1);
        QCOMPARE(res.getDif(0, 5), -1);
    }

    void mitBihData_toCSVFormat_skipsNoneBeatIndex()
    {
        MIT_BIH_ECGData data;
        AnnotationData a1;
        a1.time = 10;
        a1.anntyp = NORMAL;
        AnnotationData a2;
        a2.time = 20;
        a2.anntyp = PVC;
        AnnotationData a3;
        a3.time = 30;
        a3.anntyp = NORMAL;
        data.anotList = {a1, a2, a3};
        data.noneBeatIndex = {1};

        const CSVFormat csv = data.toCSVFormat();
        QCOMPARE(csv.sampleIndex.size(), 2);
        QCOMPARE(csv.sampleIndex.at(0), 10);
        QCOMPARE(csv.sampleIndex.at(1), 30);
        QCOMPARE(csv.type.at(0), static_cast<quint8>(NORMAL));
        QCOMPARE(csv.type.at(1), static_cast<quint8>(NORMAL));
    }

    void resultMatrix_insertBeat_countsNormalAndPvc()
    {
        ResultMatrix matrix(MatrixType::BeatType);
        matrix.insertBeat(NORMAL, NORMAL);
        matrix.insertBeat(NORMAL, NORMAL);
        matrix.insertBeat(PVC, PVC);
        QCOMPARE(matrix.totalBeats(), 3);
        QCOMPARE(matrix.getMatrix()[ArrhythmiaType::NormalArr][ArrhythmiaType::NormalArr], 2u);
        QCOMPARE(matrix.getMatrix()[ArrhythmiaType::VentricularArr][ArrhythmiaType::VentricularArr], 1u);
    }

    void beatKpi_perfectNormalAndPvc()
    {
        ResultMatrix matrix(MatrixType::BeatType);
        for (int i = 0; i < 10; ++i) {
            matrix.insertBeat(NORMAL, NORMAL);
        }
        for (int i = 0; i < 4; ++i) {
            matrix.insertBeat(PVC, PVC);
        }

        BeatKPI kpi(matrix);
        kpi.run();
        QCOMPARE(kpi.m_NPrediction.tp, 10);
        QCOMPARE(kpi.m_NPrediction.fn, 0);
        QCOMPARE(kpi.m_NPrediction.fp, 0);
        QCOMPARE(kpi.m_NPrediction.se, 100.0f);
        QCOMPARE(kpi.m_NPrediction.p, 100.0f);
        QCOMPARE(kpi.m_PVCPrediction.tp, 4);
        QCOMPARE(kpi.m_PVCPrediction.se, 100.0f);
        QCOMPARE(kpi.m_PVCPrediction.p, 100.0f);
    }

    void sample_datastream_roundtrip()
    {
        Sample original;
        original.leads[0] = 11;
        original.leads[1] = -22;
        original.leads[2] = 33;
        original.status = 7;
        original.batteryLevel = 90;
        original.leadFailed = 1;

        QByteArray buffer;
        QDataStream out(&buffer, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_15);
        out << original;

        Sample restored;
        QDataStream in(buffer);
        in.setVersion(QDataStream::Qt_5_15);
        in >> restored;

        QCOMPARE(restored.leads[0], original.leads[0]);
        QCOMPARE(restored.leads[1], original.leads[1]);
        QCOMPARE(restored.leads[2], original.leads[2]);
        QCOMPARE(restored.status, original.status);
        QCOMPARE(restored.batteryLevel, original.batteryLevel);
        QCOMPARE(restored.leadFailed, original.leadFailed);
    }

    void uiConfigs_roundtripSetters()
    {
        UIConfigs cfg;
        cfg.setDataBasePath(QStringLiteral("D:/mitdb"));
        cfg.setDataBaseName(QStringLiteral("MIT-BIH"));
        cfg.setLineEditSignal1Index(1);
        cfg.setTargetFreq(178);
        cfg.setGainSignal(1.5f);
        cfg.setExportAll(true);
        cfg.setExportRC7(true);

        QCOMPARE(cfg.dataBasePath(), QStringLiteral("D:/mitdb"));
        QCOMPARE(cfg.dataBaseName(), QStringLiteral("MIT-BIH"));
        QCOMPARE(cfg.lineEditSignal1Index(), 1);
        QCOMPARE(cfg.targetFreq(), 178);
        QCOMPARE(cfg.gainSignal(), 1.5f);
        QVERIFY(cfg.exportAll());
        QVERIFY(cfg.exportRC7());
    }

    void csv_loadAndSave_roundtrip()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString src = dir.filePath(QStringLiteral("in.csv"));
        const QString dst = dir.filePath(QStringLiteral("out.csv"));
        QVERIFY(writeTextFile(src,
                              QStringLiteral("\n"
                                             "100,1\n"
                                             "badline\n"
                                             "250,5\n"
                                             "not,a,number,row\n")));

        CSV loader;
        QVERIFY(loader.loadFromFile(src));
        QCOMPARE(loader.csvFormat().sampleIndex.size(), 2);
        QCOMPARE(loader.csvFormat().sampleIndex.at(0), 100);
        QCOMPARE(loader.csvFormat().type.at(1), static_cast<quint8>(5));

        CSV saver(nullptr, dst, loader.csvFormat());
        saver.saveSignal();

        CSV reloaded;
        QVERIFY(reloaded.loadFromFile(dst));
        QCOMPARE(reloaded.csvFormat().sampleIndex, loader.csvFormat().sampleIndex);
        QCOMPARE(reloaded.csvFormat().type, loader.csvFormat().type);
    }

    void csv_loadFromFile_missingPathFails()
    {
        CSV csv;
        QVERIFY(!csv.loadFromFile(QStringLiteral("Z:/missing-qcardio.csv")));
    }

    void analyseService_emptyPathsFail()
    {
        AnalyseService service;
        QVERIFY(!service.analyse());
    }

    void analyseService_matchingCsvPair_emitsProgressAndWritesReport()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString csv1 = dir.filePath(QStringLiteral("ref.csv"));
        const QString csv2 = dir.filePath(QStringLiteral("det.csv"));
        const QString body = QStringLiteral("100,1\n280,1\n460,5\n");
        QVERIFY(writeTextFile(csv1, body));
        QVERIFY(writeTextFile(csv2, body));

        AnalyseCfg cfg;
        cfg.csvPath1 = {csv1};
        cfg.csvPath2 = {csv2};
        cfg.outputPath = dir.path();
        cfg.sampleRate = 178;

        AnalyseService service(nullptr, cfg);
        QSignalSpy progressSpy(&service, &AnalyseService::sigProgress);
        QSignalSpy logSpy(&service, &AnalyseService::sigAppendLog);

        QVERIFY(service.analyse());
        QCOMPARE(progressSpy.count(), 1);
        QCOMPARE(progressSpy.takeFirst().at(0).toInt(), 1);
        QVERIFY(logSpy.count() >= 1);
        QVERIFY(QFile::exists(dir.filePath(QStringLiteral("Report.csv"))));
        QVERIFY(QFile::exists(dir.filePath(QStringLiteral("refVSdet.csv"))));
    }

    void exportService_rawSample_writesBin()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        MIT_BIH_ECGData data;
        data.dbName = QStringLiteral("MIT-BIH");
        data.filename = QStringLiteral("100");
        data.totalSample = 8;
        data.nsigs = {
            QVector<qreal>{1, 2, 3, 4, 5, 6, 7, 8},
            QVector<qreal>{8, 7, 6, 5, 4, 3, 2, 1},
            QVector<qreal>{0, 0, 0, 0, 0, 0, 0, 0}
        };
        data.selectedLead[0] = 0;
        data.selectedLead[1] = 1;
        data.selectedLead[2] = 2;

        ExprotSetting setting;
        setting.method = ExprotSetting::ExportMethod::RawSample;
        setting.outputPath = dir.path();
        setting.exportCSV = false;

        ExportService exporter;
        QSignalSpy doneSpy(&exporter, &ExportService::sigExportProcessEnd);
        exporter.exportData(data, setting);
        QCOMPARE(doneSpy.count(), 1);
        QVERIFY(QFile::exists(dir.filePath(QStringLiteral("MIT-BIH_100.bin"))));
    }

    void logService_appendsTimestampedText()
    {
        QTextEdit edit;
        LogService log(nullptr, &edit);
        log.sltAppendLog(QStringLiteral("hello-qcardio"));
        QVERIFY(edit.toPlainText().contains(QStringLiteral("hello-qcardio")));
        QVERIFY(edit.toPlainText().contains(QLatin1Char(':')));
    }
};

QTEST_MAIN(TstQCardio)
#include "tst_qcardio.moc"
