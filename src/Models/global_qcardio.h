#ifndef GLOBAL_QCARDIO_H
#define GLOBAL_QCARDIO_H

#include "wfdb/ecgcodes.h"
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
    QVector<quint8> typeAlignList1;

    QVector<int> sampleIndexList2;
    QVector<quint8> typeList2;
    QVector<int> alignedList2;
    QVector<quint8> typeAlignList2;

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
    int compareOfset = 4;
};

struct Predicting
{
    int   fn = 0;
    int   fp = 0;
    int   tp = 0;
    int   tn = 0;

    float rfn = 0.0f;
    float rfp = 0.0f;
    float rtp = 0.0f;
    float rtn = 0.0f;

    float se  = 0.0f;
    float p   = 0.0f;
    float fpr = 0.0f;

    void calcParams()
    {
        const int actualPos    = tp + fn;
        const int actualNeg    = tn + fp;
        const int predictedPos = tp + fp;

        se  = (actualPos    > 0) ? 100.0f * static_cast<float>(tp) / static_cast<float>(actualPos)    : 0.0f;
        p   = (predictedPos > 0) ? 100.0f * static_cast<float>(tp) / static_cast<float>(predictedPos) : 0.0f;
        fpr = (actualNeg    > 0) ? 100.0f * static_cast<float>(fp) / static_cast<float>(actualNeg)    : 0.0f;

        rfn = 100.0f * static_cast<float>(fn) / static_cast<float>(actualPos);
        rfp = 100.0f * static_cast<float>(fp) / static_cast<float>(actualNeg);
        rtp = 100.0f * static_cast<float>(tp) / static_cast<float>(actualPos);
        rtn = 100.0f * static_cast<float>(tn) / static_cast<float>(actualNeg);
    }
    void clear()
    {
        fn = 0;
        fp = 0;
        tp = 0;
        tn = 0;
        rfn = 0.0f;
        rfp = 0.0f;
        rtp = 0.0f;
        rtn = 0.0f;
        se  = 0.0f;
        p   = 0.0f;
        fpr = 0.0f;
    }
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
    int   missedBeatCount = 0;
    float normalMissed = 0.0;
    float pvcMissed = 0.0;
    quint32 totalShutdownSqrs = 0;
    QTime totalShutdown;
    BeatMatrixType beatTypeMap = {{0}};
    int compareOfset = 4;
    int totalBeat = 0;

    void clear()
    {
        qrsPredict.clear();
        normalPredict.clear();
        pvcPredict.clear();
        // svtPredict.clear();
        AFPredict.clear();
        pvc_couplet.clear();
        pvc_shortRun.clear();
        pvc_longRun.clear();
        svt_couplet.clear();
        svt_shortRun.clear();
        svt_longRun.clear();
        AF_duration.clear();
        missedBeatCount = 0;
        normalMissed = 0.0;
        pvcMissed = 0.0;
        totalShutdownSqrs=0;
        compareOfset = 4;
        totalBeat = 0;
    }
};

struct AnalyseFileProcessResult
{
    QVector<FileProcessResult> fileProcessResult;
    float avgSePvc = 0.0;
    float avgSeQrs = 0.0;
    float avgSeNor = 0.0;
    float avgPPPvc = 0.0;
    float avgPPQrs = 0.0;
    float avgPPNor = 0.0;
    int   cntSePvc = 0;
    int   cntSeQrs = 0;
    int   cntSeNor = 0;
    int   cntPPPvc = 0;
    int   cntPPQrs = 0;
    int   cntPPNor = 0;

    void calcParam()
    {
        float sumSePvc = 0.0;
        float sumSeQrs = 0.0;
        float sumSeNor = 0.0;
        float sumPPPvc = 0.0;
        float sumPPQrs = 0.0;
        float sumPPNor = 0.0;
        for(const auto& file: fileProcessResult)
        {
            sumSePvc += addVal(file.pvcPredict.se, cntSePvc);
            sumSeQrs += addVal(file.qrsPredict.se, cntSeQrs);
            sumSeNor += addVal(file.normalPredict.se, cntSeNor);
            sumPPPvc += addVal(file.pvcPredict.p, cntPPPvc);
            sumPPQrs += addVal(file.qrsPredict.p, cntPPQrs);
            sumPPNor += addVal(file.normalPredict.p, cntPPNor);
        }
        avgSePvc = divideNZ(sumSePvc, cntSePvc);
        avgSeNor = divideNZ(sumSeNor, cntSeNor);
        avgSeQrs = divideNZ(sumSeQrs, cntSeQrs);
        avgPPPvc = divideNZ(sumPPPvc, cntPPPvc);
        avgPPNor = divideNZ(sumPPQrs, cntPPQrs);
        avgPPQrs = divideNZ(sumPPNor, cntPPNor);
    }

    float divideNZ(const float& value, const int& count)
    {
        if(count == 0) return 0.0;
        return value/static_cast<float>(count);
    }

    float addVal(const float& val, int& cnt)
    {
        if(val > 0)
        {
            cnt++;
            return val;
        }
        else
            return 0.0;
    }

    void clear()
    {
        for(int i = 0; i < fileProcessResult.size(); i++)
            fileProcessResult[i].clear();
        fileProcessResult.clear();
        fileProcessResult.squeeze();

        avgSePvc = 0.0;
        avgSeQrs = 0.0;
        avgSeNor = 0.0;
        avgPPPvc = 0.0;
        avgPPQrs = 0.0;
        avgPPNor = 0.0;
    }
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

static const QSet<int> NormalBeat = {NORMAL, LBBB, RBBB, BBB, SVPB, PFUS, RHYTHM};
static const QSet<int> PVCBeat    = {PVC, FUSION, PACE, PFUS, FLWAV, VESC, RHYTHM};
static const QSet<int> NOISEBeat  = {NOISE, UNKNOWN};



#endif // GLOBAL_QCARDIO_H
