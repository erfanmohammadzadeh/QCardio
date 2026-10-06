#include "sheetanalyser.h"
#include "Models/beatmatcher.h"
#include "Models/ec57metrics.h"

#include <QDebug>

#include <cstring>

SheetAnalyser::SheetAnalyser(QObject *parent,
                             QStringList sheetPathList,
                             AnalyseCfg analyseCfg,
                             QString processFileName)
    : QObject(parent)
    , m_sheetPathList(std::move(sheetPathList))
    , m_outputPath(std::move(analyseCfg.outputPath))
    , m_processFileName(processFileName)
    , m_analyseCfg(analyseCfg)
{
}

bool SheetAnalyser::processSheets()
{
    if (m_sheetPathList.size() < 2) {
        qWarning() << "Compare requires two CSV files";
        return false;
    }
    if (!loadCSVData()) {
        QMessageBox::critical(nullptr, "Load Error", "Load data from csv file failed.");
        return false;
    }
    if (!CompareSampleIdxAndType()) {
        QMessageBox::critical(nullptr, "Compare Error", "Compare process failed.");
        return false;
    }
    if (!saveResult()) {
        QMessageBox::critical(nullptr, "Save Error", "Save process failed.");
    }
    return true;
}

FileProcessResult SheetAnalyser::fileProcessRes() const
{
    return m_fileProcessRes;
}

bool SheetAnalyser::CompareSampleIdxAndType()
{
    if (m_sheetRes.sampleIndexList1.isEmpty() || m_sheetRes.sampleIndexList2.isEmpty()) {
        qDebug() << "Empty sample lists!";
        return false;
    }

    m_fileProcessRes.clear();
    m_fileProcessRes.fileName = m_processFileName;
    m_fileProcessRes.compareOfset = m_analyseCfg.compareOfset;

    const BeatMatchResult matched = matchBeats(m_sheetRes.sampleIndexList1,
                                               m_sheetRes.typeList1,
                                               m_sheetRes.sampleIndexList2,
                                               m_sheetRes.typeList2,
                                               m_analyseCfg.sampleRate,
                                               m_analyseCfg.compareOfset);

    for (const AlignedPair &pair : matched.pairs) {
        m_sheetRes.alignedList1.append(pair.refSample);
        m_sheetRes.alignedList2.append(pair.testSample);
        m_sheetRes.difIndexList.append(pair.sampleDelta);
        m_sheetRes.typeAlignList1.append(pair.refType);
        m_sheetRes.typeAlignList2.append(pair.testType);
        m_sheetRes.difTypeList.append(pair.typeMatch);
        m_beatTypeMap.addComparison(pair.refType, pair.testType);

        if (pair.refType >= 0 && pair.testType < 0) {
            ++m_fileProcessRes.missedBeatCount;
            const AamiClass missed = aamiClassOf(pair.refType);
            if (missed == AamiN)
                ++m_fileProcessRes.normalMissed;
            else if (missed == AamiV)
                ++m_fileProcessRes.pvcMissed;
        }
    }

    quint32 beatMatrix[AamiClassCount][AamiClassCount];
    m_beatTypeMap.copyAami(beatMatrix);
    const Ec57BeatScores beatScores = Ec57BeatScores::fromMatrix(beatMatrix);
    m_fileProcessRes.qrsPredict = beatScores.qrs;
    m_fileProcessRes.normalPredict = beatScores.normal;
    m_fileProcessRes.pvcPredict = beatScores.veb;
    m_fileProcessRes.svtPredict = beatScores.sveb;

    m_fileProcessRes.profile = m_analyseCfg.profile;
    const QVector<int> activeClasses = m_analyseCfg.profile.activeClasses();
    for (const AlignedPair &pair : matched.pairs)
        accumulateTestBeat(m_fileProcessRes.testBeat, pair.refType, pair.testType, m_analyseCfg.profile);
    for (int beatClass : activeClasses) {
        const TestClassCounts counts = scoreTestClass(m_fileProcessRes.testBeat, beatClass, activeClasses);
        Predicting &stat = m_fileProcessRes.classPredict[beatClass];
        stat.tp = counts.tp;
        stat.fn = counts.fn;
        stat.fp = counts.fp;
        stat.tn = counts.tn;
        stat.calcParams(true);
    }
    m_beatTypeMap.copyAami(m_fileProcessRes.aamiBeat);
    std::memcpy(m_fileProcessRes.beatTypeMap, m_beatTypeMap.getMatrix(), sizeof(m_fileProcessRes.beatTypeMap));

    fillRunMatrices(matched.referenceBeats,
                    matched.testBeats,
                    m_analyseCfg.sampleRate,
                    true,
                    m_fileProcessRes.vRunSensitivity,
                    m_fileProcessRes.vRunPredictivity);
    fillRunMatrices(matched.referenceBeats,
                    matched.testBeats,
                    m_analyseCfg.sampleRate,
                    false,
                    m_fileProcessRes.sRunSensitivity,
                    m_fileProcessRes.sRunPredictivity);

    const Ec57RunScores ventricularRuns = Ec57RunScores::fromMatrices(m_fileProcessRes.vRunSensitivity,
                                                                      m_fileProcessRes.vRunPredictivity);
    const Ec57RunScores supraventricularRuns = Ec57RunScores::fromMatrices(m_fileProcessRes.sRunSensitivity,
                                                                           m_fileProcessRes.sRunPredictivity);
    m_fileProcessRes.pvc_couplet = ventricularRuns.couplet;
    m_fileProcessRes.pvc_shortRun = ventricularRuns.shortRun;
    m_fileProcessRes.pvc_longRun = ventricularRuns.longRun;
    m_fileProcessRes.svt_couplet = supraventricularRuns.couplet;
    m_fileProcessRes.svt_shortRun = supraventricularRuns.shortRun;
    m_fileProcessRes.svt_longRun = supraventricularRuns.longRun;

    m_fileProcessRes.totalBeat = matched.referenceBeats.size();
    m_fileProcessRes.totalShutdown = QTime(0, 0, 0);
    return true;
}

bool SheetAnalyser::saveResult()
{
    const QString outCsv = m_outputPath + QStringLiteral("/%1.csv").arg(m_processFileName);
    CSV compareOut(nullptr, outCsv);
    return compareOut.saveRawCSVRes(m_sheetRes);
}

bool SheetAnalyser::loadCSVData()
{
    CSV csv1(nullptr, m_sheetPathList[0]);
    CSV csv2(nullptr, m_sheetPathList[1]);

    if (!csv1.loadFromFile(m_sheetPathList[0]) || !csv2.loadFromFile(m_sheetPathList[1])) {
        qWarning() << "Failed to load CSV files";
        return false;
    }
    m_sheetRes.sampleIndexList1 = csv1.csvFormat().sampleIndex;
    m_sheetRes.typeList1 = csv1.csvFormat().type;
    m_sheetRes.sampleIndexList2 = csv2.csvFormat().sampleIndex;
    m_sheetRes.typeList2 = csv2.csvFormat().type;
    return true;
}
