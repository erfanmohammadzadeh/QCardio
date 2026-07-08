#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include "Views/mainwindow.h"
#include "./Controller/cwfdb.h"
#include "./Controller/csignalview.h"
#include "./Controller/cexporter.h"
#include <QtConcurrent/QtConcurrent>
#include <QGuiApplication>
#include "Controller/csettings.h"
#include "Controller/canalyser.h"
class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    MainWindow m_mainWindow;
    Cwfdb *m_cwfdb = nullptr;
    CSignalView* m_csignalView = nullptr;
    CExporter* m_cexporter = nullptr;
    UIConfigs m_uiConfig;
    CSettings m_csetting;

signals:
    void sigReadDataProcessEnd();
    void sigExportProcessEnd();
    void sigReadAutomaticDataRequested(const SignalViewParameters& params);

public Q_SLOTS:
    void sltOpenRecord(const SignalViewParameters& params);
    void sltExportRequested(const ExprotSetting &path);
    void sltExportAllRequested(const ExprotSetting &setting);
    void sltAnalyseRequested(const AnalyseCfg &analyseCfg);


};

#endif // CONTROLLER_H
