#include "sheetanalyser.h"

SheetAnalyser::SheetAnalyser(QObject *parent,
                             QStringList sheetPathList,
                             QString outputPath,
                             int processTime)
    : QObject(parent)
    , m_sheetPathList(std::move(sheetPathList))
    , m_outputPath(std::move(outputPath))
    , m_processTime(processTime)
{
}

bool SheetAnalyser::processSheets()
{
    if (m_sheetPathList.size() < 2) {
        qWarning() << "Compare requires two CSV files";
        return false;
    }

    CSV csv1(nullptr, m_sheetPathList[0]);
    CSV csv2(nullptr, m_sheetPathList[1]);

    if (!csv1.loadFromFile(m_sheetPathList[0])) {
        qWarning() << "Failed to load CSV:" << m_sheetPathList[0];
        return false;
    }

    if (!csv2.loadFromFile(m_sheetPathList[1])) {
        qWarning() << "Failed to load CSV:" << m_sheetPathList[1];
        return false;
    }

    const QString outCsv = m_outputPath + QStringLiteral("/compare%1.csv").arg(m_processTime);
    CSV compareOut(nullptr, outCsv);
    compareOut.comparesFile(csv1.csvFormat(), csv2.csvFormat());
    return true;
}
