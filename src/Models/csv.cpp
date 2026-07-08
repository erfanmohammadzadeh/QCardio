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
    QtConcurrent::run(QThreadPool::globalInstance(), [=](){
        loadData(m_filename);
        Q_EMIT sigReadyForRead();
    });
}

void CSV::loadData(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList fields = line.split(',');

        if (fields.size() >= 2)
        {
            bool ok1, ok2;
            int sampleIndex = fields[0].toInt(&ok1);
            int type = fields[1].toInt(&ok2);

            if (!ok1 || !ok2)
            {
                // Log error or handle invalid data
                continue;
            }

            // Append to vectors
            m_csvFormat.sampleIndex.append(sampleIndex);
            m_csvFormat.type.append(static_cast<unsigned char>(type));
        }
    }
    file.close();
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
    CSVResult result;
    int rowSize = qMin(csv1.sampleIndex.size(), csv2.sampleIndex.size());
    for(int i = 0; i < rowSize; i++)
    {
        result.difSampleIndex << qAbs(csv1.sampleIndex.at(i) - csv2.sampleIndex.at(i));
        result.isTypeEqual << (csv1.type.at(i) == csv2.type.at(2));
    }
    saveCSV();
}