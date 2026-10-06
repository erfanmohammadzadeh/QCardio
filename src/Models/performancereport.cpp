#include "performancereport.h"

#include "Models/ec57metrics.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QVector>

namespace {

QString formatRate(float value)
{
    if (value < 0.0f)
        return QStringLiteral("nan");
    return QString::number(static_cast<double>(value), 'f', 2);
}

QString formatCount(int value)
{
    return QString::number(value);
}

bool openText(QFile &file)
{
    return file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
}

void writeRatePair(QTextStream &out, const IecRatePair &rate)
{
    out << formatRate(rate.gross) << "," << formatRate(rate.average);
}

void writeSummaryRow(QTextStream &out,
                     const QString &name,
                     int tp,
                     int fn,
                     int fp,
                     int tn,
                     bool tnDefined,
                     const IecDetectionSummary &summary,
                     bool fprDefined)
{
    out << name << ","
        << formatCount(tp) << ","
        << formatCount(fn) << ","
        << formatCount(fp) << ","
        << (tnDefined ? formatCount(tn) : QStringLiteral("nan")) << ",";
    writeRatePair(out, summary.sensitivity);
    out << ",";
    writeRatePair(out, summary.positivePredictivity);
    out << ",";
    if (fprDefined)
        writeRatePair(out, summary.falsePositiveRate);
    else
        out << "nan,nan";
    out << "\n";
}

struct CountSum {
    int tp = 0;
    int fn = 0;
    int fp = 0;
    int tn = 0;
    int ppTp = 0;
    int ppFp = 0;

    void add(const Predicting &stat)
    {
        tp += stat.tp;
        fn += stat.fn;
        fp += stat.fp;
        tn += stat.tn;
        ppTp += stat.ppTp;
        ppFp += stat.ppFp;
    }
};

QString classToken(int beatClass)
{
    return testClassName(beatClass).replace(QLatin1Char(' '), QLatin1Char('_'));
}

QVector<int> matrixColumns(const BeatTestProfile &profile)
{
    QVector<int> columns = profile.activeClasses();
    columns.append(ClassMiss);
    return columns;
}

void writeBeatHeader(QTextStream &out, const BeatTestProfile &profile)
{
    out << "Record";
    if (profile.reportBeats) {
        const QVector<int> rows = profile.activeClasses();
        const QVector<int> columns = matrixColumns(profile);
        for (int row : rows) {
            for (int column : columns)
                out << "," << testClassLetter(row) << testClassLetter(column);
        }
        for (int beatClass : rows) {
            const QString name = classToken(beatClass);
            out << "," << name << "_TP"
                << "," << name << "_FN"
                << "," << name << "_FP"
                << "," << name << "_TN"
                << "," << name << "_Se"
                << "," << name << "_+P"
                << "," << name << "_FPR";
        }
    }
    if (profile.reportQrs)
        out << ",QTP,QFN,QFP,QRS_Se,QRS_+P";
    out << "\n";
}

void writeBeatRow(QTextStream &out, const FileProcessResult &file, const BeatTestProfile &profile)
{
    out << file.fileName;
    if (profile.reportBeats) {
        const QVector<int> rows = profile.activeClasses();
        const QVector<int> columns = matrixColumns(profile);
        for (int row : rows) {
            for (int column : columns)
                out << "," << file.testBeat[row][column];
        }
        for (int beatClass : rows) {
            const Predicting &stat = file.classPredict[beatClass];
            out << "," << stat.tp
                << "," << stat.fn
                << "," << stat.fp
                << "," << stat.tn
                << "," << formatRate(stat.se)
                << "," << formatRate(stat.p)
                << "," << formatRate(stat.fpr);
        }
    }
    if (profile.reportQrs) {
        out << "," << file.qrsPredict.tp
            << "," << file.qrsPredict.fn
            << "," << file.qrsPredict.fp
            << "," << formatRate(file.qrsPredict.se)
            << "," << formatRate(file.qrsPredict.p);
    }
    out << "\n";
}

bool writeBeatReport(const QString &path, const AnalyseFileProcessResult &result)
{
    QFile file(path);
    if (!openText(file))
        return false;

    QTextStream out(&file);
    const BeatTestProfile &profile = result.profile;
    writeBeatHeader(out, profile);
    for (const FileProcessResult &record : result.fileProcessResult) {
        writeBeatRow(out, record, profile);
        if (out.status() != QTextStream::Ok) {
            file.close();
            return false;
        }
    }

    CountSum qrs;
    for (const FileProcessResult &record : result.fileProcessResult)
        qrs.add(record.qrsPredict);

    out << "\nClass,TP,FN,FP,TN,Sensitivity gross,Sensitivity average,"
           "Positive predictivity gross,Positive predictivity average,"
           "FPR gross,FPR average\n";
    if (profile.reportQrs)
        writeSummaryRow(out, QStringLiteral("QRS"), qrs.tp, qrs.fn, qrs.fp, 0, false, result.qrs, false);
    if (profile.reportBeats) {
        for (int beatClass : profile.activeClasses()) {
            CountSum counts;
            for (const FileProcessResult &record : result.fileProcessResult)
                counts.add(record.classPredict[beatClass]);
            writeSummaryRow(out,
                            testClassName(beatClass),
                            counts.tp,
                            counts.fn,
                            counts.fp,
                            counts.tn,
                            true,
                            result.classSummary[beatClass],
                            true);
        }
    }
    file.close();
    return out.status() == QTextStream::Ok;
}

float shutdownPercent(int part, int total)
{
    return ec57Percent(part, total);
}

bool writeShutdownReport(const QString &path, const AnalyseFileProcessResult &result)
{
    QFile file(path);
    if (!openText(file))
        return false;

    const QVector<int> classes = result.profile.activeClasses();
    QTextStream out(&file);
    out << "Record";
    for (int beatClass : classes)
        out << "," << classToken(beatClass) << " missed";
    out << ",Missed beats,% all missed";
    for (int beatClass : classes)
        out << ",% " << testClassName(beatClass) << " missed";
    out << "\n";

    QVector<int> grossMissed(classes.size(), 0);
    QVector<int> grossReference(classes.size(), 0);
    int grossAllMissed = 0;
    int grossAllReference = 0;

    for (const FileProcessResult &record : result.fileProcessResult) {
        int recordMissed = 0;
        int recordReference = 0;
        QVector<int> classMissed(classes.size(), 0);
        QVector<int> classReference(classes.size(), 0);
        for (int index = 0; index < classes.size(); ++index) {
            const int beatClass = classes.at(index);
            classMissed[index] = static_cast<int>(record.testBeat[beatClass][ClassMiss]);
            for (int column = 0; column < TestBeatClassCount; ++column)
                classReference[index] += static_cast<int>(record.testBeat[beatClass][column]);
            recordMissed += classMissed[index];
            recordReference += classReference[index];
            grossMissed[index] += classMissed[index];
            grossReference[index] += classReference[index];
        }
        grossAllMissed += recordMissed;
        grossAllReference += recordReference;

        out << record.fileName;
        for (int missed : classMissed)
            out << "," << missed;
        out << "," << recordMissed
            << "," << formatRate(shutdownPercent(recordMissed, recordReference));
        for (int index = 0; index < classes.size(); ++index)
            out << "," << formatRate(shutdownPercent(classMissed[index], classReference[index]));
        out << "\n";
        if (out.status() != QTextStream::Ok) {
            file.close();
            return false;
        }
    }

    out << "Gross";
    for (int missed : grossMissed)
        out << "," << missed;
    out << "," << grossAllMissed
        << "," << formatRate(shutdownPercent(grossAllMissed, grossAllReference));
    for (int index = 0; index < classes.size(); ++index)
        out << "," << formatRate(shutdownPercent(grossMissed[index], grossReference[index]));
    out << "\n";
    file.close();
    return out.status() == QTextStream::Ok;
}

void writeRunFields(QTextStream &out, const Predicting &stat)
{
    out << stat.tp << ","
        << stat.fn << ","
        << stat.ppTp << ","
        << stat.ppFp << ","
        << formatRate(stat.se) << ","
        << formatRate(stat.p);
}

bool writeRunReport(const QString &path, const AnalyseFileProcessResult &result)
{
    QFile file(path);
    if (!openText(file))
        return false;

    const bool ventricular = result.profile.reportVentricularRuns;
    const bool supraventricular = result.profile.reportSupraventricularRuns;
    QTextStream out(&file);
    out << "Record";
    if (ventricular) {
        out << ",V couplet SeTP,V couplet SeFN,V couplet +P TP,V couplet +P FP,V couplet Se,V couplet +P"
               ",V short SeTP,V short SeFN,V short +P TP,V short +P FP,V short Se,V short +P"
               ",V long SeTP,V long SeFN,V long +P TP,V long +P FP,V long Se,V long +P";
    }
    if (supraventricular) {
        out << ",S couplet SeTP,S couplet SeFN,S couplet +P TP,S couplet +P FP,S couplet Se,S couplet +P"
               ",S short SeTP,S short SeFN,S short +P TP,S short +P FP,S short Se,S short +P"
               ",S long SeTP,S long SeFN,S long +P TP,S long +P FP,S long Se,S long +P";
    }
    out << "\n";

    CountSum vCouplet;
    CountSum vShort;
    CountSum vLong;
    CountSum sCouplet;
    CountSum sShort;
    CountSum sLong;

    for (const FileProcessResult &record : result.fileProcessResult) {
        out << record.fileName;
        if (ventricular) {
            out << ",";
            writeRunFields(out, record.pvc_couplet);
            out << ",";
            writeRunFields(out, record.pvc_shortRun);
            out << ",";
            writeRunFields(out, record.pvc_longRun);
        }
        if (supraventricular) {
            out << ",";
            writeRunFields(out, record.svt_couplet);
            out << ",";
            writeRunFields(out, record.svt_shortRun);
            out << ",";
            writeRunFields(out, record.svt_longRun);
        }
        out << "\n";

        vCouplet.add(record.pvc_couplet);
        vShort.add(record.pvc_shortRun);
        vLong.add(record.pvc_longRun);
        sCouplet.add(record.svt_couplet);
        sShort.add(record.svt_shortRun);
        sLong.add(record.svt_longRun);

        if (out.status() != QTextStream::Ok) {
            file.close();
            return false;
        }
    }

    out << "\nClass,Se TP,Se FN,+P TP,+P FP,Sensitivity gross,Sensitivity average,"
           "Positive predictivity gross,Positive predictivity average\n";

    auto writeAggregate = [&](const QString &name, const CountSum &counts, const IecDetectionSummary &summary) {
        out << name << ","
            << counts.tp << ","
            << counts.fn << ","
            << counts.ppTp << ","
            << counts.ppFp << ",";
        writeRatePair(out, summary.sensitivity);
        out << ",";
        writeRatePair(out, summary.positivePredictivity);
        out << "\n";
    };

    if (ventricular) {
        writeAggregate(QStringLiteral("V couplet"), vCouplet, result.vCouplet);
        writeAggregate(QStringLiteral("V short run"), vShort, result.vShortRun);
        writeAggregate(QStringLiteral("V long run"), vLong, result.vLongRun);
    }
    if (supraventricular) {
        writeAggregate(QStringLiteral("S couplet"), sCouplet, result.sCouplet);
        writeAggregate(QStringLiteral("S short run"), sShort, result.sShortRun);
        writeAggregate(QStringLiteral("S long run"), sLong, result.sLongRun);
    }

    file.close();
    return out.status() == QTextStream::Ok;
}

} // namespace

namespace PerformanceReport {

bool write(const QString &reportPath, const AnalyseFileProcessResult &result)
{
    const QFileInfo info(reportPath);
    const QString directory = info.absolutePath();
    const QString baseName = info.completeBaseName();
    const QString shutdownPath = directory + QLatin1Char('/') + baseName + QStringLiteral("_shutdown.csv");
    const QString runPath = directory + QLatin1Char('/') + baseName + QStringLiteral("_runs.csv");

    const BeatTestProfile &profile = result.profile;
    bool ok = true;
    if (profile.reportBeats || profile.reportQrs)
        ok = writeBeatReport(reportPath, result) && ok;
    if (profile.reportShutdown)
        ok = writeShutdownReport(shutdownPath, result) && ok;
    if (profile.reportVentricularRuns || profile.reportSupraventricularRuns)
        ok = writeRunReport(runPath, result) && ok;
    return ok;
}

} // namespace PerformanceReport
