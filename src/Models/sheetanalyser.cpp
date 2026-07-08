#include "sheetanalyser.h"

SheetAnalyser::SheetAnalyser(QObject *parent, QStringList sheetPathList, QString outputPath, int processTime)
    : QObject{parent}, m_sheetPathList(sheetPathList), m_outputPath(outputPath)
{
    connect(this, &SheetAnalyser::sigStartCompare, this,[=](){
        qDebug() << "------5";

        QString outcsv = m_outputPath + QString("/compare%1.csv").arg(processTime);
        CSV compare(nullptr, outcsv);
        compare.comparesFile(m_csv1->csvFormat(), m_csv2->csvFormat());
    });
}

void SheetAnalyser::processSheets()
{

    if(m_sheetPathList.size() < 2) return;
    bool processEnd[2] = {false};

    m_csv1 = new CSV(this, m_sheetPathList[0]);
    connect(m_csv1, &CSV::sigReadyForRead, this, [&processEnd, this]() {
        //
        processEnd[0] = true;
        if(processEnd[0] == true && processEnd[1] == true)
        {
            Q_EMIT sigStartCompare();
        }
    });
    m_csv1->loadRequested();

    m_csv2 = new CSV(this, m_sheetPathList[1]);
    connect(m_csv2, &CSV::sigReadyForRead, this, [&processEnd, this]() {
        //
        processEnd[1] = true;
        if(processEnd[0] == true && processEnd[1] == true)
        {
            Q_EMIT sigStartCompare();
        }
    });
    m_csv2->loadRequested();
}
