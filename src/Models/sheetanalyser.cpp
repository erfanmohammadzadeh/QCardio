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

    if (!csv1.loadFromFile(m_sheetPathList[0]) || !csv2.loadFromFile(m_sheetPathList[1])) {
        qWarning() << "Failed to load CSV files";
        return false;
    }

    // FIX 1: Corrected target list assignment
    m_sheetRes.sampleIndexList1 = csv1.csvFormat().sampleIndex;
    m_sheetRes.sampleIndexList2 = csv2.csvFormat().sampleIndex;

    runMachAndCheck();

    const QString outCsv = m_outputPath + QStringLiteral("/compare%1.csv").arg(m_processTime);
    CSV compareOut(nullptr, outCsv);
    compareOut.saveRawCSVRes(m_sheetRes);
    return true;
}

void SheetAnalyser::runMachAndCheck()
{
    int csv1Size = m_sheetRes.sampleIndexList1.size();
    const int csv2Size = m_sheetRes.sampleIndexList2.size();

    // Handle edge case
    if (csv1Size == 0 || csv2Size == 0) {
        qDebug() << "Empty sample lists!";
        return;
    }

    // Constants
    const int MAX_DIFF_THRESHOLD = 300;  // Initial max threshold
    const int VALID_DIFF_MAX = 100;      // Maximum valid difference (exclusive)
    const int VALID_DIFF_MIN = 0;        // Minimum valid difference (inclusive)
    const int INVALID_INDEX = -1;        // Sentinel for invalid alignment

    int lastMatchIndex = -1;  // Track the last valid match index

    for(int i = 0; i < csv1Size; i++)
    {
        int minDif = MAX_DIFF_THRESHOLD;
        int bestMatchIndex = 0;

        // Find best match in csv2 for current csv1 index
        for(int j = 0; j < csv2Size; j++)
        {
            int currentDif = m_sheetRes.getDif(i, j);
            if(currentDif < minDif)
            {
                minDif = currentDif;
                bestMatchIndex = j;
            }
        }

        // Check for gap in matching indices
        if(lastMatchIndex != -1 && (bestMatchIndex - lastMatchIndex) > 1)
        {
            // Gap detected - insert invalid entries for the gap
            for(int gap = lastMatchIndex + 1; gap < bestMatchIndex; gap++)
            {
                m_sheetRes.alignedList1.append(INVALID_INDEX);
                m_sheetRes.alignedList2.append(m_sheetRes.sampleIndexList2.at(gap));
                m_sheetRes.difIndexList.append(INVALID_INDEX);
            }
        }

        // Now handle the valid/invalid match
        if(minDif >= VALID_DIFF_MIN && minDif < VALID_DIFF_MAX)
        {
            // Valid difference
            m_sheetRes.alignedList1.append(m_sheetRes.sampleIndexList1.at(i));
            m_sheetRes.alignedList2.append(m_sheetRes.sampleIndexList2.at(bestMatchIndex));
            m_sheetRes.difIndexList.append(minDif);
            lastMatchIndex = bestMatchIndex;
        }
        else
        {
            // Invalid difference - use best match anyway but mark as invalid
            m_sheetRes.alignedList1.append(m_sheetRes.sampleIndexList1.at(i));
            m_sheetRes.alignedList2.append(m_sheetRes.sampleIndexList2.at(bestMatchIndex));
            m_sheetRes.difIndexList.append(INVALID_INDEX);
            // Don't update lastMatchIndex for invalid matches to preserve gap detection
        }
    }
}


