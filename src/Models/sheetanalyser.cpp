#include "sheetanalyser.h"
#include "Models/beatkpi.h"
#include "wfdb/ecgcodes.h"

SheetAnalyser::SheetAnalyser(QObject *parent,
                             QStringList sheetPathList,
                             AnalyseCfg analyseCfg,
                             QString processFileName)
    : QObject(parent)
    , m_sheetPathList(std::move(sheetPathList))
    , m_outputPath(std::move(analyseCfg.outputPath))
    , m_processFileName(processFileName)
    , m_analyseCfg(analyseCfg)

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
        const int csv1Size = m_sheetRes.sampleIndexList1.size();
        const int csv2Size = m_sheetRes.sampleIndexList2.size();

        if (csv1Size == 0 || csv2Size == 0) {
            qDebug() << "Empty sample lists!";
            return false;
        }

        constexpr int MAX_DIFF_THRESHOLD = 300;
        constexpr int VALID_DIFF_MAX     = 100;
        constexpr int VALID_DIFF_MIN     = 0;
        constexpr int INVALID_INDEX      = -1;

        m_fileProcessRes.clear();
        m_fileProcessRes.fileName = m_processFileName;

        // Track which csv2 entries have already been consumed by an alignment.
        std::vector<bool> used2(csv2Size, false);

        int lastMatchIndex = -1;

        m_fileProcessRes.totalBeat = csv1Size;
        for (int i = 0; i < csv1Size; ++i)
        {
            // --- find best unused match in csv2 ---
            int minDif          = MAX_DIFF_THRESHOLD;
            int bestMatchIndex  = INVALID_INDEX;

            for (int j = 0; j < csv2Size; ++j)
            {
                if (used2[j]) continue;
                if (j <= lastMatchIndex) continue;   // enforce monotonic alignment

                const int currentDif = m_sheetRes.getDif(i, j);
                if (currentDif < minDif)
                {
                    minDif         = currentDif;
                    bestMatchIndex = j;
                }
            }


            const bool validMatch =
                (bestMatchIndex != INVALID_INDEX) &&
                (minDif >= VALID_DIFF_MIN) &&
                (minDif <  VALID_DIFF_MAX);

            if (validMatch)
            {
                // --- record FP rows for any csv2 beats skipped since last match ---
                for (int gap = lastMatchIndex + 1; gap < bestMatchIndex; ++gap)
                {
                    m_sheetRes.alignedList1.append(INVALID_INDEX);
                    m_sheetRes.alignedList2.append(m_sheetRes.sampleIndexList2.at(gap));
                    m_sheetRes.difIndexList.append(INVALID_INDEX);
                    m_sheetRes.typeAlignList1.append(INVALID_INDEX);
                    m_sheetRes.typeAlignList2.append(m_sheetRes.typeList2.at(gap));
                    m_sheetRes.difTypeList.append(false);

                    // Predicted beat with no matching reference beat => FP
                    if(m_analyseCfg.compareOfset < i)
                    {
                        m_fileProcessRes.qrsPredict.fp++;

                        // Feed the classifier: reference = Unknown, predicted = actual
                        m_beatTypeMap.insertBeat(UNKNOWN,
                                                 m_sheetRes.typeList2.at(gap));
                    }
                }

                // --- record the aligned (TP) pair ---
                m_sheetRes.alignedList1.append(m_sheetRes.sampleIndexList1.at(i));
                m_sheetRes.alignedList2.append(m_sheetRes.sampleIndexList2.at(bestMatchIndex));
                m_sheetRes.difIndexList.append(minDif);
                m_sheetRes.typeAlignList1.append(m_sheetRes.typeList1.at(i));
                m_sheetRes.typeAlignList2.append(m_sheetRes.typeList2.at(bestMatchIndex));
                m_sheetRes.difTypeList.append(isTypeMatch(m_sheetRes.typeList1.at(i),m_sheetRes.typeList2.at(bestMatchIndex)));

                if(m_analyseCfg.compareOfset < i)
                {
                    m_fileProcessRes.qrsPredict.tp++;

                    m_beatTypeMap.insertBeat(m_sheetRes.typeList1.at(i),
                                             m_sheetRes.typeList2.at(bestMatchIndex));
                }

                used2[bestMatchIndex] = true;
                lastMatchIndex        = bestMatchIndex;
            }
            else
            {
                // --- no usable match: reference beat with no detection => FN ---
                m_sheetRes.alignedList1.append(m_sheetRes.sampleIndexList1.at(i));
                m_sheetRes.alignedList2.append(INVALID_INDEX);
                m_sheetRes.difIndexList.append(INVALID_INDEX);
                m_sheetRes.typeAlignList1.append(m_sheetRes.typeList1.at(i));
                m_sheetRes.typeAlignList2.append(INVALID_INDEX);
                m_sheetRes.difTypeList.append(false);

                if(m_analyseCfg.compareOfset < i)
                {
                    m_fileProcessRes.qrsPredict.fn++;

                    // Feed the classifier: reference = actual, predicted = Unknown
                    m_beatTypeMap.insertBeat(m_sheetRes.typeList1.at(i),
                                             UNKNOWN);

                    m_fileProcessRes.missedBeatCount++;
                    if(i > 1)
                        m_fileProcessRes.totalShutdownSqrs += abs(m_sheetRes.sampleIndexList1.at(i)-m_sheetRes.sampleIndexList1.at(i-1));
                    if(NormalBeat.contains(m_sheetRes.typeList1.at(i)))
                        m_fileProcessRes.normalMissed++;
                    else if(PVCBeat.contains(m_sheetRes.typeList1.at(i)))
                        m_fileProcessRes.pvcMissed++;
                }
            }
        }

        // --- trailing csv2 beats never matched => FP ---
        for (int j = lastMatchIndex + 1; j < csv2Size; ++j)
        {
            if (used2[j]) continue;

            m_sheetRes.alignedList1.append(INVALID_INDEX);
            m_sheetRes.alignedList2.append(m_sheetRes.sampleIndexList2.at(j));
            m_sheetRes.difIndexList.append(INVALID_INDEX);
            m_sheetRes.typeAlignList1.append(INVALID_INDEX);
            m_sheetRes.typeAlignList2.append(m_sheetRes.typeList2.at(j));
            m_sheetRes.difTypeList.append(false);

            if(m_analyseCfg.compareOfset < j)
            {
                m_fileProcessRes.qrsPredict.fp++;
                m_beatTypeMap.insertBeat(UNKNOWN, m_sheetRes.typeList2.at(j));
            }
        }

        // --- finalize KPIs ---
        BeatKPI beatKpi(m_beatTypeMap);
        beatKpi.run();

        m_fileProcessRes.qrsPredict.calcParams();
        std::memcpy(m_fileProcessRes.beatTypeMap,
                    m_beatTypeMap.getMatrix(),
                    sizeof(m_fileProcessRes.beatTypeMap));
        m_fileProcessRes.normalPredict = beatKpi.m_NPrediction;
        m_fileProcessRes.pvcPredict    = beatKpi.m_PVCPrediction;
        m_fileProcessRes.totalShutdown = convertSampleCountToTimeInTime(m_fileProcessRes.totalShutdownSqrs, m_analyseCfg.sampleRate);
        m_fileProcessRes.compareOfset = m_analyseCfg.compareOfset;
    }
    catch (...)
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

bool SheetAnalyser::isTypeMatch(const quint8 &ref, const quint8 &det)
{
    if (NormalBeat.contains(ref) && NormalBeat.contains(det)) {
        return true;
    }
    else if (PVCBeat.contains(ref) && PVCBeat.contains(det)) {
        return true;
    }
    else if (NOISEBeat.contains(ref) && NOISEBeat.contains(det)) {
        return true;
    }
    else
        return false;
}

QTime SheetAnalyser::convertSampleCountToTimeInTime(quint64 sampleCount, quint16 samplingTime)
{
    QTime time;
    time.setHMS(0,0,0,0);
    quint64 duration    = sampleCount/samplingTime;
    time  = time.addSecs(duration%(24*3600));
    return  time;
}
