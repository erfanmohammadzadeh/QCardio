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

void CSV::saveCSV()
{
    QFile file(m_filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    // Find minimum time for shifting
    long minTime = 0;
    for (const auto& ann : std::as_const(m_csvFormat.sampleIndex)) {
        if (ann < minTime) minTime = ann;
    }

    int numAnnotations = m_csvFormat.sampleIndex.size();

    for (int i = numAnnotations-1; i >= 0; i--)
    {
        long sampleIndex = m_csvFormat.sampleIndex.at(i) - minTime;
        out << sampleIndex/178 << ","
            << m_csvFormat.type.at(i) << "\n";
    }

    file.close();
}

QString CSV::getFilename() const
{
    return m_filename;
}

void CSV::comparesFile(const CSVFormat &csv1, const CSVFormat &csv2)
{
    m_csvFormat.sampleIndex.clear();
    m_csvFormat.type.clear();

    const int rowSize = qMin(csv1.sampleIndex.size(), csv2.sampleIndex.size());
    for (int i = 0; i < rowSize; ++i) {
        m_csvFormat.sampleIndex << qAbs(csv1.sampleIndex.at(i) - csv2.sampleIndex.at(i));
        m_csvFormat.type << static_cast<quint8>(csv1.type.at(i) == csv2.type.at(i) ? 1 : 0);
    }

    saveCSV();
}