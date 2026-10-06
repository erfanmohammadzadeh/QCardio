#include "csv.h"
#include "Models/performancereport.h"

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
        if (line.isEmpty())
        {
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
    int numAnnotations = m_csvFormat.sampleIndex.size();

    for (int i = 0; i < numAnnotations; ++i)
    {
        long sampleIndex = (m_csvFormat.sampleIndex.at(i) - minTime);
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

    for(int i = 0; i < numAnnotations; i++)
    {
        const int diff = res.difIndexList.at(i);
        const bool isMatch = diff != -1;

        out << res.alignedList1.at(i) << ","
            << res.alignedList2.at(i) << ","
            << diff << ","
            << (isMatch ? "QRS Match" : "QRS not Match") << ","
            << res.typeAlignList1.at(i) << ","
            << res.typeAlignList2.at(i) << ","
            << (res.difTypeList.at(i) ? "Type Match" : "Type Not Match") << "\n";
    }
    file.close();
    return true;
}

bool CSV::saveProcessFileResult(const AnalyseFileProcessResult &fileProcessRes)
{
    return PerformanceReport::write(m_filename, fileProcessRes);
}



QString CSV::getFilename() const
{
    return m_filename;
}
