#include "cwfdb.h"

Cwfdb::Cwfdb(QObject *parent)
    : QObject{parent}
{

}

Cwfdb::~Cwfdb()
{
    clearVec();
}

MIT_BIH_ECGData Cwfdb::getStructData() const
{
    return m_strData;
}

void Cwfdb::clearVec()
{
    if(signalInfo)
        delete signalInfo;
    if(sampleVector)
        delete sampleVector;
    m_strData.clear();
}

bool Cwfdb::readData(const SignalViewParameters &params)
{
    clearVec();
    setwfdb(const_cast<char*>(params.dbPath.toStdString().c_str()));
    Q_EMIT sigAppnedLog("Rec: " + params.signalFilePath + "  DBName: " +params.dbName + "DB Path: " +params.dbPath);
    int numberOfSignals = 0;
    // std::vector<WFDB_Siginfo> siginfo;
    WFDB_Frequency target_fs = params.targetFs;

    //specify database name
    numberOfSignals = isigopen(const_cast<char*>(params.signalFilePath.toStdString().c_str()), NULL, 0);

    std::vector<WFDB_Siginfo> siginfo(numberOfSignals);
    if (isigopen(const_cast<char*>(params.signalFilePath.toStdString().c_str()), siginfo.data(), numberOfSignals) != numberOfSignals) {
        Q_EMIT sigAppnedLog("Failed to read header");
        return false;
    }

    if(setifreq(target_fs) != 0)
        return false;

    if (numberOfSignals > 0) {
        // siginfo.resize(numberOfSignals);

        if (isigopen(const_cast<char*>(params.signalFilePath.toStdString().c_str()), siginfo.data(), numberOfSignals) != numberOfSignals) {
            Q_EMIT sigAppnedLog("Failed to read header");
            return false;
        }

        long totalSamples = siginfo[0].nsamp;
        if (totalSamples <= 0) {
            Q_EMIT sigAppnedLog( "No samples in file");
            return false;
        }

        // Create a 2D vector to store all samples
        std::vector<std::vector<WFDB_Sample>> allData(numberOfSignals, std::vector<WFDB_Sample>(totalSamples));

        // Read all samples
        for (long sampleIndex = 0; sampleIndex < totalSamples; ++sampleIndex) {
            WFDB_Sample* signalData = new WFDB_Sample[numberOfSignals];

            getvec(signalData);
            // CORRECTED: Use signalIndex to access signalData
            for (int signalIndex = 0; signalIndex < numberOfSignals; ++signalIndex) {
                allData[signalIndex][sampleIndex] = signalData[signalIndex];
            }

            delete[] signalData;
        }

        // QVector<Sample> sampleData
        for(int i = 0 ; i < SAVED_LEAD_COUNT; i++)
            m_strData.selectedLead[i] = params.selectedLead[i];
        m_strData.filename = params.signalFilePath;
        m_strData.dbName = params.dbName;
        // Now you can access all data
        QVector<qreal> nsig;
        QVector<QVector<qreal>> sigList;
        for (int signalIndex = 0; signalIndex < numberOfSignals; ++signalIndex) {
            nsig.clear();
            for (long sampleIndex = 0; sampleIndex < totalSamples; ++sampleIndex) {
                nsig.append((allData[signalIndex][sampleIndex]*params.gain)+params.offset);
            }
            sigList.append(nsig);
        }
        m_strData.totalSample = totalSamples;
        m_strData.nsigs = sigList;
        if(totalSamples > 1000)
            return true;
    }
    return false;
}

bool Cwfdb::readAnot(const SignalViewParameters &params)
{
    // AnnotationReader reader(params.dbPath, params.signalFilePath, "atr");
    AnnotationReader *reader = new AnnotationReader(nullptr, params, "atr");
    connect(reader, &AnnotationReader::sigAppnedLog, this, &Cwfdb::sigAppnedLog);
    if (!reader->loadAnnotations()) {
        Q_EMIT sigAppnedLog("Failed to load annotations");
        reader->deleteLater();
        return false;
    }

    m_strData.anotList = reader->getAnnotations();
    reader->calcNoneBeatIndex(m_strData);
    // m_strData.rrIntervals = reader->computeRRIntervals(m_strData.anotList, params.targetFs);

    // if (!m_strData.rrIntervals.isEmpty()) {
        // double meanRR = 0.0;
        // double minRR = m_strData.rrIntervals[0].intervalSeconds;
        // double maxRR = m_strData.rrIntervals[0].intervalSeconds;

        // for (const auto& rr : std::as_const(m_strData.rrIntervals)) {
        //     meanRR += rr.intervalSeconds;
        //     minRR = qMin(minRR, rr.intervalSeconds);
        //     maxRR = qMax(maxRR, rr.intervalSeconds);
        // }
        // meanRR /= m_strData.rrIntervals.size();
#if DEBUG_CWFDB
        //qDebug() << "\nRR Interval Statistics:";
        //qDebug() << QString("  Mean: %1 ms").arg(meanRR * 1000, 0, 'f', 2);
        //qDebug() << QString("  Min: %1 ms").arg(minRR * 1000, 0, 'f', 2);
        //qDebug() << QString("  Max: %1 ms").arg(maxRR * 1000, 0, 'f', 2);
        //qDebug() << QString("  HR: %1 bpm").arg(60.0 / meanRR, 0, 'f', 1);
#endif
    // }
    reader->deleteLater();
    return true;
}




