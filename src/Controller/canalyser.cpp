#include "canalyser.h"

#include <QtConcurrent/QtConcurrent>

CAnalyser::CAnalyser(QObject *parent, AnalyseCfg cfg)
    : QObject(parent)
    , m_analyseCfg(std::move(cfg))
{
}

bool CAnalyser::analyse()
{
    const int processFile = qMin(m_analyseCfg.csvPath1.size(), m_analyseCfg.csvPath2.size());
    if (processFile == 0) {
        qWarning() << "No CSV file pairs to compare";
        return false;
    }

    bool allSucceeded = true;
    QVector<FileProcessResult> fileResultList;
    for (int i = 0; i < processFile; ++i) {
        QStringList pairFile;
        pairFile << m_analyseCfg.csvPath1[i] << m_analyseCfg.csvPath2[i];
        QString base1 = QFileInfo(m_analyseCfg.csvPath1[i]).baseName();
        QString base2 = QFileInfo(m_analyseCfg.csvPath2[i]).baseName();
        QString filename = base1 + "VS" + base2;
        SheetAnalyser analyser(nullptr, pairFile, m_analyseCfg, filename);
        allSucceeded = analyser.processSheets() && allSucceeded;
        fileResultList.append(analyser.fileProcessRes());
    }

    if(!saveProcessFileResult(fileResultList))
    {
        QMessageBox::critical(nullptr, "Save Report Err", "Save report result failed.");
    }

    return allSucceeded;
}

bool CAnalyser::saveProcessFileResult(const QVector<FileProcessResult> &fileProcessRes)
{
    QString path = m_analyseCfg.outputPath + "/" + "Report.csv";
    Q_EMIT sigAppendLog("Result Path: " + path);
    CSV csvProcessFile(nullptr, path);
    return csvProcessFile.saveProcessFileResult(fileProcessRes);
}
