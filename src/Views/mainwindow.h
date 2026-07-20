#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <Models/directoryvalidator.h>
#include <Views/signalviewwidget.h>
#include <QFileInfo>
#include "Models/uiconfigs.h"
#include "Controller/csettings.h"
#include <QListWidget>
#include <QTextEdit>
#include <Models/define.h>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setSignalWidget(SignalViewWidget *signalWidget);
    void updateRecordList(const QString &recordDirectory);
    void loadStyle();
    void enableUIBtn(const bool &isEnable);
    void loadUIConfig(UIConfigs *uiConfig);
    void getUIConfig(UIConfigs &uiConfig);

    void setSetting(CSettings *newSetting);
    void addFileToListWidget(QListWidget *widget,
                             QStringList &filePaths,
                             QStringList &fileNames,
                             const QString& type,
                             const QString &path);
    // void addFileToListWidget(QListWidget *windget, QStringList &filepath, QStringList& filename , const QString &type, const QString &path);
    QTextEdit *getLogFiledWidget();

private slots:
    void on_pushButtonSetPath_clicked();

    void on_pushButtonRead_clicked();

    void on_listWidgetItems_currentRowChanged(int currentRow);

    void on_pushButtonExport_clicked();


    void on_pushButtonUpdate_clicked();

    void on_pushButtonData1_clicked();

    void on_pushButtonData2_clicked();

    void on_pushButtonCompare_clicked();

    void on_toolButtonClearLogs_clicked();

private:
    Ui::MainWindow *ui;

    void setupSignal(QVBoxLayout *mainLayout, SignalViewWidget *signalWidget);
    void saveSetting();
    ExprotSetting::ExportMethod getExportMethod();
    bool readSignalSetting(SignalViewParameters &params);
    int m_listItemIdx = 0;
    QStringList m_heaFilesWithPath;
    QStringList m_headerFilePath;

    QStringList m_CSV1FilesWithPath;
    QStringList m_CSV1FilePath;

    QStringList m_CSV2FilesWithPath;
    QStringList m_CSV2FilePath;

    CSettings *m_setting;
    quint8 m_counterLog = 0;
    QString m_databasePath;

Q_SIGNALS:
    void sigReadDataRequested(const SignalViewParameters& params);
    void sigExportRequested(const ExprotSetting& path);
    void sigExportAllRequested(const ExprotSetting &setting);
    void sigAnalyseRequested(const AnalyseCfg cfg);
    void sigAppendLog(const QString& text);

protected:
    void closeEvent(QCloseEvent*event) override;

};
#endif // MAINWINDOW_H
