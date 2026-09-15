#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include "Views/mainwindow.h"
#include "Controllers/signalviewcontroller.h"
#include "Services/wfdbservice.h"
#include "Services/exportservice.h"
#include "Services/settingsservice.h"
#include "Services/analyseservice.h"
#include "Services/logservice.h"

class AppController : public QObject
{
    Q_OBJECT
public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

signals:
    void sigReadDataProcessEnd();
    void sigExportProcessEnd();
    void sigReadAutomaticDataRequested(const SignalViewParameters& params);
    void sigAppendLog(const QString& text);

public Q_SLOTS:
    void sltOpenRecord(const SignalViewParameters& params);
    void sltExportRequested(const ExprotSetting &path);
    void sltExportAllRequested(const ExprotSetting &setting);
    void sltAnalyseRequested(const AnalyseCfg &analyseCfg);
    void sltReadDataProcessEnd();

private:
    void initConnection();
    void createObj();

    MainWindow *m_mainWindow = nullptr;
    WfdbService *m_wfdbService = nullptr;
    SignalViewController *m_signalViewController = nullptr;
    ExportService *m_exportService = nullptr;
    UIConfigs *m_uiConfig = nullptr;
    SettingsService *m_settingsService = nullptr;
    LogService *m_logService = nullptr;
};

#endif // APPCONTROLLER_H
