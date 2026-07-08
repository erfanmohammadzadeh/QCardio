#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QSlider>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    updateRecordList(ui->lineEditPath->text());
    loadStyle();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonSetPath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(nullptr,
                                                     "Select Database Path",
                                                     QDir::homePath());
    updateRecordList(path);
    ui->lineEditPath->setText(path);
}


void MainWindow::on_pushButtonRead_clicked()
{
    enableUIBtn(false);

    QString path = ui->lineEditPath->text();
    if(DirectoryValidator::validateDirectory(path))
    {
        SignalViewParameters params = readSignalSetting();
        Q_EMIT sigReadDataRequested(params);
    }
    else
        QMessageBox::critical(nullptr, "Database Path", "Path is not readable");
}

void MainWindow::setupSignal(QGridLayout *mainLayout, SignalViewWidget* signalView)
{
    mainLayout->addWidget(signalView);
}

void MainWindow::saveSetting()
{
    UIConfigs uiConfig;
    getUIConfig(uiConfig);
    m_setting->saveSetting(uiConfig);
}

ExprotSetting::ExportMethod MainWindow::getExportMethod()
{
    if(ui->radioButtonRec7->isChecked())
        return ExprotSetting::ExportMethod::RC7;
    else if(ui->radioButtonRawSample->isChecked())
        return ExprotSetting::ExportMethod::RawSample;
}

SignalViewParameters MainWindow::readSignalSetting()
{
    SignalViewParameters params;
    params.dbPath = ui->lineEditPath->text();
    params.signalFilePath = m_heaFilesWithPath.at(m_listItemIdx);
    params.targetFs = ui->spinBoxTargetFreq->value();
    params.gain = ui->doubleSpinBoxGain->value();
    params.offset = ui->spinBoxOffset->value();
    params.selectedLead[0] = ui->comboBoxSignal1->currentIndex();
    params.selectedLead[1] = ui->comboBoxSignal2->currentIndex();
    params.selectedLead[2] = ui->comboBoxSignal3->currentIndex();
    params.dbName = ui->comboBoxDatabaseName->currentText();
    return params;
}

void MainWindow::setSetting(CSettings *newSetting)
{
    m_setting = newSetting;
}

void MainWindow::addFileToListWidget(QListWidget *windget,
                                     QStringList &filepath,
                                     QStringList& filename,
                                     const QString&type,
                                     const QString&path)
{
    QDir dir(path);
    filepath.clear();
    filename.clear();
    windget->clear();
    QFileInfoList infoList = dir.entryInfoList();
    for (const QFileInfo& info : std::as_const(infoList)) {
        if (info.suffix() == type) {
            filename << info.fileName().split(".").first();
            filepath << info.filePath();
        }
    }
    windget->addItems(filename);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSetting();
    event->accept();
}

void MainWindow::setSignalWidget(const QList<SignalViewWidget*> widgetList)
{
    setupSignal(ui->gridLayoutLead1, widgetList[0]);
    setupSignal(ui->gridLayoutLead2, widgetList[1]);
    setupSignal(ui->gridLayoutLead2, widgetList[2]);
}

void MainWindow::updateRecordList(const QString& recordDirectory)
{
    if(!DirectoryValidator::validateDirectory(recordDirectory))
        return;
    addFileToListWidget(ui->listWidgetItems,m_headerFilePath,m_heaFilesWithPath,"hea", recordDirectory);
    ui->listWidgetItems->addItems(m_heaFilesWithPath);
}

void MainWindow::loadStyle()
{
    QFile styleFile(":/Res/style/style.qss");
    styleFile.open(QFile::ReadOnly);
    QString style = QLatin1String(styleFile.readAll());
    setStyleSheet(style);
}

void MainWindow::enableUIBtn(const bool& isEnable)
{
    ui->pushButtonExport->setEnabled(isEnable);
    ui->pushButtonRead->setEnabled(isEnable);
    ui->pushButtonUpdate->setEnabled(isEnable);
}

void MainWindow::loadUIConfig(const UIConfigs &uiConfig)
{
    ui->lineEditPath->setText(uiConfig.dataBasePath());
    ui->comboBoxDatabaseName->setCurrentText(uiConfig.dataBaseName());
    ui->comboBoxSignal1->setCurrentIndex(uiConfig.lineEditSignal1Index());
    ui->comboBoxSignal2->setCurrentIndex(uiConfig.lineEditSignal2Index());
    ui->comboBoxSignal3->setCurrentIndex(uiConfig.lineEditSignal3Index());
    ui->doubleSpinBoxGain->setValue(uiConfig.gainSignal());
    ui->spinBoxTargetFreq->setValue(uiConfig.targetFreq());
    ui->checkBoxExportAllData->setChecked(uiConfig.exportAll());
    ui->radioButtonRec7->setChecked(uiConfig.exportRC7());
    ui->radioButtonRawSample->setChecked(uiConfig.exportRaw());
}

void MainWindow::getUIConfig(UIConfigs &uiConfig)
{
    uiConfig.setDataBasePath(ui->lineEditPath->text());
    uiConfig.setDataBaseName(ui->comboBoxDatabaseName->currentText());
    uiConfig.setLineEditSignal1Index(ui->comboBoxSignal1->currentIndex());
    uiConfig.setLineEditSignal2Index(ui->comboBoxSignal2->currentIndex());
    uiConfig.setLineEditSignal3Index(ui->comboBoxSignal3->currentIndex());
    uiConfig.setTargetFreq(ui->spinBoxTargetFreq->value());
    uiConfig.setGainSignal(ui->doubleSpinBoxGain->value());
    uiConfig.setSignalOfset(ui->spinBoxOffset->value());
    uiConfig.setExportAll(ui->checkBoxExportAllData->isChecked());
    uiConfig.setExportRC7(ui->radioButtonRec7->isChecked());
    uiConfig.setExportRaw(ui->radioButtonRawSample->isChecked());
}

void MainWindow::on_listWidgetItems_currentRowChanged(int currentRow)
{
    // Handle invalid selection
    if (currentRow < 0 || currentRow >= m_headerFilePath.size()) {
        ui->textEditSignalInfo->clear();
        m_listItemIdx = -1;
        return;
    }

    m_listItemIdx = currentRow;

    QFile file(m_headerFilePath.at(currentRow));

    if (!file.open(QIODevice::ReadOnly)) {
        // Handle error - show message or clear text
        ui->textEditSignalInfo->clear();
        ui->textEditSignalInfo->setText("Error: Could not open file");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    ui->textEditSignalInfo->clear();
    ui->textEditSignalInfo->setText(data);
}

void MainWindow::on_pushButtonExport_clicked()
{
    QString path = QFileDialog::getExistingDirectory(nullptr, "Select Directory To Export", QDir::homePath());
    if(DirectoryValidator::validateDirectory(path))
    {
        enableUIBtn(false);
        ExprotSetting expSetting;
        expSetting.method = getExportMethod();
        expSetting.params = readSignalSetting();
        expSetting.outputPath = path;
        expSetting.pathList = m_heaFilesWithPath;
        expSetting.compressingRequsted = ui->checkBoxCompression->isChecked();
        expSetting.exportCSV = ui->checkBoxExportCSV->isChecked();
        if(!ui->checkBoxExportAllData->isChecked())
            Q_EMIT sigExportRequested(expSetting);
        else
        {
            Q_EMIT sigExportAllRequested(expSetting);
        }
    }
    else
        QMessageBox::critical(nullptr, "Dir Validation", "Dir is invalid");

}


void MainWindow::on_pushButtonUpdate_clicked()
{
    on_pushButtonRead_clicked();
}



void MainWindow::on_pushButtonData1_clicked()
{
    QString path = QFileDialog::getExistingDirectory(nullptr, "Select Data 1 Directory", QDir::homePath());
    if(DirectoryValidator::validateDirectory(path))
    {
        ui->lineEditPathData1->setText(path);
        addFileToListWidget(ui->listWidgetDir1,
                            m_CSV1FilePath,
                            m_CSV1FilesWithPath, "csv", path);
    }
    else
        QMessageBox::critical(nullptr, "Dir Validation", "Dir is invalid");

}


void MainWindow::on_pushButtonData2_clicked()
{
    QString path = QFileDialog::getExistingDirectory(nullptr, "Select Data 2 Directory", QDir::homePath());
    if(DirectoryValidator::validateDirectory(path))
    {
        ui->lineEditPathData2->setText(path);
        addFileToListWidget(ui->listWidgetDir2,m_CSV2FilePath,m_CSV2FilesWithPath, "csv", path);
    }
    else
        QMessageBox::critical(nullptr, "Dir Validation", "Dir is invalid");
}


void MainWindow::on_pushButtonCompare_clicked()
{
    QString path = QFileDialog::getExistingDirectory(nullptr, "Select Comapre Result irectory", QDir::homePath());
    if(DirectoryValidator::validateDirectory(path))
    {
        AnalyseCfg analyseCfg;
        analyseCfg.csvPath1 = m_CSV1FilesWithPath;
        analyseCfg.csvPath2 = m_CSV2FilesWithPath;
        analyseCfg.outputPath = path;
        Q_EMIT sigAnalyseRequested(analyseCfg);
    }
    else
        QMessageBox::critical(nullptr, "Dir Validation", "Dir is invalid");
}

