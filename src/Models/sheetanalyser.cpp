#include "sheetanalyser.h"

SheetAnalyser::SheetAnalyser(QObject *parent,
                             QStringList sheetPathList,
                             QString outputPath,
                             QString processFileName)
    : QObject(parent)
    , m_sheetPathList(std::move(sheetPathList))
    , m_outputPath(std::move(outputPath))
    , m_processFileName(processFileName)
{
}
bool SheetAnalyser::processSheets()
{
    if (m_sheetPathList.size() < 2) {
        qWarning() << "Compare requires two CSV files";
        return false;
    }
    if(!loadCSVData())
    {
        QMessageBox::critical(nullptr, "Load Error", "Load data from csv file failed.");
        return false;
    }
    if(!CompareSampleIdxAndType())
    {
        QMessageBox::critical(nullptr, "Compare Error", "Compare process failed.");
        return false;
    }
    if(!saveResult())
    {
        QMessageBox::critical(nullptr, "Save Error", "Save process failed.");
    }
    return true;
}

FileProcessResult SheetAnalyser::fileProcessRes() const
{
    return m_fileProcessRes;
}

bool SheetAnalyser::CompareSampleIdxAndType()
{
    try
    {
        int csv1Size = m_sheetRes.sampleIndexList1.size();
        const int csv2Size = m_sheetRes.sampleIndexList2.size();

        // Handle edge case
        if (csv1Size == 0 || csv2Size == 0) {
            qDebug() << "Empty sample lists!";
            return false;
        }

        // Constants
        const int MAX_DIFF_THRESHOLD = 300;  // Initial max threshold
        const int VALID_DIFF_MAX = 100;      // Maximum valid difference (exclusive)
        const int VALID_DIFF_MIN = 0;        // Minimum valid difference (inclusive)
        const int INVALID_INDEX = -1;        // Sentinel for invalid alignment

        int lastMatchIndex = -1;  // Track the last valid match index
        m_fileProcessRes.clear();
        m_fileProcessRes.fileName = m_processFileName;
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
                    m_sheetRes.typeAlignList1.append(INVALID_INDEX);
                    m_sheetRes.typeAlignList2.append(m_sheetRes.typeList2.at(gap));
                    m_sheetRes.difTypeList.append(false);
                    m_fileProcessRes.qrsPredict.fp++;
                }
            }

            // Now handle the valid/invalid match
            if(minDif >= VALID_DIFF_MIN && minDif < VALID_DIFF_MAX)
            {
                // Valid difference
                m_sheetRes.alignedList1.append(m_sheetRes.sampleIndexList1.at(i));
                m_sheetRes.alignedList2.append(m_sheetRes.sampleIndexList2.at(bestMatchIndex));
                m_sheetRes.difIndexList.append(minDif);
                m_fileProcessRes.qrsPredict.tp++;
                lastMatchIndex = bestMatchIndex;
            }
            else
            {
                // Invalid difference - use best match anyway but mark as invalid
                m_sheetRes.alignedList1.append(m_sheetRes.sampleIndexList1.at(i));
                m_sheetRes.alignedList2.append(m_sheetRes.sampleIndexList2.at(bestMatchIndex));
                m_sheetRes.difIndexList.append(INVALID_INDEX);
                m_fileProcessRes.qrsPredict.fn++;
                // Don't update lastMatchIndex for invalid matches to preserve gap detection
            }

            m_sheetRes.typeAlignList1.append(m_sheetRes.typeList1.at(i));
            m_sheetRes.typeAlignList2.append(m_sheetRes.typeList2.at(bestMatchIndex));
            m_sheetRes.difTypeList.append(m_sheetRes.typeList1.at(i) == m_sheetRes.typeList2.at(bestMatchIndex));

            //In this part count type mach and qrs validity
            if(m_sheetRes.typeList1.at(i) == 1 && m_sheetRes.typeList2.at(bestMatchIndex) == 1)
            {
                m_fileProcessRes.normalPredict.tp++;
            }
            else if(m_sheetRes.typeList1.at(i) == 1 && m_sheetRes.typeList2.at(bestMatchIndex) != 1)
            {
                m_fileProcessRes.normalPredict.fn++;
            }
            else if(m_sheetRes.typeList1.at(i) != 1 && m_sheetRes.typeList2.at(bestMatchIndex) == 1)
            {
                m_fileProcessRes.normalPredict.fp++;
            }

            if(m_sheetRes.typeList1.at(i) == 5 && m_sheetRes.typeList2.at(bestMatchIndex) == 5)
            {
                m_fileProcessRes.pvcPredict.tp++;
            }
            else if(m_sheetRes.typeList1.at(i) == 5 && m_sheetRes.typeList2.at(bestMatchIndex) != 5)
            {
                m_fileProcessRes.pvcPredict.fn++;
            }
            else if(m_sheetRes.typeList1.at(i) != 5 && m_sheetRes.typeList2.at(bestMatchIndex) == 5)
            {
                m_fileProcessRes.pvcPredict.fp++;
            }
        }
        m_fileProcessRes.qrsPredict.calcParams();
        m_fileProcessRes.normalPredict.calcParams();
        m_fileProcessRes.pvcPredict.calcParams();
    }
    catch(...)
    {
        return false;
    }
    return true;
}

bool SheetAnalyser::saveResult()
{
    const QString outCsv = m_outputPath + QStringLiteral("/%1.csv").arg(m_processFileName);
    CSV compareOut(nullptr, outCsv);
    return compareOut.saveRawCSVRes(m_sheetRes);
}

bool SheetAnalyser::loadCSVData()
{
    CSV csv1(nullptr, m_sheetPathList[0]);
    CSV csv2(nullptr, m_sheetPathList[1]);

    if (!csv1.loadFromFile(m_sheetPathList[0]) || !csv2.loadFromFile(m_sheetPathList[1])) {
        qWarning() << "Failed to load CSV files";
        return false;
    }
    m_sheetRes.sampleIndexList1 = csv1.csvFormat().sampleIndex;
    m_sheetRes.typeList1 = csv1.csvFormat().type;
    m_sheetRes.sampleIndexList2 = csv2.csvFormat().sampleIndex;
    m_sheetRes.typeList2 = csv2.csvFormat().type;
    return true;
}


