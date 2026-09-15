#ifndef ANNOTATIONSERVICE_H
#define ANNOTATIONSERVICE_H

#include <QObject>
#include "Models/global_qcardio.h"
#include "ThirdParty/wfdb/wfdb.h"

class AnnotationService : public QObject
{
    Q_OBJECT
public:
    AnnotationService(QObject *parent = nullptr,
                      const SignalViewParameters& signalParam = SignalViewParameters(),
                      const QString& annotatorName = "atr");
    bool loadAnnotations();
    const QVector<AnnotationData>& getAnnotations() const;
    void calcNoneBeatIndex(MIT_BIH_ECGData& data);
    QVector<RRInterval> computeRRIntervals(const QVector<AnnotationData>& annotations, int samplingRate);

signals:
    void sigAppendLog(const QString& text);

private:
    QString m_recordName;
    QString m_databaseName;
    QString m_annotatorName;
    int m_sourceFreq = 360;
    QVector<AnnotationData> m_annotations;
    double m_originalFrequency = 0;
    double m_resampledFrequency = 0;
};

#endif // ANNOTATIONSERVICE_H
