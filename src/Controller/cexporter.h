#ifndef CEXPORTER_H
#define CEXPORTER_H

#include <QObject>
#include <Models/sample.h>
#include <Models/global_qcardio.h>
#include <QFileDialog>
class CExporter : public QObject
{
    Q_OBJECT
public:
    explicit CExporter(QObject *parent = nullptr);
    void exportData(const MIT_BIH_ECGData& data, const ExprotSetting &exportSetting);

private:
    void exportDataInSample(const MIT_BIH_ECGData& data, const ExprotSetting &exportSetting);
    bool exportDataInRC7(const MIT_BIH_ECGData& data, const ExprotSetting &exportSetting);
    void saveCSV(const QString &filePath, const MIT_BIH_ECGData &data);

signals:
};

#endif // CEXPORTER_H
