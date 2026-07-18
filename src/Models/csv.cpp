#include "csv.h"

CSV::CSV(QObject *parent, QString filename, CSVFormat csvData)
    : QObject{parent}, m_filename(filename), m_csvFormat(csvData)
{
}

CSVFormat CSV::csvFormat() const
{
    return m_csvFormat;
}

void CSV::loadRequested()
{
    QtConcurrent::run(QThreadPool::globalInstance(), [=]() {
        loadFromFile(m_filename);
        Q_EMIT sigReadyForRead();
    });
}

bool CSV::loadFromFile(const QString &path)
{
    m_csvFormat.sampleIndex.clear();
    m_csvFormat.type.clear();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open CSV:" << path;
        return false;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }

        const QStringList fields = line.split(',');
        if (fields.size() < 2) {
            continue;
        }

        bool ok1 = false;
        bool ok2 = false;
        const int sampleIndex = fields[0].toInt(&ok1);
        const int type = fields[1].toInt(&ok2);

        if (!ok1 || !ok2) {
            continue;
        }

        m_csvFormat.sampleIndex.append(sampleIndex);
        m_csvFormat.type.append(static_cast<quint8>(type));
    }

    return true;
}

void CSV::loadData(const QString &path)
{
    loadFromFile(path);
}

void CSV::saveSignal()
{
    QFile file(m_filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    // Find minimum time for shifting
    long minTime = 0;
    // for (const auto& ann : std::as_const(m_csvFormat.sampleIndex)) {
    //     if (ann < minTime) minTime = ann;
    // }

    int numAnnotations = m_csvFormat.sampleIndex.size();

    for (int i = 0; i < numAnnotations; ++i)
    {
        long sampleIndex = (m_csvFormat.sampleIndex.at(i) - minTime);
        // qDebug() << "sav "<< sampleIndex;
        out << sampleIndex << ","
            << m_csvFormat.type.at(i) << "\n";
    }

    file.close();
}

bool CSV::saveRawCSVRes(const SheetResult &res)
{
    QFile file(m_filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    int numAnnotations = res.alignedList1.size(); // Use the final aligned size
    qDebug() << "Total aligned rows: " << numAnnotations;

    for(int i = 0; i < numAnnotations; i++)
    {
        int diff = res.difIndexList.at(i);

        // FIX 2 & 3: Match only if it's NOT a desync marker (-1) AND within tolerance limit
        bool isMach = (diff != -1) && ((diff / 178) < 3);

        out << res.alignedList1.at(i) << ","
            << res.alignedList2.at(i) << ","
            << diff << ","
            << (isMach ? "QRS Macth" : "QRS not Macth") << ","
            << res.typeAlignList1.at(i) << ","
            << res.typeAlignList2.at(i) << ","
            << (res.difTypeList.at(i) ? "Type Macth" : "Type Not Macth")        << "\n";
    }
    file.close();
    return true;
}

bool CSV::saveProcessFileResult(const QVector<FileProcessResult> &fileProcessRes)
{
    QFile file(m_filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << "FileName,QRS_TP,QRS_FP,QRS_FN,QRS_Se,QRS_P+,QRS_Accuracy,"
           "Normal_TP,Normal_FP,Normal_FN,Normal_Se,Normal_P+,Normal_Accuracy,"
           "PVC_TP,PVC_FP,PVC_FN,PVC_Se,PVC_P+,PVC_Accuracy\n";

    for (const FileProcessResult &res : fileProcessRes)
    {
        out << res.fileName << ","
            << res.qrsPredict.tp << ","
            << res.qrsPredict.fp << ","
            << res.qrsPredict.fn << ","
            << res.qrsPredict.se << ","
            << res.qrsPredict.p << ","
            << res.qrsPredict.accuracy << ","
            << res.normalPredict.tp << ","
            << res.normalPredict.fp << ","
            << res.normalPredict.fn << ","
            << res.normalPredict.se << ","
            << res.normalPredict.p << ","
            << res.normalPredict.accuracy << ","
            << res.pvcPredict.tp << ","
            << res.pvcPredict.fp << ","
            << res.pvcPredict.fn << ","
            << res.pvcPredict.se << ","
            << res.pvcPredict.p << ","
            << res.pvcPredict.accuracy << ","
            << "\n";

        // Check for write errors
        if (out.status() != QTextStream::Ok)
        {
            file.close();
            return false;
        }
    }

    file.close();
    return true;
}
QString CSV::getFilename() const
{
    return m_filename;
}
