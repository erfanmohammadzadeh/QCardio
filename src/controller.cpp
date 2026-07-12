#include "controller.h"
#include <QProgressBar>
#include <QMessageBox>

Controller::Controller(QObject *parent)
    : QObject{parent}
{
    m_mainWindow.show();
    connect(&m_mainWindow, &MainWindow::sigReadDataRequested,
            this, &Controller::sltOpenRecord);

    connect(this, &Controller::sigReadAutomaticDataRequested,
            this, &Controller::sltOpenRecord);

    connect(&m_mainWindow, &MainWindow::sigExportRequested,
            this, &Controller::sltExportRequested);

    connect(&m_mainWindow, &MainWindow::sigExportAllRequested,
            this, &Controller::sltExportAllRequested);

    connect(this, &Controller::sigReadDataProcessEnd, this,[=]()
            {
                m_csignalView->setData(m_cwfdb->getStructData());
                m_mainWindow.enableUIBtn(true);
            });
    connect(this, &Controller::sigExportProcessEnd, this,[=]()
            {
                QMessageBox::information(nullptr, "Export", "Export Process Completed");
            });
    connect(&m_mainWindow, &MainWindow::sigAnalyseRequested, this, &Controller::sltAnalyseRequested);

    m_cwfdb = new Cwfdb(this);
    m_csignalView = new CSignalView();
    m_cexporter = new CExporter(this);

    connect(m_cexporter, &CExporter::sigExportProcessEnd, this,[=](){m_mainWindow.enableUIBtn(true);});

    m_mainWindow.setSignalWidget(m_csignalView->signalWidget());
    m_csetting.loadSetting(m_uiConfig);
    m_mainWindow.loadUIConfig(m_uiConfig);
    m_mainWindow.setSetting(&m_csetting);
}

Controller::~Controller()
{
    m_csignalView->deleteLater();
}

void Controller::sltOpenRecord(const SignalViewParameters& params)
{
    QtConcurrent::run(QThreadPool::globalInstance(),[=] {
        bool success = m_cwfdb->readData(params);
        success = m_cwfdb->readAnot(params) && success;
        Q_EMIT sigReadDataProcessEnd();
        return success;  // Return the result
    });
}

void Controller::sltExportRequested(const ExprotSetting &exportSetting)
{
    QtConcurrent::run(QThreadPool::globalInstance(),[=] {
        m_cexporter->exportData(m_cwfdb->getStructData(), exportSetting);
        Q_EMIT sigExportProcessEnd();
    });
}

void Controller::sltExportAllRequested(const ExprotSetting& setting)
{
    QProgressBar *progressBar = new QProgressBar(&m_mainWindow);
    progressBar->setRange(0, setting.pathList.size());
    progressBar->setValue(0);
    progressBar->setVisible(true);

    QSharedPointer<QAtomicInt> counter = QSharedPointer<QAtomicInt>::create(0);
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>();

    QStringList pathCopy = setting.pathList;
    SignalViewParameters settingCopy = setting.params;

    QFuture<void> future = QtConcurrent::run(QThreadPool::globalInstance(),
                                             [=]() mutable
                                             {
                                                 for(const QString& id : pathCopy)
                                                 {
                                                     bool success = false;
                                                     int retryCount = 0;
                                                     const int MAX_RETRIES = 3;  // Reduced retries for testing

                                                     // Keep reading until success or max retries
                                                     settingCopy.signalFilePath = id;

                                                     while (!success && retryCount < MAX_RETRIES)
                                                     {
                                                         qDebug() << "Attempting to read:" << id << "retry" << retryCount;

                                                         // Directly call readData instead of using signal
                                                         success = m_cwfdb->readData(settingCopy);
                                                         success = m_cwfdb->readAnot(settingCopy) && success;
                                                         if(success) m_csignalView->setData(m_cwfdb->getStructData());

                                                         if (!success)
                                                         {
                                                             retryCount++;
                                                             qDebug() << "Read failed for" << id << "retry" << retryCount;

                                                             // Optional: Wait before retrying
                                                             QThread::msleep(50);
                                                         }
                                                     }

                                                     if (success)
                                                     {
                                                         // Only export if read was successful
                                                         qDebug() << "Exporting:" << id;
                                                         MIT_BIH_ECGData data = m_cwfdb->getStructData();
                                                         data.filename = id;
                                                         m_cexporter->exportData(data, setting);
                                                     }
                                                     else
                                                     {
                                                         qDebug() << "Failed to read data for" << id << "after" << MAX_RETRIES << "attempts";
                                                         // Handle failure - maybe skip this file or log error
                                                     }

                                                     // Update progress regardless of success/failure
                                                     int current = counter->fetchAndAddOrdered(1) + 1;
                                                     QMetaObject::invokeMethod(progressBar, "setValue",
                                                                               Qt::QueuedConnection,
                                                                               Q_ARG(int, current));
                                                 }
                                             }
                                             );

    connect(watcher, &QFutureWatcher<void>::finished,
            [=]() {
                progressBar->setVisible(false);
                progressBar->deleteLater();
                watcher->deleteLater();
                m_mainWindow.enableUIBtn(true);
            });

    watcher->setFuture(future);
}

void Controller::sltAnalyseRequested(const AnalyseCfg& analyseCfg)
{
    QtConcurrent::run(QThreadPool::globalInstance(), [=]() {
        CAnalyser analyser(nullptr, analyseCfg);
        const bool success = analyser.analyse();

        QMetaObject::invokeMethod(
            &m_mainWindow,
            [success]() {
                if (success) {
                    QMessageBox::information(nullptr,
                                             QStringLiteral("Compare"),
                                             QStringLiteral("Compare process completed"));
                } else {
                    QMessageBox::warning(nullptr,
                                        QStringLiteral("Compare"),
                                        QStringLiteral("Compare failed. Check CSV paths and file contents."));
                }
            },
            Qt::QueuedConnection);
    });
}