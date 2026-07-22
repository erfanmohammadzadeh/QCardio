#ifndef ANNOTATIONREADER_H
#define ANNOTATIONREADER_H
#include <Models/global_qcardio.h>
#include "wfdb/wfdb.h"
#include <QObject>
class AnnotationReader : public QObject {
    Q_OBJECT
public:
    AnnotationReader(QObject *parent = nullptr,
                     const SignalViewParameters& signalParam = SignalViewParameters(),
                     const QString& annotatorName = "atr");
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
signals:
    void sigAppnedLog(const QString& text);
};

#endif // ANNOTATIONREADER_H
