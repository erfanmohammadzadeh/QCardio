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
    for (int i = 0; i < processFile; ++i) {
        QStringList pairFile;
        pairFile << m_analyseCfg.csvPath1[i] << m_analyseCfg.csvPath2[i];

        SheetAnalyser analyser(nullptr, pairFile, m_analyseCfg.outputPath, i);
        allSucceeded = analyser.processSheets() && allSucceeded;
    }

    return allSucceeded;
}
