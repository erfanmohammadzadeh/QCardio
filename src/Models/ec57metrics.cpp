#include "ec57metrics.h"

#include <QtGlobal>

#include <algorithm>
#include <cstring>
#include <initializer_list>

namespace {

int cell(const quint32 matrix[AamiClassCount][AamiClassCount], int row, int column)
{
    return static_cast<int>(matrix[row][column]);
}

int sumColumns(const quint32 matrix[AamiClassCount][AamiClassCount], int row, std::initializer_list<int> columns)
{
    int total = 0;
    for (int column : columns)
        total += cell(matrix, row, column);
    return total;
}

int sumRunRow(const quint32 matrix[IecRunBinCount][IecRunBinCount], int row, int firstColumn, int lastColumn)
{
    int total = 0;
    for (int column = firstColumn; column <= lastColumn; ++column)
        total += static_cast<int>(matrix[row][column]);
    return total;
}

int sumRunColumn(const quint32 matrix[IecRunBinCount][IecRunBinCount], int column, int firstRow, int lastRow)
{
    int total = 0;
    for (int row = firstRow; row <= lastRow; ++row)
        total += static_cast<int>(matrix[row][column]);
    return total;
}

bool isRunBeat(int annotationCode, bool ventricular)
{
    const AamiClass beatClass = aamiClassOf(annotationCode);
    if (ventricular)
        return beatClass == AamiV || beatClass == AamiF;
    return beatClass == AamiS;
}

int longestRunInWindow(const QVector<AnnotatedBeat> &beats, bool ventricular, int windowStart, int windowEnd)
{
    int best = 0;
    int current = 0;
    for (const AnnotatedBeat &beat : beats) {
        if (beat.sample < windowStart || beat.sample > windowEnd) {
            current = 0;
            continue;
        }
        if (isRunBeat(beat.type, ventricular)) {
            ++current;
            best = std::max(best, current);
        } else {
            current = 0;
        }
    }
    return best;
}

struct RunEvent {
    int startSample = 0;
    int endSample = 0;
    int length = 0;
};

QVector<RunEvent> findRuns(const QVector<AnnotatedBeat> &beats, bool ventricular)
{
    QVector<RunEvent> runs;
    int length = 0;
    int startSample = 0;
    int endSample = 0;

    auto flush = [&]() {
        if (length <= 0)
            return;
        runs.append(RunEvent{startSample, endSample, length});
        length = 0;
    };

    for (const AnnotatedBeat &beat : beats) {
        if (isRunBeat(beat.type, ventricular)) {
            if (length == 0)
                startSample = beat.sample;
            endSample = beat.sample;
            ++length;
        } else {
            flush();
        }
    }
    flush();
    return runs;
}

void zeroRunMatrix(quint32 matrix[IecRunBinCount][IecRunBinCount])
{
    std::memset(matrix, 0, sizeof(quint32) * IecRunBinCount * IecRunBinCount);
}

struct DetectionAccumulator {
    int tp = 0;
    int fn = 0;
    int fp = 0;
    int tn = 0;
    int ppTp = 0;
    int ppFp = 0;
    double sumSensitivity = 0.0;
    double sumPredictivity = 0.0;
    double sumFalsePositiveRate = 0.0;
    int sensitivityCount = 0;
    int predictivityCount = 0;
    int falsePositiveCount = 0;

    void add(const Predicting &stat, bool withFalsePositiveRate)
    {
        tp += stat.tp;
        fn += stat.fn;
        fp += stat.fp;
        tn += stat.tn;
        ppTp += stat.ppTp;
        ppFp += stat.ppFp;
        if (stat.se >= 0.0f) {
            sumSensitivity += stat.se;
            ++sensitivityCount;
        }
        if (stat.p >= 0.0f) {
            sumPredictivity += stat.p;
            ++predictivityCount;
        }
        if (withFalsePositiveRate && stat.fpr >= 0.0f) {
            sumFalsePositiveRate += stat.fpr;
            ++falsePositiveCount;
        }
    }

    IecDetectionSummary summary(bool withFalsePositiveRate) const
    {
        IecDetectionSummary out;
        out.sensitivity.gross = ec57Percent(tp, tp + fn);
        out.sensitivity.average = sensitivityCount > 0
                                      ? static_cast<float>(sumSensitivity / sensitivityCount)
                                      : -1.0f;
        out.positivePredictivity.gross = ec57Percent(ppTp, ppTp + ppFp);
        out.positivePredictivity.average = predictivityCount > 0
                                               ? static_cast<float>(sumPredictivity / predictivityCount)
                                               : -1.0f;
        if (withFalsePositiveRate) {
            out.falsePositiveRate.gross = ec57Percent(fp, fp + tn);
            out.falsePositiveRate.average = falsePositiveCount > 0
                                                ? static_cast<float>(sumFalsePositiveRate / falsePositiveCount)
                                                : -1.0f;
        }
        return out;
    }
};

} // namespace

int matrixSum(const quint32 matrix[AamiClassCount][AamiClassCount], int row, int column)
{
    return cell(matrix, row, column);
}

int referenceClassCount(const quint32 matrix[AamiClassCount][AamiClassCount], int referenceClass)
{
    int total = 0;
    for (int column = 0; column < AamiClassCount; ++column)
        total += cell(matrix, referenceClass, column);
    return total;
}

Ec57BeatScores Ec57BeatScores::fromMatrix(const quint32 matrix[AamiClassCount][AamiClassCount])
{
    // IEC 60601-2-47:2012 Annex AA, clause 201.12.1.101.1.5.2.
    // QTP counts every reference QRS paired with an algorithm QRS.
    // QFN counts reference QRS paired with O or X. QFP counts algorithm QRS paired with O or X.
    Ec57BeatScores scores;

    const int qtp = cell(matrix, AamiN, AamiN) + cell(matrix, AamiN, AamiS) + cell(matrix, AamiN, AamiV) + cell(matrix, AamiN, AamiF) + cell(matrix, AamiN, AamiQ)
                  + cell(matrix, AamiS, AamiN) + cell(matrix, AamiS, AamiS) + cell(matrix, AamiS, AamiV) + cell(matrix, AamiS, AamiF) + cell(matrix, AamiS, AamiQ)
                  + cell(matrix, AamiV, AamiN) + cell(matrix, AamiV, AamiS) + cell(matrix, AamiV, AamiV) + cell(matrix, AamiV, AamiF) + cell(matrix, AamiV, AamiQ)
                  + cell(matrix, AamiF, AamiN) + cell(matrix, AamiF, AamiS) + cell(matrix, AamiF, AamiV) + cell(matrix, AamiF, AamiF) + cell(matrix, AamiF, AamiQ)
                  + cell(matrix, AamiQ, AamiN) + cell(matrix, AamiQ, AamiS) + cell(matrix, AamiQ, AamiV) + cell(matrix, AamiQ, AamiF) + cell(matrix, AamiQ, AamiQ);
    const int qfn = sumColumns(matrix, AamiN, {AamiO, AamiX})
                  + sumColumns(matrix, AamiS, {AamiO, AamiX})
                  + sumColumns(matrix, AamiV, {AamiO, AamiX})
                  + sumColumns(matrix, AamiF, {AamiO, AamiX})
                  + sumColumns(matrix, AamiQ, {AamiO, AamiX});
    const int qfp = sumColumns(matrix, AamiO, {AamiN, AamiS, AamiV, AamiF, AamiQ})
                  + sumColumns(matrix, AamiX, {AamiN, AamiS, AamiV, AamiF, AamiQ});

    scores.qrs.tp = qtp;
    scores.qrs.fn = qfn;
    scores.qrs.fp = qfp;
    scores.qrs.calcParams(false);

    // VEB. Fv and Qv are omitted from both VTP and VFP.
    scores.veb.tp = cell(matrix, AamiV, AamiV);
    scores.veb.fn = sumColumns(matrix, AamiV, {AamiN, AamiS, AamiF, AamiQ, AamiO, AamiX});
    scores.veb.fp = cell(matrix, AamiN, AamiV) + cell(matrix, AamiS, AamiV)
                  + cell(matrix, AamiO, AamiV) + cell(matrix, AamiX, AamiV);
    scores.veb.tn = cell(matrix, AamiN, AamiN) + cell(matrix, AamiN, AamiS) + cell(matrix, AamiN, AamiF) + cell(matrix, AamiN, AamiQ)
                  + cell(matrix, AamiS, AamiN) + cell(matrix, AamiS, AamiS) + cell(matrix, AamiS, AamiF) + cell(matrix, AamiS, AamiQ)
                  + cell(matrix, AamiF, AamiN) + cell(matrix, AamiF, AamiS) + cell(matrix, AamiF, AamiF) + cell(matrix, AamiF, AamiQ)
                  + cell(matrix, AamiQ, AamiN) + cell(matrix, AamiQ, AamiS) + cell(matrix, AamiQ, AamiF) + cell(matrix, AamiQ, AamiQ)
                  + cell(matrix, AamiO, AamiN) + cell(matrix, AamiO, AamiS) + cell(matrix, AamiO, AamiF) + cell(matrix, AamiO, AamiQ)
                  + cell(matrix, AamiX, AamiN) + cell(matrix, AamiX, AamiS) + cell(matrix, AamiX, AamiF) + cell(matrix, AamiX, AamiQ);
    scores.veb.calcParams(true);

    // SVEB. Qs is omitted from SVTP and SVFP.
    scores.sveb.tp = cell(matrix, AamiS, AamiS);
    scores.sveb.fn = sumColumns(matrix, AamiS, {AamiN, AamiV, AamiF, AamiQ, AamiO, AamiX});
    scores.sveb.fp = cell(matrix, AamiN, AamiS) + cell(matrix, AamiV, AamiS) + cell(matrix, AamiF, AamiS)
                   + cell(matrix, AamiO, AamiS) + cell(matrix, AamiX, AamiS);
    scores.sveb.tn = cell(matrix, AamiN, AamiN) + cell(matrix, AamiN, AamiV) + cell(matrix, AamiN, AamiF) + cell(matrix, AamiN, AamiQ)
                   + cell(matrix, AamiV, AamiN) + cell(matrix, AamiV, AamiV) + cell(matrix, AamiV, AamiF) + cell(matrix, AamiV, AamiQ)
                   + cell(matrix, AamiF, AamiN) + cell(matrix, AamiF, AamiV) + cell(matrix, AamiF, AamiF) + cell(matrix, AamiF, AamiQ)
                   + cell(matrix, AamiQ, AamiN) + cell(matrix, AamiQ, AamiV) + cell(matrix, AamiQ, AamiF) + cell(matrix, AamiQ, AamiQ)
                   + cell(matrix, AamiO, AamiN) + cell(matrix, AamiO, AamiV) + cell(matrix, AamiO, AamiF) + cell(matrix, AamiO, AamiQ)
                   + cell(matrix, AamiX, AamiN) + cell(matrix, AamiX, AamiV) + cell(matrix, AamiX, AamiF) + cell(matrix, AamiX, AamiQ);
    scores.sveb.calcParams(true);

    // Class-N counts. Not a Table 201.103 statistic; kept so a normal-beat matrix can be audited.
    scores.normal.tp = cell(matrix, AamiN, AamiN);
    scores.normal.fn = sumColumns(matrix, AamiN, {AamiS, AamiV, AamiF, AamiQ, AamiO, AamiX});
    scores.normal.fp = cell(matrix, AamiS, AamiN) + cell(matrix, AamiV, AamiN) + cell(matrix, AamiF, AamiN)
                     + cell(matrix, AamiQ, AamiN) + cell(matrix, AamiO, AamiN) + cell(matrix, AamiX, AamiN);
    int allPairs = 0;
    int normalColumn = 0;
    for (int row = 0; row < AamiClassCount; ++row) {
        for (int column = 0; column < AamiClassCount; ++column)
            allPairs += cell(matrix, row, column);
        normalColumn += cell(matrix, row, AamiN);
    }
    const int normalRow = referenceClassCount(matrix, AamiN);
    scores.normal.tn = allPairs - normalRow - normalColumn + scores.normal.tp;
    scores.normal.calcParams(false);
    return scores;
}

Ec57RunScores Ec57RunScores::fromMatrices(const quint32 sensitivity[IecRunBinCount][IecRunBinCount],
                                          const quint32 predictivity[IecRunBinCount][IecRunBinCount])
{
    // Clause 201.12.1.101.1.5.3. Sensitivity and positive predictivity use different
    // numerators because a reference run and an algorithm run are counted separately.
    Ec57RunScores scores;

    auto fill = [&](Predicting &stat, int bin, int binEnd) {
        stat.tp = 0;
        stat.fn = 0;
        stat.ppTp = 0;
        stat.ppFp = 0;
        stat.splitPredictivity = true;
        for (int row = bin; row <= binEnd; ++row) {
            stat.tp += sumRunRow(sensitivity, row, bin, IecRunBinCount - 1);
            stat.fn += sumRunRow(sensitivity, row, 0, bin - 1);
        }
        for (int column = bin; column <= binEnd; ++column) {
            stat.ppTp += sumRunColumn(predictivity, column, bin, IecRunBinCount - 1);
            stat.ppFp += sumRunColumn(predictivity, column, 0, bin - 1);
        }
        stat.calcParams(false);
    };

    fill(scores.couplet, 2, 2);
    fill(scores.shortRun, 3, 5);
    fill(scores.longRun, 6, 6);
    return scores;
}

void fillRunMatrices(const QVector<AnnotatedBeat> &reference,
                     const QVector<AnnotatedBeat> &test,
                     int sampleRate,
                     bool ventricular,
                     quint32 sensitivity[IecRunBinCount][IecRunBinCount],
                     quint32 predictivity[IecRunBinCount][IecRunBinCount])
{
    zeroRunMatrix(sensitivity);
    zeroRunMatrix(predictivity);
    const int window = matchWindowSamples(sampleRate);

    const QVector<RunEvent> referenceRuns = findRuns(reference, ventricular);
    for (const RunEvent &run : referenceRuns) {
        const int opposite = longestRunInWindow(test, ventricular, run.startSample - window, run.endSample + window);
        const int row = runLengthBin(run.length);
        const int column = runLengthBin(opposite);
        ++sensitivity[row][column];
    }

    const QVector<RunEvent> testRuns = findRuns(test, ventricular);
    for (const RunEvent &run : testRuns) {
        const int opposite = longestRunInWindow(reference, ventricular, run.startSample - window, run.endSample + window);
        const int row = runLengthBin(opposite);
        const int column = runLengthBin(run.length);
        ++predictivity[row][column];
    }
}

void FileProcessResult::clear()
{
    fileName.clear();
    qrsPredict.clear();
    normalPredict.clear();
    pvcPredict.clear();
    svtPredict.clear();
    AFPredict.clear();
    pvc_couplet.clear();
    pvc_shortRun.clear();
    pvc_longRun.clear();
    svt_couplet.clear();
    svt_shortRun.clear();
    svt_longRun.clear();
    AF_duration.clear();
    missedBeatCount = 0;
    normalMissed = 0.0f;
    pvcMissed = 0.0f;
    totalShutdownSqrs = 0;
    totalShutdown = QTime(0, 0, 0);
    std::memset(beatTypeMap, 0, sizeof(beatTypeMap));
    std::memset(aamiBeat, 0, sizeof(aamiBeat));
    std::memset(vRunSensitivity, 0, sizeof(vRunSensitivity));
    std::memset(vRunPredictivity, 0, sizeof(vRunPredictivity));
    std::memset(sRunSensitivity, 0, sizeof(sRunSensitivity));
    std::memset(sRunPredictivity, 0, sizeof(sRunPredictivity));
    std::memset(testBeat, 0, sizeof(testBeat));
    for (Predicting &stat : classPredict)
        stat.clear();
    profile = BeatTestProfile();
    compareOfset = 0;
    totalBeat = 0;
}

void AnalyseFileProcessResult::calcParam()
{
    // Gross statistics weight every event equally. Average statistics weight every
    // record equally and skip a record only when that record's denominator is zero.
    DetectionAccumulator qrsAcc;
    DetectionAccumulator vebAcc;
    DetectionAccumulator svebAcc;
    DetectionAccumulator vCoupletAcc;
    DetectionAccumulator vShortAcc;
    DetectionAccumulator vLongAcc;
    DetectionAccumulator sCoupletAcc;
    DetectionAccumulator sShortAcc;
    DetectionAccumulator sLongAcc;

    for (const FileProcessResult &file : fileProcessResult) {
        qrsAcc.add(file.qrsPredict, false);
        vebAcc.add(file.pvcPredict, true);
        svebAcc.add(file.svtPredict, true);
        vCoupletAcc.add(file.pvc_couplet, false);
        vShortAcc.add(file.pvc_shortRun, false);
        vLongAcc.add(file.pvc_longRun, false);
        sCoupletAcc.add(file.svt_couplet, false);
        sShortAcc.add(file.svt_shortRun, false);
        sLongAcc.add(file.svt_longRun, false);
    }

    for (int beatClass = ClassNormal; beatClass <= ClassUnknown; ++beatClass) {
        if (!profile.classSelected(beatClass)) {
            classSummary[beatClass] = IecDetectionSummary();
            continue;
        }
        DetectionAccumulator classAcc;
        for (const FileProcessResult &file : fileProcessResult)
            classAcc.add(file.classPredict[beatClass], true);
        classSummary[beatClass] = classAcc.summary(true);
    }

    qrs = qrsAcc.summary(false);
    veb = vebAcc.summary(true);
    sveb = svebAcc.summary(true);
    vCouplet = vCoupletAcc.summary(false);
    vShortRun = vShortAcc.summary(false);
    vLongRun = vLongAcc.summary(false);
    sCouplet = sCoupletAcc.summary(false);
    sShortRun = sShortAcc.summary(false);
    sLongRun = sLongAcc.summary(false);
}

void AnalyseFileProcessResult::clear()
{
    for (FileProcessResult &file : fileProcessResult)
        file.clear();
    fileProcessResult.clear();
    fileProcessResult.squeeze();
    profile = BeatTestProfile();
    for (IecDetectionSummary &summary : classSummary)
        summary = IecDetectionSummary();
    qrs = IecDetectionSummary();
    veb = IecDetectionSummary();
    sveb = IecDetectionSummary();
    vCouplet = IecDetectionSummary();
    vShortRun = IecDetectionSummary();
    vLongRun = IecDetectionSummary();
    sCouplet = IecDetectionSummary();
    sShortRun = IecDetectionSummary();
    sLongRun = IecDetectionSummary();
}
