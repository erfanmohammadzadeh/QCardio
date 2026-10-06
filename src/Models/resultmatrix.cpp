#include "resultmatrix.h"

#include <cstring>

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

void ResultMatrix::insertBeat(const int &ref, int detected)
{
    addComparison(ref, detected);
}

void ResultMatrix::addComparison(int refAnn, int detAnn)
{
    const bool referenceMissing = refAnn < 0;
    const bool detectionMissing = detAnn < 0;
    const AamiClass referenceClass = referenceMissing ? AamiO : aamiClassOf(refAnn);
    const AamiClass detectionClass = detectionMissing ? AamiO : aamiClassOf(detAnn);
    if (referenceClass == AamiNotQrs || detectionClass == AamiNotQrs)
        return;

    ++m_aami[referenceClass][detectionClass];

    if (!referenceMissing && !detectionMissing) {
        const int refidx = static_cast<int>(convertTypeToArrhythmia(refAnn));
        const int detidx = static_cast<int>(convertTypeToArrhythmia(detAnn));
        if (refidx >= 0 && refidx < ArrhythmiaType::ArrhythmiaTypeCount &&
            detidx >= 0 && detidx < ArrhythmiaType::ArrhythmiaTypeCount) {
            ++matrixBeatType[refidx][detidx];
        }
    }
}

void ResultMatrix::copyAami(quint32 dest[AamiClassCount][AamiClassCount]) const
{
    std::memcpy(dest, m_aami, sizeof(m_aami));
}

