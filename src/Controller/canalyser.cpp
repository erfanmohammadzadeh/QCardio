#include "canalyser.h"

CAnalyser::CAnalyser(QObject *parent, AnalyseCfg cfg)
    : QObject{parent}, m_analyseCfg(cfg)
{
}

void CAnalyser::analyse()
{
    QtConcurrent::run(QThreadPool::globalInstance(), [=](){
        int processFile = qMin(m_analyseCfg.csvPath1.size(), m_analyseCfg.csvPath2.size());
        for(int i = 0; i < processFile; i++)
        {
            QStringList pairFile;
            pairFile << m_analyseCfg.csvPath1[i] << m_analyseCfg.csvPath2[i];
            SheetAnalyser analyser(nullptr,pairFile, m_analyseCfg.outputPath,i);
            analyser.processSheets();
        }
    });
}
