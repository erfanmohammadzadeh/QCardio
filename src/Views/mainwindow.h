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
    void setSignalWidget(const QList<SignalViewWidget *> widgetList);
    void updateRecordList(const QString &recordDirectory);
    void loadStyle();
    void enableUIBtn(const bool &isEnable);
    void loadUIConfig(const UIConfigs& uiConfig);
    void getUIConfig(UIConfigs &uiConfig);

    void setSetting(CSettings *newSetting);

private slots:
    void on_pushButtonSetPath_clicked();

    void on_pushButtonRead_clicked();

    void on_listWidgetItems_currentRowChanged(int currentRow);

    void on_pushButtonExport_clicked();


    void on_pushButtonUpdate_clicked();

private:
    Ui::MainWindow *ui;

    void setupSignal(QGridLayout* mainLayout, SignalViewWidget* signalWidget);
    void saveSetting();
    ExprotSetting::ExportMethod getExportMethod();
    SignalViewParameters readSignalSetting();
    int m_listItemIdx = 0;
    QStringList m_heaFilesWithPath;
    QStringList m_headerFilePath;
    CSettings *m_setting;

Q_SIGNALS:
    void sigReadDataRequested(const SignalViewParameters& params);
    void sigExportRequested(const ExprotSetting& path);
    void sigExportAllRequested(const ExprotSetting &setting);

protected:
    void closeEvent(QCloseEvent*event) override;
};
#endif // MAINWINDOW_H
