#include "beatkpi.h"
#include "Models/ec57metrics.h"

BeatKPI::BeatKPI(ResultMatrix beatMatrix)
{
    inputData = beatMatrix;
    m_totalBeat = inputData.totalBeats();
}

void BeatKPI::run()
{
    quint32 matrix[AamiClassCount][AamiClassCount];
    inputData.copyAami(matrix);
    const Ec57BeatScores scores = Ec57BeatScores::fromMatrix(matrix);
    m_QrsPrediction = scores.qrs;
    m_NPrediction = scores.normal;
    m_PVCPrediction = scores.veb;
    m_SvebPrediction = scores.sveb;
}
