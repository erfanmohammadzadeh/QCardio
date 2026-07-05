#ifndef ANNOTATIONREADER_H
#define ANNOTATIONREADER_H
#include <Models/global_qcardio.h>
#include "wfdb/wfdb.h"

struct AnnotationStats {
    int totalBeats;
    int normalBeats;
    int abnormalBeats;
    int pacemakerBeats;
    int artifactCount;
    // Add more as needed

    AnnotationStats() : totalBeats(0), normalBeats(0), abnormalBeats(0),
        pacemakerBeats(0), artifactCount(0) {}
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

class AnnotationReader {
public:
    AnnotationReader(const QString& dbPath, const QString& recordName, const QString& annotatorName = "atr")
        :m_databaseName(dbPath), m_recordName(recordName), m_annotatorName(annotatorName)
    {
        setwfdb(const_cast<char*>(dbPath.toStdString().c_str()));
    }

    // annotationreader.cpp
    bool loadAnnotations();

    // Getters
    const QVector<AnnotationData>& getAnnotations() const { return m_annotations; }
    const AnnotationStats& getStats() const { return m_stats; }

    // Utility functions


    QVector<RRInterval> computeRRIntervals(const QVector<AnnotationData>& annotations,
                                           int samplingRate);
    void applyResampling(double newFrequency);

    QVector<AnnotationData> getResampledAnnotations() const;
    double getOriginalFrequency() const { return m_originalFrequency; }
    double getResampledFrequency() const { return m_resampledFrequency; }
    QVector<AnnotationData> getResampledQRSAnnotations() const;
private:
    QString m_recordName;
    QString m_databaseName;
    QString m_annotatorName;
    QVector<AnnotationData> m_annotations;
    AnnotationStats m_stats;
    double m_originalFrequency;
    double m_resampledFrequency;

    long resampleTime(long oldSample, double oldFreq, double newFreq);

    void resampleAnnotations(QVector<AnnotationData>& annotations,
                             double oldFreq, double newFreq);
};

#endif // ANNOTATIONREADER_H
