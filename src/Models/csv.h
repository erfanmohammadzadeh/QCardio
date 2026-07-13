#ifndef CSV_H
#define CSV_H

#include <QObject>
#include <Models/global_qcardio.h>
#include <QtConcurrent/QtConcurrent>
class CSV : public QObject
{
    Q_OBJECT
public:
    explicit CSV(QObject *parent = nullptr, QString filename = "", CSVFormat csvData=CSVFormat());

    CSVFormat csvFormat() const;
    bool loadFromFile(const QString &path);
    void loadRequested();
    void saveSignal();
    void saveRawCSV(const CSVFormat &csv1, const CSVFormat &csv2);
    void comparesFile(const CSVFormat& csv1, const CSVFormat& csv2);
    QString getFilename() const;

private:
    QString m_filename;
    CSVFormat m_csvFormat;
    void loadData(const QString& path);

signals:
    void sigReadyForRead();
};

#endif // CSV_H
