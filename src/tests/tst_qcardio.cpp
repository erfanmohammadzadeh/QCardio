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
#include "Models/beatmatcher.h"
#include "Models/ec57metrics.h"
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

    void predicting_calcParams_zeroDenominatorsAreUndefined()
    {
        Predicting p;
        p.calcParams();
        QCOMPARE(p.se, -1.0f);
        QCOMPARE(p.p, -1.0f);
        QCOMPARE(p.fpr, -1.0f);
        QCOMPARE(p.rfn, -1.0f);
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

    void aamiClass_mapsEctopicFusionAndPaced()
    {
        QCOMPARE(aamiClassOf(SVPB), AamiS);
        QCOMPARE(aamiClassOf(ABERR), AamiS);
        QCOMPARE(aamiClassOf(LBBB), AamiN);
        QCOMPARE(aamiClassOf(VESC), AamiV);
        QCOMPARE(aamiClassOf(RONT), AamiV);
        QCOMPARE(aamiClassOf(FUSION), AamiF);
        QCOMPARE(aamiClassOf(PACE), AamiQ);
        QCOMPARE(aamiClassOf(PFUS), AamiQ);
        QCOMPARE(aamiClassOf(RHYTHM), AamiNotQrs);
        QCOMPARE(aamiClassOf(NOISE), AamiNotQrs);
    }

    void ec57_vebExcludesFusionAndUnclassifiableFromFalsePositives()
    {
        quint32 matrix[AamiClassCount][AamiClassCount] = {};
        matrix[AamiV][AamiV] = 8;
        matrix[AamiV][AamiN] = 2;
        matrix[AamiN][AamiV] = 1;
        matrix[AamiF][AamiV] = 5;
        matrix[AamiQ][AamiV] = 4;
        matrix[AamiN][AamiN] = 10;

        const Ec57BeatScores scores = Ec57BeatScores::fromMatrix(matrix);
        QCOMPARE(scores.veb.tp, 8);
        QCOMPARE(scores.veb.fn, 2);
        QCOMPARE(scores.veb.fp, 1);
        QCOMPARE(scores.veb.tn, 10);
        QCOMPARE(scores.veb.se, 80.0f);
        QCOMPARE(scores.veb.p, 100.0f * 8.0f / 9.0f);
        QCOMPARE(scores.veb.fpr, 100.0f * 1.0f / 11.0f);
        QCOMPARE(scores.qrs.tp, 30);
        QCOMPARE(scores.qrs.fn, 0);
        QCOMPARE(scores.qrs.fp, 0);
        QCOMPARE(scores.qrs.se, 100.0f);
        QCOMPARE(scores.qrs.fpr, -1.0f);
    }

    void ec57_svebExcludesUnclassifiableFalsePositives()
    {
        quint32 matrix[AamiClassCount][AamiClassCount] = {};
        matrix[AamiS][AamiS] = 4;
        matrix[AamiN][AamiS] = 1;
        matrix[AamiQ][AamiS] = 7;

        const Ec57BeatScores scores = Ec57BeatScores::fromMatrix(matrix);
        QCOMPARE(scores.sveb.tp, 4);
        QCOMPARE(scores.sveb.fp, 1);
        QCOMPARE(scores.sveb.p, 80.0f);
    }

    void ec57_runSensitivityAndPredictivityUseSeparateNumerators()
    {
        quint32 sensitivity[IecRunBinCount][IecRunBinCount] = {};
        quint32 predictivity[IecRunBinCount][IecRunBinCount] = {};
        sensitivity[2][3] = 2;
        predictivity[3][3] = 2;

        const Ec57RunScores scores = Ec57RunScores::fromMatrices(sensitivity, predictivity);
        QCOMPARE(scores.couplet.tp, 2);
        QCOMPARE(scores.couplet.fn, 0);
        QCOMPARE(scores.couplet.se, 100.0f);
        QCOMPARE(scores.couplet.ppTp, 0);
        QCOMPARE(scores.couplet.p, -1.0f);
        QCOMPARE(scores.shortRun.tp, 0);
        QCOMPARE(scores.shortRun.ppTp, 2);
        QCOMPARE(scores.shortRun.se, -1.0f);
        QCOMPARE(scores.shortRun.p, 100.0f);
    }

    void aggregate_grossWeightsEventsAndAverageIncludesZeroSensitivity()
    {
        AnalyseFileProcessResult aggregate;
        FileProcessResult first;
        first.qrsPredict.tp = 1;
        first.qrsPredict.fn = 1;
        first.qrsPredict.fp = 1;
        first.qrsPredict.calcParams(false);
        first.pvcPredict.tp = 1;
        first.pvcPredict.fp = 1;
        first.pvcPredict.calcParams(true);

        FileProcessResult second;
        second.qrsPredict.tp = 9;
        second.qrsPredict.fn = 1;
        second.qrsPredict.calcParams(false);

        FileProcessResult empty;
        empty.qrsPredict.calcParams(false);

        FileProcessResult missed;
        missed.qrsPredict.tp = 0;
        missed.qrsPredict.fn = 4;
        missed.qrsPredict.calcParams(false);

        aggregate.fileProcessResult = {first, second, empty, missed};
        aggregate.calcParam();

        QCOMPARE(aggregate.qrs.sensitivity.gross, 62.5f);
        QCOMPARE(aggregate.qrs.sensitivity.average, 140.0f / 3.0f);
        QCOMPARE(aggregate.qrs.positivePredictivity.gross, 100.0f * 10.0f / 11.0f);
        QCOMPARE(aggregate.veb.positivePredictivity.average, 50.0f);
        QCOMPARE(aggregate.qrs.positivePredictivity.average, 75.0f);
    }

    void matchBeats_uses150MillisecondWindowAndLearningPeriod()
    {
        const int sampleRate = 178;
        const int window = matchWindowSamples(sampleRate);
        QVERIFY(window >= 26 && window <= 27);

        const QVector<int> referenceSamples = {100, 1000, 2000, 3000};
        const QVector<quint8> referenceTypes = {1, 1, 5, 1};
        const QVector<int> testSamples = {110, 1010, 2005, 4000};
        const QVector<quint8> testTypes = {1, 1, 1, 5};

        const BeatMatchResult scored = matchBeats(referenceSamples, referenceTypes,
                                                  testSamples, testTypes,
                                                  sampleRate, 500);
        QCOMPARE(scored.referenceBeats.size(), 3);
        QCOMPARE(scored.pairs.size(), 4);

        int truePositives = 0;
        int falseNegatives = 0;
        int falsePositives = 0;
        for (const AlignedPair &pair : scored.pairs) {
            if (pair.refType >= 0 && pair.testType >= 0)
                ++truePositives;
            else if (pair.refType >= 0)
                ++falseNegatives;
            else
                ++falsePositives;
        }
        QCOMPARE(truePositives, 2);
        QCOMPARE(falseNegatives, 1);
        QCOMPARE(falsePositives, 1);
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
        QVERIFY(!service.run());
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

        QVERIFY(service.run());
        QCOMPARE(progressSpy.count(), 1);
        QCOMPARE(progressSpy.takeFirst().at(0).toInt(), 1);
        QVERIFY(logSpy.count() >= 1);
        QVERIFY(QFile::exists(dir.filePath(QStringLiteral("Report.csv"))));
        QVERIFY(QFile::exists(dir.filePath(QStringLiteral("Report_shutdown.csv"))));
        QVERIFY(QFile::exists(dir.filePath(QStringLiteral("Report_runs.csv"))));
        QVERIFY(QFile::exists(dir.filePath(QStringLiteral("refVSdet.csv"))));

        QFile report(dir.filePath(QStringLiteral("Report.csv")));
        QVERIFY(report.open(QIODevice::ReadOnly | QIODevice::Text));
        const QString reportText = QString::fromUtf8(report.readAll());
        QVERIFY(reportText.contains(QStringLiteral("QRS_Se")));
        QVERIFY(reportText.contains(QStringLiteral("PVC_Se")));
        QVERIFY(reportText.contains(QStringLiteral("Normal_Se")));
        QVERIFY(!reportText.contains(QStringLiteral("Fusion_Se")));
        QVERIFY(reportText.contains(QStringLiteral("Sensitivity gross")));
        QVERIFY(reportText.contains(QStringLiteral("100.00")));
    }

    void beatProfile_foldsUnselectedSubtypeIntoItsGroup()
    {
        const BeatTestProfile grouped;
        QCOMPARE(resolveTestClass(FUSION, grouped), ClassPvc);
        QCOMPARE(resolveTestClass(VESC, grouped), ClassPvc);
        QCOMPARE(resolveTestClass(kInterpolatedAnnotation, grouped), ClassPvc);
        QCOMPARE(resolveTestClass(LBBB, grouped), ClassNormal);
        QCOMPARE(resolveTestClass(SVPB, grouped), ClassNormal);
        QCOMPARE(resolveTestClass(UNKNOWN, grouped), ClassUnknown);

        quint32 matrix[TestBeatClassCount][TestBeatClassCount] = {};
        accumulateTestBeat(matrix, FUSION, PVC, grouped);
        const TestClassCounts pvc = scoreTestClass(matrix, ClassPvc, grouped.activeClasses());
        QCOMPARE(pvc.tp, 1);
        QCOMPARE(pvc.fn, 0);
        QCOMPARE(pvc.fp, 0);

        BeatTestProfile separate = grouped;
        separate.separateFusion = true;
        separate.separatePvc = true;
        quint32 split[TestBeatClassCount][TestBeatClassCount] = {};
        accumulateTestBeat(split, FUSION, PVC, separate);
        const QVector<int> active = separate.activeClasses();
        QVERIFY(active.contains(ClassFusion));
        const TestClassCounts fusion = scoreTestClass(split, ClassFusion, active);
        const TestClassCounts pvcSplit = scoreTestClass(split, ClassPvc, active);
        QCOMPARE(fusion.tp, 0);
        QCOMPARE(fusion.fn, 1);
        QCOMPARE(pvcSplit.fp, 1);
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
