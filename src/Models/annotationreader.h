#ifndef ANNOTATIONREADER_H
#define ANNOTATIONREADER_H
#include <Models/global_qcardio.h>
#include "wfdb/wfdb.h"

class AnnotationReader {
public:
    AnnotationReader(const QString& dbPath, const QString& recordName, const QString& annotatorName = "atr");
    bool loadAnnotations();
    const QVector<AnnotationData>& getAnnotations() const;
    QVector<RRInterval> computeRRIntervals(const QVector<AnnotationData>& annotations,
                                           int samplingRate);
    void applyResampling(double newFrequency);
    QVector<AnnotationData> getResampledAnnotations() const;
private:
    QString m_recordName;
    QString m_databaseName;
    QString m_annotatorName;
    QVector<AnnotationData> m_annotations;
    double m_originalFrequency;
    double m_resampledFrequency;

    long resampleTime(long oldSample, double oldFreq, double newFreq);

    void resampleAnnotations(QVector<AnnotationData>& annotations,
                             double oldFreq, double newFreq);
};

#endif // ANNOTATIONREADER_H
