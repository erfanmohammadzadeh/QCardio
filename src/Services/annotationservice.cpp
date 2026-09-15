#include "annotationservice.h"

#include <QSet>

AnnotationService::AnnotationService(QObject* parent,
                                     const SignalViewParameters &signalParam,
                                     const QString &annotatorName)
    : QObject(parent)
    , m_databaseName(signalParam.dbPath)
    , m_recordName(signalParam.signalFilePath)
    , m_annotatorName(annotatorName)
{
    setwfdb(const_cast<char*>(signalParam.dbPath.toStdString().c_str()));
}

bool AnnotationService::loadAnnotations()
{
    Q_EMIT sigAppendLog("Record Atribute: " + m_recordName);
    m_annotatorName = "atr";

    QByteArray recordBA = m_recordName.toLatin1();
    QByteArray annotatorBA = m_annotatorName.toLatin1();

    double freq = sampfreq((char*)recordBA.constData());
    if (freq < 0) {
        m_originalFrequency = m_sourceFreq;
        Q_EMIT sigAppendLog("Could not read sampling frequency for" + m_recordName + QString(". Defaulting to %1 Hz.").arg(m_sourceFreq));
    } else {
        m_originalFrequency = freq;
        Q_EMIT sigAppendLog(QString("Original Frequency %1Hz").arg(m_originalFrequency));
    }

    WFDB_Anninfo annInfo;
    annInfo.name = (char*)annotatorBA.constData();
    annInfo.stat = WFDB_READ;

    if (annopen((char*)recordBA.constData(), &annInfo, 1) < 0) {
        Q_EMIT sigAppendLog("Failed to open annotations for" + m_recordName);
        return false;
    }

    m_annotations.clear();
    WFDB_Annotation annot;

    while (getann(0, &annot) == 0) {
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
        if (annot.aux) {
            int len = annot.aux[0];
            data.aux = QString::fromLatin1(
                reinterpret_cast<const char*>(annot.aux + 1),
                len);
        }
        m_annotations.append(data);
    }
    wfdbquit();
    return true;
}

const QVector<AnnotationData> &AnnotationService::getAnnotations() const
{
    return m_annotations;
}

const static QSet<QString> noneBeatChar = {
    "[", "!", "]", "x", "(", ")", "p", "t", "u",
    "`", "'", "^", "|", "~", "+", "s", "T", "*",
    "D", "=", "@"
};

void AnnotationService::calcNoneBeatIndex(MIT_BIH_ECGData &data)
{
    for (int i = m_annotations.size() - 1; i >= 0; --i) {
        if (noneBeatChar.contains(m_annotations[i].symbol)) {
            for (auto &signal : data.nsigs)
                if (i < signal.size())
                    data.noneBeatIndex.append(i);
        }
    }
}

QVector<RRInterval> AnnotationService::computeRRIntervals(
    const QVector<AnnotationData> &annotations,
    int samplingRate)
{
    QVector<RRInterval> rrIntervals;
    QVector<AnnotationData> qrsBeats;

    for (const auto& ann : annotations)
        if (wfdb_isqrs(ann.anntyp))
            qrsBeats.append(ann);

    Q_EMIT sigAppendLog(QString("%1 QRS index Found").arg(qrsBeats.size()));

    if (qrsBeats.size() < 2) {
        Q_EMIT sigAppendLog("Not enough QRS beats to compute RR intervals");
        return rrIntervals;
    }

    for (int i = 1; i < qrsBeats.size(); i++) {
        RRInterval rr;
        rr.time1 = qrsBeats[i-1].time;
        rr.time2 = qrsBeats[i].time;
        rr.interval = rr.time2 - rr.time1;
        rr.intervalSeconds = static_cast<double>(rr.interval) / samplingRate;
        rrIntervals.append(rr);
    }

    Q_EMIT sigAppendLog(QString("%1 RR intervals computed").arg(rrIntervals.size()));
    return rrIntervals;
}
