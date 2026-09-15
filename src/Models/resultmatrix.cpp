#include "resultmatrix.h"

ResultMatrix::ResultMatrix(MatrixType type)
{
    m_matrixType = type;
}

quint32 *ResultMatrix::getDif()
{
    return m_difBeatType;
}

RunMatrixType &ResultMatrix::getRunMatrix()
{
    return matrixRun;
}

BeatMatrixType &ResultMatrix::getMatrix()
{
    return matrixBeatType;
}

//filter type
void ResultMatrix::insertBeat(const int &ref, int detected)
{
    int refType = NORMAL, detType = NORMAL;

    if (NormalBeat.contains(ref) && NormalBeat.contains(detected)) {
        detType = NORMAL;
        refType = NORMAL;
    } else if (PVCBeat.contains(ref) && PVCBeat.contains(detected)) {
        detType = PVC;
        refType = PVC;
    } else {
        refType = ref;
        detType = detected;
    }

    const int refidx = static_cast<int>(convertTypeToArrhythmia(refType));
    const int detidx = static_cast<int>(convertTypeToArrhythmia(detType));
    matrixBeatType[refidx][detidx]++;
}

