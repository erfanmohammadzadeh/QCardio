#include "annotationreader.h"

AnnotationReader::AnnotationReader(const QString &dbPath, const QString &recordName, const QString &annotatorName)
    :m_databaseName(dbPath), m_recordName(recordName), m_annotatorName(annotatorName)
{
    setwfdb(const_cast<char*>(dbPath.toStdString().c_str()));
}

bool AnnotationReader::loadAnnotations() {
    qDebug() << "Record:" << m_recordName;
    m_annotatorName = "atr";

    QByteArray recordBA = m_recordName.toLatin1();
    QByteArray annotatorBA = m_annotatorName.toLatin1();

    WFDB_Anninfo annInfo;
    annInfo.name = (char*)annotatorBA.constData();
    annInfo.stat = WFDB_READ;//for aha this should change to AHA

    if (annopen((char*)recordBA.constData(), &annInfo, 1) < 0) {
        qCritical() << "Failed to open XWS annotations for" << m_recordName;
        return false;
    }

    m_annotations.clear();
    WFDB_Annotation annot;

    while (getann(0, &annot) >= 0) {
        AnnotationData data;
        data.time = annot.time;
        data.timeResampled = annot.time;
        data.anntyp = annot.anntyp;
        data.symbol = QString::fromLatin1(annstr(annot.anntyp));
        data.description = QString::fromLatin1(anndesc(annot.anntyp));
        data.subtype = annot.subtyp;
        data.channel = annot.chan;
        data.number = annot.num;  // In XWS, 'num' often indicates wave type
        data.aux = (annot.aux != NULL) ? QString::fromLatin1(annot.aux) : "";
        m_annotations.append(data);
    }

    wfdbquit();
    m_originalFrequency = sampfreq(annInfo.name);
    return true;
}

const QVector<AnnotationData> &AnnotationReader::getAnnotations() const
{
    return m_annotations;
}

QVector<RRInterval> AnnotationReader::computeRRIntervals(
    const QVector<AnnotationData> &annotations,
    int samplingRate) {

    QVector<RRInterval> rrIntervals;
    QVector<AnnotationData> qrsBeats;

    // Extract QRS annotations - FIXED: use anntyp, not number
    for (const auto& ann : annotations)
        if (wfdb_isqrs(ann.anntyp))
            qrsBeats.append(ann);

    qDebug() << "Found" << qrsBeats.size() << "QRS beats";

    if (qrsBeats.size() < 2) {
        qWarning() << "Not enough QRS beats to compute RR intervals";
        return rrIntervals;
    }

    // Compute intervals between consecutive beats
    for (int i = 1; i < qrsBeats.size(); i++) {
        RRInterval rr;
        rr.time1 = qrsBeats[i-1].time;
        rr.time2 = qrsBeats[i].time;
        rr.interval = rr.time2 - rr.time1;
        rr.intervalSeconds = static_cast<double>(rr.interval) / samplingRate;
        rrIntervals.append(rr);
    }

    qDebug() << "Computed" << rrIntervals.size() << "RR intervals";
    return rrIntervals;
}

void AnnotationReader::applyResampling(double newFrequency) {
    if (m_originalFrequency == 0.0) {
        qWarning() << "Original frequency not set. Cannot resample.";
        return;
    }

    resampleAnnotations(m_annotations, m_originalFrequency, newFrequency);
    m_resampledFrequency = newFrequency;
}

QVector<AnnotationData> AnnotationReader::getResampledAnnotations() const {
    QVector<AnnotationData> resampled;
    for (const auto& ann : m_annotations) {
        AnnotationData copy = ann;
        copy.time = ann.timeResampled;
        resampled.append(copy);
    }
    return resampled;
}

long AnnotationReader::resampleTime(long oldSample, double oldFreq, double newFreq) {
    // Calculate new sample position
    double newSample = oldSample * (newFreq / oldFreq);
    // Round to nearest integer
    return static_cast<long>(std::round(newSample));
}

void AnnotationReader::resampleAnnotations(QVector<AnnotationData> &annotations, double oldFreq, double newFreq) {
    if (oldFreq == newFreq) {
        qDebug() << "Sampling frequencies are the same, no resampling needed.";
        return;
    }
    qDebug() << "Resampling annotations from" << oldFreq << "Hz to" << newFreq << "Hz";

    int resampledCount = 0;
    for (auto& ann : annotations) {
        ann.timeResampled = resampleTime(ann.time, oldFreq, newFreq);
        resampledCount++;
    }
}
