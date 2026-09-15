#include "wfdbservice.h"
#include "Services/annotationservice.h"

#include <vector>

WfdbService::WfdbService(QObject *parent)
    : QObject{parent}
{
}

WfdbService::~WfdbService()
{
    clearVec();
}

MIT_BIH_ECGData WfdbService::getStructData() const
{
    return m_strData;
}

void WfdbService::clearVec()
{
    if (signalInfo)
        delete signalInfo;
    if (sampleVector)
        delete sampleVector;
    m_strData.clear();
}

bool WfdbService::readData(const SignalViewParameters &params)
{
    clearVec();
    setwfdb(const_cast<char*>(params.dbPath.toStdString().c_str()));
    Q_EMIT sigAppendLog("Rec: " + params.signalFilePath + "  DBName: " + params.dbName + "DB Path: " + params.dbPath);
    int numberOfSignals = 0;
    WFDB_Frequency target_fs = params.targetFs;

    numberOfSignals = isigopen(const_cast<char*>(params.signalFilePath.toStdString().c_str()), NULL, 0);

    std::vector<WFDB_Siginfo> siginfo(numberOfSignals);
    if (isigopen(const_cast<char*>(params.signalFilePath.toStdString().c_str()), siginfo.data(), numberOfSignals) != numberOfSignals) {
        Q_EMIT sigAppendLog("Failed to read header");
        return false;
    }

    if (setifreq(target_fs) != 0)
        return false;

    if (numberOfSignals > 0) {
        if (isigopen(const_cast<char*>(params.signalFilePath.toStdString().c_str()), siginfo.data(), numberOfSignals) != numberOfSignals) {
            Q_EMIT sigAppendLog("Failed to read header");
            return false;
        }

        long totalSamples = siginfo[0].nsamp;
        if (totalSamples <= 0) {
            Q_EMIT sigAppendLog("No samples in file");
            return false;
        }

        std::vector<std::vector<WFDB_Sample>> allData(numberOfSignals, std::vector<WFDB_Sample>(totalSamples));

        for (long sampleIndex = 0; sampleIndex < totalSamples; ++sampleIndex) {
            WFDB_Sample* signalData = new WFDB_Sample[numberOfSignals];

            getvec(signalData);
            for (int signalIndex = 0; signalIndex < numberOfSignals; ++signalIndex) {
                allData[signalIndex][sampleIndex] = signalData[signalIndex];
            }

            delete[] signalData;
        }

        for (int i = 0; i < SAVED_LEAD_COUNT; i++)
            m_strData.selectedLead[i] = params.selectedLead[i];
        m_strData.filename = params.signalFilePath;
        m_strData.dbName = params.dbName;
        QVector<qreal> nsig;
        QVector<QVector<qreal>> sigList;
        for (int signalIndex = 0; signalIndex < numberOfSignals; ++signalIndex) {
            nsig.clear();
            for (long sampleIndex = 0; sampleIndex < totalSamples; ++sampleIndex) {
                nsig.append((allData[signalIndex][sampleIndex] * params.gain) + params.offset);
            }
            sigList.append(nsig);
        }
        m_strData.totalSample = totalSamples;
        m_strData.nsigs = sigList;
        if (totalSamples > 1000)
            return true;
    }
    return false;
}

bool WfdbService::readAnot(const SignalViewParameters &params)
{
    AnnotationService *reader = new AnnotationService(nullptr, params, "atr");
    connect(reader, &AnnotationService::sigAppendLog, this, &WfdbService::sigAppendLog);
    if (!reader->loadAnnotations()) {
        Q_EMIT sigAppendLog("Failed to load annotations");
        reader->deleteLater();
        return false;
    }

    m_strData.anotList = reader->getAnnotations();
    reader->calcNoneBeatIndex(m_strData);
    reader->deleteLater();
    return true;
}
