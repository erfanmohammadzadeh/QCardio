#ifndef SHEETANALYSER_H
#define SHEETANALYSER_H

#include <QObject>
#include <Models/csv.h>
#include "Models/global_qcardio.h"
#include "Models/resultmatrix.h"

enum MatchStatus
{
    NTP,
    NFP,
    NFN,
    PVCTP,
    PVCFP,
    PVCFN,
    SVPBTP,
    SVPBFP,
    SVPBFN,
    NotValid
};

class SheetAnalyser : public QObject
{
    Q_OBJECT
public:
    explicit SheetAnalyser(QObject *parent = nullptr,
                           QStringList sheetPathList = {},
                           AnalyseCfg analyseCfg = AnalyseCfg(),
                           QString processFileName = 0);
    bool processSheets();

    FileProcessResult fileProcessRes() const;

private:
    QStringList m_sheetPathList;
    QString m_outputPath;
    SheetResult m_sheetRes;
    QString m_processFileName = 0;
    FileProcessResult m_fileProcessRes;
    bool CompareSampleIdxAndType();
    bool saveResult();
    bool loadCSVData();
    bool isTypeMatch(const quint8& ref, const quint8& det);
    int  sampleRate = 178;

    ResultMatrix m_beatTypeMap = ResultMatrix(MatrixType::BeatType);
    ResultMatrix m_pvcRunMap = ResultMatrix(MatrixType::RunEpisode);
    ResultMatrix m_svtRunMap = ResultMatrix(MatrixType::RunEpisode);

    QTime convertSampleCountToTimeInTime(quint64 sampleCount, quint16 samplingTime);

};

#endif // SHEETANALYSER_H
