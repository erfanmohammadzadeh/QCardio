#include "analyseservice.h"
#include "Models/csv.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>

AnalyseService::AnalyseService(QObject *parent, AnalyseCfg cfg)
    : QObject(parent)
    , m_analyseCfg(std::move(cfg))
{
}

bool AnalyseService::run()
{
    const int processFile = qMin(m_analyseCfg.csvPath1.size(), m_analyseCfg.csvPath2.size());
    if (processFile == 0) {
        qWarning() << "No CSV file pairs to compare";
        return false;
    }

    bool allSucceeded = true;
    AnalyseFileProcessResult fileResult;
    for (int i = 0; i < processFile; ++i) {
        QStringList pairFile;
        pairFile << m_analyseCfg.csvPath1[i] << m_analyseCfg.csvPath2[i];
        QString base1 = QFileInfo(m_analyseCfg.csvPath1[i]).baseName();
        QString base2 = QFileInfo(m_analyseCfg.csvPath2[i]).baseName();
        QString filename = base1 + "VS" + base2;
        Q_EMIT sigAppendLog(QString("Comparing %1 (%2/%3)").arg(filename).arg(i + 1).arg(processFile));
        SheetAnalyser analyser(nullptr, pairFile, m_analyseCfg, filename);
        allSucceeded = analyser.processSheets() && allSucceeded;
        fileResult.fileProcessResult.append(analyser.fileProcessRes());
        Q_EMIT sigProgress(i + 1, processFile);
    }
    fileResult.calcParam();

    if (!saveProcessFileResult(fileResult)) {
        QMessageBox::critical(nullptr, "Save Report Err", "Save report result failed.");
    }

    return allSucceeded;
}

bool AnalyseService::saveProcessFileResult(const AnalyseFileProcessResult &fileProcessRes)
{
    QString path = m_analyseCfg.outputPath + "/" + "Report.csv";
    Q_EMIT sigAppendLog("Result Path: " + path);
    CSV csvProcessFile(nullptr, path);
    return csvProcessFile.saveProcessFileResult(fileProcessRes);
}
