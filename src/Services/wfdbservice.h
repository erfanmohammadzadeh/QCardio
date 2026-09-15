#ifndef WFDBSERVICE_H
#define WFDBSERVICE_H

#include <QObject>
#include "ThirdParty/wfdb/wfdb.h"
#include "Models/sample.h"
#include "Models/global_qcardio.h"

#define DEBUG_WFDBSERVICE false

class WfdbService : public QObject
{
    Q_OBJECT
public:
    explicit WfdbService(QObject *parent = nullptr);
    ~WfdbService();

    bool readData(const SignalViewParameters& params);
    bool readAnot(const SignalViewParameters& params);
    MIT_BIH_ECGData getStructData() const;

signals:
    void sigAppendLog(const QString& text);

private:
    void clearVec();

    WFDB_Siginfo *signalInfo = nullptr;
    WFDB_Sample *sampleVector = nullptr;
    int numberOfSignals = 0;
    MIT_BIH_ECGData m_strData;
};

#endif // WFDBSERVICE_H
