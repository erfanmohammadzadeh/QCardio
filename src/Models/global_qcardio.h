#ifndef GLOBAL_QCARDIO_H
#define GLOBAL_QCARDIO_H

#include <QString>
#include <QDir>
#include <QMessageBox>
#include <QFile>
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

struct MIT_BIH_ECGData
{
    QString filename;
    QString dbName;
    qint64 totalSample = 0;
    quint16 sampling       = 178;
    QVector<QVector<qreal>> nsigs;
    quint8 selectedLead[LEAD_COUNT];
    QDateTime recordDate   = QDateTime(QDate(2000,1,1),QTime(0,0,0));

    void clear()
    {
        for(QVector<qreal> sig: nsigs)
        {
            sig.clear();
            sig.squeeze();
        }
        nsigs.clear();
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

#endif // GLOBAL_QCARDIO_H
