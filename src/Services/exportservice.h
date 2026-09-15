#ifndef EXPORTSERVICE_H
#define EXPORTSERVICE_H

#include <QObject>
#include "Models/sample.h"
#include "Models/global_qcardio.h"

class ExportService : public QObject
{
    Q_OBJECT
public:
    explicit ExportService(QObject *parent = nullptr);
    void exportData(const MIT_BIH_ECGData& data, const ExprotSetting &exportSetting);

signals:
    void sigExportProcessEnd();
    void sigAppendLog(const QString& text);

private:
    void exportDataInSample(const MIT_BIH_ECGData& data, const ExprotSetting &exportSetting);
    bool exportDataInRC7(const MIT_BIH_ECGData& data, const ExprotSetting &exportSetting);
};

#endif // EXPORTSERVICE_H
