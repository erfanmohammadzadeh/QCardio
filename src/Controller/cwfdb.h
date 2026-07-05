#ifndef CWFDB_H
#define CWFDB_H
#include <iostream>
#include <string.h>
#include <QObject>
#include "wfdb/wfdb.h"
#include <QDebug>
#include <Models/sample.h>
#include <Models/global_qcardio.h>
#include <Models/annotationreader.h>

class Cwfdb : public QObject
{
    Q_OBJECT
public:
    explicit Cwfdb(QObject *parent = nullptr);
    ~Cwfdb();
    WFDB_Siginfo *signalInfo = nullptr;
    WFDB_Sample *sampleVector = nullptr;
    int numberOfSignals;
    MIT_BIH_ECGData m_strData;


    void clearVec();
    bool readData(const SignalViewParameters& params);
    bool readAnot(const SignalViewParameters& params);

    MIT_BIH_ECGData getStructData() const;

signals:
};

#endif // CWFDB_H
