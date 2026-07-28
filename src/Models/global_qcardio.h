#ifndef GLOBAL_QCARDIO_H
#define GLOBAL_QCARDIO_H

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

    QString toString(int samplingRate) const {
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
};

struct Predicting
{
    int   fn = 0; // False Negatives (we did not detect)
    int   fp = 0; // False Positives (we detect more)
    int   tp = 0; // True Positives (detect true)
    float se = 0.0;
    float p = 0.0;
    float accuracy = 0.0;

    void calcParams()
    {
        int total = this->tp + this->fp + this->fn;
        if (total == 0) {
            this->se = 0.0;
            this->p = 0.0;
            this->accuracy = 0.0;
            return;
        }

        this->se = (static_cast<float>(this->tp) / (this->tp + this->fn)) * 100;
        this->p  = (static_cast<float>(this->tp) / (this->tp + this->fp)) * 100;
        this->accuracy = (static_cast<float>(this->tp) / total) * 100;
    }

    void clear()
    {
        this->fn = 0;
        this->fp = 0;
        this->tp = 0;
        this->se = 0.0;
        this->p = 0.0;
        this->accuracy = 0.0;
    }
};
struct FileProcessResult
{
    QString fileName;
    Predicting qrsPredict;
    Predicting normalPredict;
    Predicting pvcPredict;

    void clear()
    {
        qrsPredict.clear();
        normalPredict.clear();
        pvcPredict.clear();
    }
};

const static QStringList datasetName = {"MIT-BIH", "AHA", "ESC", "CU"};

#endif // GLOBAL_QCARDIO_H
