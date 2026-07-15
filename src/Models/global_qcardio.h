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

    void clear()
    {
        for(QVector<qreal> sig: nsigs)
        {
            sig.clear();
            sig.squeeze();
        }
        nsigs.clear();
    }

    CSVFormat toCSVFormat() const
    {
        CSVFormat converted;
        for(const AnnotationData& anot : this->anotList)
        {
            // qDebug() << "insave: " << anot.time;
            converted.sampleIndex << anot.time;
            converted.type << anot.anntyp;
        }
        return converted;
    }
};

struct SignalViewParameters
{
    QString signalFilePath;
    QString dbPath;
    QString dbName;
    quint8 selectedLead[LEAD_COUNT];
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
#endif // GLOBAL_QCARDIO_H
