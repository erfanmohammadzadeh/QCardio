#include "appcontroller.h"

#include <QProgressBar>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>
#include <QThread>
#include <QFutureWatcher>
#include <QSharedPointer>
#include <QAtomicInt>

AppController::AppController(QObject *parent)
    : QObject{parent}
{
    createObj();
    initConnection();

    m_mainWindow->show();
    m_mainWindow->setSignalWidget(m_signalViewController->signalWidget());
    m_settingsService->loadSetting(m_uiConfig);
    m_mainWindow->loadUIConfig(m_uiConfig);
    m_mainWindow->setSetting(m_settingsService);
}

AppController::~AppController()
{
    m_mainWindow->deleteLater();
    m_signalViewController->deleteLater();
    delete m_uiConfig;
}

void AppController::initConnection()
{
    connect(m_mainWindow, &MainWindow::sigReadDataRequested, this, &AppController::sltOpenRecord);
    connect(this, &AppController::sigReadAutomaticDataRequested, this, &AppController::sltOpenRecord);
    connect(m_mainWindow, &MainWindow::sigExportRequested, this, &AppController::sltExportRequested);
    connect(m_mainWindow, &MainWindow::sigExportAllRequested, this, &AppController::sltExportAllRequested);
    connect(this, &AppController::sigReadDataProcessEnd, this, &AppController::sltReadDataProcessEnd);
    connect(this, &AppController::sigExportProcessEnd, this, [=]() {
        QMessageBox::information(nullptr, "Export", "Export Process Completed");
    });
    connect(m_mainWindow, &MainWindow::sigAnalyseRequested, this, &AppController::sltAnalyseRequested);
    connect(m_mainWindow, &MainWindow::sigAppendLog, m_logService, &LogService::sltAppendLog);
    connect(this, &AppController::sigAppendLog, m_logService, &LogService::sltAppendLog);
    connect(m_exportService, &ExportService::sigExportProcessEnd, this, [=]() {
        m_mainWindow->enableUIBtn(true);
    });
    connect(m_wfdbService, &WfdbService::sigAppendLog, m_logService, &LogService::sltAppendLog);
    connect(m_exportService, &ExportService::sigAppendLog, m_logService, &LogService::sltAppendLog);
}

void AppController::createObj()
{
    m_mainWindow = new MainWindow;
    m_wfdbService = new WfdbService(this);
    m_signalViewController = new SignalViewController();
    m_exportService = new ExportService(this);
    m_settingsService = new SettingsService(this);
    m_uiConfig = new UIConfigs();
    m_logService = new LogService(this, m_mainWindow->getLogFiledWidget());
}

void AppController::sltOpenRecord(const SignalViewParameters& params)
{
    QtConcurrent::run(QThreadPool::globalInstance(), [=] {
        bool success = m_wfdbService->readData(params);
        success = m_wfdbService->readAnot(params) && success;
        Q_EMIT sigReadDataProcessEnd();
        return success;
    });
}

void AppController::sltExportRequested(const ExprotSetting &exportSetting)
{
    QtConcurrent::run(QThreadPool::globalInstance(), [=] {
        m_exportService->exportData(m_wfdbService->getStructData(), exportSetting);
        Q_EMIT sigExportProcessEnd();
    });
}

void AppController::sltExportAllRequested(const ExprotSetting& setting)
{
    QProgressBar *progressBar = new QProgressBar(m_mainWindow);
    progressBar->setRange(0, setting.pathList.size());
    progressBar->setValue(0);
    progressBar->setVisible(true);
    m_mainWindow->addProgressBar(progressBar);

    QSharedPointer<QAtomicInt> counter = QSharedPointer<QAtomicInt>::create(0);
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>();

    QStringList pathCopy = setting.pathList;
    SignalViewParameters settingCopy = setting.params;

    QFuture<void> future =
        QtConcurrent::run(QThreadPool::globalInstance(), [=]() mutable
                          {
                              for (const QString& id : pathCopy)
                              {
                                  bool success = false;
                                  int retryCount = 0;
                                  const int MAX_RETRIES = 3;

                                  settingCopy.signalFilePath = id;

                                  while (!success && retryCount < MAX_RETRIES)
                                  {
                                      Q_EMIT sigAppendLog(QString("Attempting to read %1 retry %2 count").arg(id).arg(retryCount));
                                      success = m_wfdbService->readData(settingCopy);
                                      success = m_wfdbService->readAnot(settingCopy) && success;
                                      if (success)
                                          m_signalViewController->setData(m_wfdbService->getStructData());

                                      if (!success) {
                                          retryCount++;
                                          Q_EMIT sigAppendLog(QString("Read failed for %1 retry %2").arg(id).arg(retryCount));
                                          QThread::msleep(50);
                                      }
                                  }

                                  if (success)
                                  {
                                      Q_EMIT sigAppendLog("Exporting:" + id);
                                      MIT_BIH_ECGData data = m_wfdbService->getStructData();
                                      data.filename = id;
                                      m_exportService->exportData(data, setting);
                                  }
                                  else
                                  {
                                      Q_EMIT sigAppendLog(QString("Failed to read data for %1 after %2 attempts.").arg(id).arg(MAX_RETRIES));
                                  }

                                  int current = counter->fetchAndAddOrdered(1) + 1;
                                  QMetaObject::invokeMethod(progressBar, "setValue",
                                                            Qt::QueuedConnection,
                                                            Q_ARG(int, current));
                              }
                          });

    connect(watcher, &QFutureWatcher<void>::finished,
            [=]() {
                progressBar->setVisible(false);
                progressBar->deleteLater();
                watcher->deleteLater();
                m_mainWindow->enableUIBtn(true);
            });

    watcher->setFuture(future);
}

void AppController::sltAnalyseRequested(const AnalyseCfg& analyseCfg)
{
    const int totalFiles = qMin(analyseCfg.csvPath1.size(), analyseCfg.csvPath2.size());

    QProgressBar *progressBar = new QProgressBar(m_mainWindow);
    progressBar->setRange(0, totalFiles);
    progressBar->setValue(0);
    progressBar->setFormat(QStringLiteral("Compare %v/%m"));
    progressBar->setTextVisible(true);
    progressBar->setMinimumWidth(180);
    progressBar->setVisible(true);
    m_mainWindow->addProgressBar(progressBar);
    m_mainWindow->enableUIBtn(false);

    QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>(this);
    QFuture<bool> future =
        QtConcurrent::run(QThreadPool::globalInstance(), [=]() {
            AnalyseService analyser(nullptr, analyseCfg);
            connect(&analyser, &AnalyseService::sigAppendLog,
                    m_logService, &LogService::sltAppendLog, Qt::QueuedConnection);
            connect(&analyser, &AnalyseService::sigProgress,
                    progressBar,
                    [progressBar](int current, int total) {
                        progressBar->setRange(0, total);
                        progressBar->setValue(current);
                    },
                    Qt::QueuedConnection);

            return analyser.run();
        });

    connect(watcher, &QFutureWatcher<bool>::finished, this, [=]() {
        const bool success = watcher->result();
        progressBar->setVisible(false);
        progressBar->deleteLater();
        watcher->deleteLater();
        m_mainWindow->enableUIBtn(true);

        if (success) {
            QMessageBox::information(m_mainWindow,
                                     QStringLiteral("Compare"),
                                     QStringLiteral("Compare process completed"));
        } else {
            QMessageBox::warning(m_mainWindow,
                                 QStringLiteral("Compare"),
                                 QStringLiteral("Compare failed. Check CSV paths and file contents."));
        }
    });

    watcher->setFuture(future);
}

void AppController::sltReadDataProcessEnd()
{
    m_signalViewController->setData(m_wfdbService->getStructData());
    m_mainWindow->enableUIBtn(true);
}
