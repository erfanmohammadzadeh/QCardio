#ifndef GLOBAL_QCARDIO_H
#define GLOBAL_QCARDIO_H

#include "wfdb/ecgcodes.h"
#include "aamiclass.h"
#include "beattestprofile.h"
#include <QString>
#include <QDir>
#include <QMessageBox>
#include <QFile>
#include <algorithm>
#define LEAD_COUNT 3

enum DirectoryValidationFlags {
    Exists = 0x01,
    Readable = 0x02,
    Writable = 0x04,
    Executable = 0x08,
    IsDirectory = 0x10,
    NotEmpty = 0x20,
    IsAbsolute = 0x40
};


struct AnnotationData {
    long time;           // Sample index
    long timeResampled;
    int anntyp;          // Annotation type code
    QString symbol;      // Symbol like "N", "V", "Q"
    QString description; // Description like "Normal beat"
    int subtype;         // Subtype information
    int channel;         // Channel number
    int number;          // Additional number (e.g., waveform morphology)
    QString aux;         // Auxiliary text (rhythm info, comments)

    // Optional: Add a constructor for convenience
    AnnotationData()
        : time(0), anntyp(0), subtype(0), channel(0), number(0) {}
};

struct RRInterval {
    long time1;     // Sample index of first beat
    long time2;     // Sample index of second beat
    long interval;  // Difference in samples
    double intervalSeconds; // Interval in seconds

    QString toString() const {
        return QString("RR = %1 samples (%2 ms)")
        .arg(interval)
            .arg(intervalSeconds * 1000, 0, 'f', 2);
    }
};

struct CSVFormat
{
    QVector<int> sampleIndex;
    QVector<quint8> type;
};

struct MIT_BIH_ECGData
{
    QString filename;
    QString dbName;
    qint64 totalSample = 0;
    quint16 sampling       = 178;
    QVector<QVector<qreal>> nsigs;
    quint8 selectedLead[LEAD_COUNT];
    QDateTime recordDate   = QDateTime(QDate(2000,1,1),QTime(0,0,0));
    QVector<AnnotationData> anotList;
    QVector<RRInterval> rrIntervals;
    QVector<int> noneBeatIndex;
    float adcPerMv = 200.0;

    void clear()
    {
        for(QVector<qreal> sig: nsigs)
        {
            sig.clear();
            sig.squeeze();
        }
        nsigs.clear();
        noneBeatIndex.clear();
    }
    CSVFormat toCSVFormat() const
    {
        CSVFormat converted;
        for (int i = 0; i < anotList.size(); i++)
        {
            if (noneBeatIndex.contains(i))
                continue;

            converted.sampleIndex << anotList[i].time;
            converted.type << anotList[i].anntyp;
        }
        return converted;
    }
    QStringList getAnotLable() const
    {
        QStringList anotLableList;
        for(const AnnotationData& label : anotList)
        {
            anotLableList << label.symbol;
        }
        return anotLableList;
    }
    QVector<int> getStartIndex() const
    {
        QVector<int> startIdxList;
        for(const AnnotationData& label : anotList)
        {
            startIdxList << label.time;
        }
        return startIdxList;
    }
    QStringList getAuxList() const
    {
        QStringList auxList;
        for(const AnnotationData& label : anotList)
        {
            auxList << label.aux;
        }
        return auxList;
    }
};

struct SignalViewParameters
{
    QString signalFilePath;
    QString dbPath;
    QString dbName;
    quint8 selectedLead[LEAD_COUNT];
    int sourceFs = 360;
    int targetFs = 178;
    float gain = 1.0;
    float offset = 0.0;
};

struct ExprotSetting
{
    enum ExportMethod
    {
        RawSample,
        RC7
    };
    ExportMethod method = RawSample;
    QString outputPath;
    SignalViewParameters params;
    QStringList pathList;
    bool compressingRequsted = true;
    bool exportCSV = true;
};

struct SheetResult
{
    QVector<int> sampleIndexList1;
    QVector<quint8> typeList1;
    QVector<int> alignedList1;
    QVector<int> typeAlignList1;

    QVector<int> sampleIndexList2;
    QVector<quint8> typeList2;
    QVector<int> alignedList2;
    QVector<int> typeAlignList2;

    QVector<int> difIndexList;
    QVector<bool> difTypeList;

    int getDif(int idx1, int idx2)
    {
        // Changed '>' to '>=' to fix the off-by-one crash
        if (idx1 < 0 || idx1 >= this->sampleIndexList1.size() ||
            idx2 < 0 || idx2 >= this->sampleIndexList2.size())
        {
            return -1;
        }

        return qAbs(this->sampleIndexList1.at(idx1) - this->sampleIndexList2.at(idx2));
    }

    int addInvalid(int idx, int csvFile)
    {
        if (csvFile == 1)
        {
            this->sampleIndexList1.insert(idx, -1);
            return this->sampleIndexList1.size();
        }
        else if (csvFile == 2)
        {
            this->sampleIndexList2.insert(idx, -1);
            return this->sampleIndexList2.size();
        }

        // Added fallback return to prevent undefined behavior
        return -1;
    }
};

struct AnalyseCfg
{
    QStringList csvPath1;
    QStringList csvPath2;
    QString outputPath;
    int sampleRate = 178;
    // First sample of the test period. 0 scores the whole file.
    // The compare screen sets this to 5 minutes, per IEC 60601-2-47:2012.
    int compareOfset = 0;
    BeatTestProfile profile;
};

// One detection statistic.
// Sensitivity uses tp and fn. Positive predictivity uses ppTp and ppFp.
// For QRS and VEB those numerators are the same count. For couplets and runs
// IEC 60601-2-47 counts reference runs and algorithm runs separately, so the
// two numerators differ and splitPredictivity is set.
// A rate of -1 means the denominator was zero and the statistic is undefined.
struct Predicting
{
    int fn = 0;
    int fp = 0;
    int tp = 0;
    int tn = 0;
    int ppTp = 0;
    int ppFp = 0;
    bool splitPredictivity = false;

    float rfn = -1.0f;
    float rfp = -1.0f;
    float rtp = -1.0f;
    float rtn = -1.0f;

    float se = -1.0f;
    float p = -1.0f;
    float fpr = -1.0f;

    void calcParams(bool computeFalsePositiveRate = true)
    {
        se = ec57Percent(tp, tp + fn);
        rfn = ec57Percent(fn, tp + fn);
        rtp = se;

        if (!splitPredictivity) {
            ppTp = tp;
            ppFp = fp;
        }
        p = ec57Percent(ppTp, ppTp + ppFp);

        if (computeFalsePositiveRate) {
            fpr = ec57Percent(fp, tn + fp);
            rfp = fpr;
            rtn = ec57Percent(tn, tn + fp);
        } else {
            fpr = -1.0f;
            rfp = -1.0f;
            rtn = -1.0f;
        }
    }

    void clear()
    {
        fn = 0;
        fp = 0;
        tp = 0;
        tn = 0;
        ppTp = 0;
        ppFp = 0;
        splitPredictivity = false;
        rfn = -1.0f;
        rfp = -1.0f;
        rtp = -1.0f;
        rtn = -1.0f;
        se = -1.0f;
        p = -1.0f;
        fpr = -1.0f;
    }
};

// Gross weights each event equally. Average weights each record equally.
struct IecRatePair {
    float gross = -1.0f;
    float average = -1.0f;
};

struct IecDetectionSummary {
    IecRatePair sensitivity;
    IecRatePair positivePredictivity;
    IecRatePair falsePositiveRate;
};

enum ArrhythmiaType
{
    NormalArr            = 1 ,
    SupraventricularArr  = 2 ,
    BundleBranchBlockArr = 3 ,
    AberrantArr          = 4 ,
    AtrialFibrillation   = 5 ,
    VentricularArr       = 6 ,
    RonTArr              = 7 ,
    InterpolatedArr      = 8 ,
    VentricularEscapeArr = 9 ,
    FusionArr            = 10 ,
    UnknownArr           = 11,
    Paced                = 12,
    ArrhythmiaTypeCount  = 13,
};

typedef quint32 BeatMatrixType[ArrhythmiaType::ArrhythmiaTypeCount][ArrhythmiaType::ArrhythmiaTypeCount];
typedef quint32 RunMatrixType[6][6];

struct FileProcessResult
{
    QString fileName;
    Predicting qrsPredict;
    Predicting normalPredict;
    Predicting pvcPredict;
    Predicting svtPredict;
    Predicting AFPredict;
    Predicting pvc_couplet;
    Predicting pvc_shortRun;
    Predicting pvc_longRun;
    Predicting svt_couplet;
    Predicting svt_shortRun;
    Predicting svt_longRun;
    Predicting AF_duration;
    int missedBeatCount = 0;
    float normalMissed = 0.0f;
    float pvcMissed = 0.0f;
    quint32 totalShutdownSqrs = 0;
    QTime totalShutdown;
    BeatMatrixType beatTypeMap = {{0}};
    quint32 aamiBeat[AamiClassCount][AamiClassCount] = {};
    quint32 vRunSensitivity[IecRunBinCount][IecRunBinCount] = {};
    quint32 vRunPredictivity[IecRunBinCount][IecRunBinCount] = {};
    quint32 sRunSensitivity[IecRunBinCount][IecRunBinCount] = {};
    quint32 sRunPredictivity[IecRunBinCount][IecRunBinCount] = {};
    BeatTestProfile profile;
    quint32 testBeat[TestBeatClassCount][TestBeatClassCount] = {};
    Predicting classPredict[ClassUnknown + 1] = {};
    int compareOfset = 0;
    int totalBeat = 0;

    void clear();
};

struct AnalyseFileProcessResult
{
    QVector<FileProcessResult> fileProcessResult;
    BeatTestProfile profile;
    IecDetectionSummary classSummary[ClassUnknown + 1];
    IecDetectionSummary qrs;
    IecDetectionSummary veb;
    IecDetectionSummary sveb;
    IecDetectionSummary vCouplet;
    IecDetectionSummary vShortRun;
    IecDetectionSummary vLongRun;
    IecDetectionSummary sCouplet;
    IecDetectionSummary sShortRun;
    IecDetectionSummary sLongRun;

    void calcParam();
    void clear();
};

const static QStringList datasetName = {"MIT-BIH", "AHA", "ESC", "CU"};
const static QStringList noneBeat = {"[", "!", "]", "x",
                                     "(", ")", "p", "t", "u", "`", "'", "^",
                                     "|", "~", "+", "s", "T", "*", "D", "=","@", "\""};

enum MatrixType
{
    BeatType,
    RunEpisode
};

static ArrhythmiaType convertTypeToArrhythmia(int index)
{
    switch (index)
    {
    case NORMAL:
        return ArrhythmiaType::NormalArr;
        break;
    case BBB:
        return ArrhythmiaType::BundleBranchBlockArr;
        break;
    case ABERR:
        return ArrhythmiaType::AberrantArr;
        break;
    case PVC:
        return ArrhythmiaType::VentricularArr;
        break;
    case FUSION:
        return ArrhythmiaType::FusionArr;
        break;
    case SVPB:
        return ArrhythmiaType::SupraventricularArr;
        break;
    case VESC:
        return ArrhythmiaType::VentricularEscapeArr;
        break;
    case PACE:
        return ArrhythmiaType::Paced;
        break;
    case RONT:
        return ArrhythmiaType::RonTArr;
        break;
    default:
        return ArrhythmiaType::UnknownArr;
        break;
    }
}

#endif // GLOBAL_QCARDIO_H
