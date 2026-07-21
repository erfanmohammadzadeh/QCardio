#ifndef ANNOTATIONREADER_H
#define ANNOTATIONREADER_H
#include <Models/global_qcardio.h>
#include "wfdb/wfdb.h"

class AnnotationReader {
public:
    AnnotationReader(const SignalViewParameters& signalParam = SignalViewParameters(), const QString& annotatorName = "atr");
    bool loadAnnotations();
    const QVector<AnnotationData>& getAnnotations() const;
    QVector<RRInterval> computeRRIntervals(const QVector<AnnotationData>& annotations, int samplingRate);
private:
    QString m_recordName;
    QString m_databaseName;
    QString m_annotatorName;
    int m_sourceFreq = 360;
    QVector<AnnotationData> m_annotations;
    double m_originalFrequency;
    double m_resampledFrequency;
};

#endif // ANNOTATIONREADER_H
