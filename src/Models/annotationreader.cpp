#include "annotationreader.h"

AnnotationReader::AnnotationReader(const QString &dbPath, const QString &recordName, const QString &annotatorName)
    :m_databaseName(dbPath), m_recordName(recordName), m_annotatorName(annotatorName)
{
    setwfdb(const_cast<char*>(dbPath.toStdString().c_str()));
}

// bool AnnotationReader::loadAnnotations() {
//     qDebug() << "Record:" << m_recordName;
//     m_annotatorName = "atr";

//     QByteArray recordBA = m_recordName.toLatin1();
//     QByteArray annotatorBA = m_annotatorName.toLatin1();

//     WFDB_Anninfo annInfo;
//     annInfo.name = (char*)annotatorBA.constData();
//     annInfo.stat = WFDB_READ;//for aha this should change to AHA

//     if (annopen((char*)recordBA.constData(), &annInfo, 1) < 0) {
//         qCritical() << "Failed to open XWS annotations for" << m_recordName;
//         return false;
//     }

//     m_annotations.clear();
//     WFDB_Annotation annot;

//     while (getann(0, &annot) >= 0) {
//         AnnotationData data;
//         data.time = annot.time;
//         qDebug() << "drs "<< annot.time;
//         data.timeResampled = annot.time;
//         data.anntyp = annot.anntyp;
//         data.symbol = QString::fromLatin1(annstr(annot.anntyp));
//         data.description = QString::fromLatin1(anndesc(annot.anntyp));
//         data.subtype = annot.subtyp;
//         data.channel = annot.chan;
//         data.number = annot.num;  // In XWS, 'num' often indicates wave type
//         data.aux = (annot.aux != NULL) ? QString::fromLatin1(annot.aux) : "";
//         m_annotations.append(data);
//     }

//     wfdbquit();
//     m_originalFrequency = //sampfreq(annInfo.name);
//     return true;
// }

bool AnnotationReader::loadAnnotations() {
    qDebug() << "Record:" << m_recordName;
    m_annotatorName = "atr";

    QByteArray recordBA = m_recordName.toLatin1();
    QByteArray annotatorBA = m_annotatorName.toLatin1();

    // 1. Read the sampling frequency first using the record name
    double freq = sampfreq((char*)recordBA.constData());
    if (freq < 0) {
        qWarning() << "Could not read sampling frequency for" << m_recordName << ". Defaulting to 360 Hz.";
        m_originalFrequency = 360.0; // MIT-BIH Arrhythmia Database default
    } else {
        m_originalFrequency = freq;
        qDebug() << "Original Frequency:" << m_originalFrequency << "Hz";
    }

    WFDB_Anninfo annInfo;
    annInfo.name = (char*)annotatorBA.constData();
    annInfo.stat = WFDB_READ;

    if (annopen((char*)recordBA.constData(), &annInfo, 1) < 0) {
        qCritical() << "Failed to open annotations for" << m_recordName;
        return false;
    }

    m_annotations.clear();
    WFDB_Annotation annot;

    // 2. Strict == 0 check to prevent parsing junk data at EOF
    while (getann(0, &annot) == 0) {
        // Double check against corrupted/negative time tokens
        if (annot.time < 0) {
            continue;
        }

        AnnotationData data;
        data.time = annot.time;
        data.timeResampled = annot.time;
        data.anntyp = annot.anntyp;
        data.symbol = QString::fromLatin1(annstr(annot.anntyp));
        data.description = QString::fromLatin1(anndesc(annot.anntyp));
        data.subtype = annot.subtyp;
        data.channel = annot.chan;
        data.number = annot.num;
        data.aux = (annot.aux != NULL) ? QString::fromLatin1(annot.aux) : "";
        m_annotations.append(data);
    }

    wfdbquit();
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
