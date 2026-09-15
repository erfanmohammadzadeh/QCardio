#ifndef BEATKPI_H
#define BEATKPI_H
#include <Models/global_qcardio.h>
#include <Models/resultmatrix.h>
class BeatKPI
{
public:
    BeatKPI(ResultMatrix beatMatrix = ResultMatrix(MatrixType::BeatType));

    ResultMatrix inputData = ResultMatrix(MatrixType::BeatType);
    Predicting m_NPrediction;
    Predicting m_PVCPrediction;

    void calcNormalKPI();
    void calcPVCKPI();
    void run();

private:
    int m_totalBeat = 0;

};

#endif // BEATKPI_H
