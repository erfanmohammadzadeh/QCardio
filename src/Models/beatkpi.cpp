#include "beatkpi.h"

BeatKPI::BeatKPI(ResultMatrix beatMatrix)
{
    inputData = beatMatrix;
    m_totalBeat = inputData.totalBeats();
}

void BeatKPI::calcNormalKPI()
{
    m_NPrediction.tp = inputData.getMatrix()[ArrhythmiaType::NormalArr][ArrhythmiaType::NormalArr];

    m_NPrediction.fn = inputData.getMatrix()[ArrhythmiaType::NormalArr][ArrhythmiaType::VentricularArr] +
                       inputData.getMatrix()[ArrhythmiaType::NormalArr][ArrhythmiaType::SupraventricularArr] +
                       inputData.getMatrix()[ArrhythmiaType::NormalArr][ArrhythmiaType::UnknownArr];

    m_NPrediction.fp = inputData.getMatrix()[ArrhythmiaType::VentricularArr][ArrhythmiaType::NormalArr] +
                       inputData.getMatrix()[ArrhythmiaType::SupraventricularArr][ArrhythmiaType::NormalArr] +
                       inputData.getMatrix()[ArrhythmiaType::UnknownArr][ArrhythmiaType::NormalArr];

    m_NPrediction.tn = inputData.getMatrix()[ArrhythmiaType::VentricularArr][ArrhythmiaType::VentricularArr] +
                       inputData.getMatrix()[ArrhythmiaType::VentricularArr][ArrhythmiaType::SupraventricularArr]+
                       inputData.getMatrix()[ArrhythmiaType::VentricularArr][ArrhythmiaType::UnknownArr]+
                       inputData.getMatrix()[ArrhythmiaType::VentricularArr][ArrhythmiaType::FusionArr]+
                       inputData.getMatrix()[ArrhythmiaType::SupraventricularArr][ArrhythmiaType::VentricularArr]+
                       inputData.getMatrix()[ArrhythmiaType::SupraventricularArr][ArrhythmiaType::FusionArr]+
                       inputData.getMatrix()[ArrhythmiaType::SupraventricularArr][ArrhythmiaType::SupraventricularArr]+
                       inputData.getMatrix()[ArrhythmiaType::SupraventricularArr][ArrhythmiaType::UnknownArr]+
                       inputData.getMatrix()[ArrhythmiaType::UnknownArr][ArrhythmiaType::VentricularArr]+
                       inputData.getMatrix()[ArrhythmiaType::UnknownArr][ArrhythmiaType::FusionArr]+
                       inputData.getMatrix()[ArrhythmiaType::UnknownArr][ArrhythmiaType::SupraventricularArr]+
                       inputData.getMatrix()[ArrhythmiaType::UnknownArr][ArrhythmiaType::UnknownArr];

    m_NPrediction.calcParams();
}

void BeatKPI::calcPVCKPI()
{
    m_PVCPrediction.tp = inputData.getMatrix()[ArrhythmiaType::VentricularArr][ArrhythmiaType::VentricularArr];

    m_PVCPrediction.fn = inputData.getMatrix()[ArrhythmiaType::VentricularArr][ArrhythmiaType::NormalArr] +
                         inputData.getMatrix()[ArrhythmiaType::VentricularArr][ArrhythmiaType::SupraventricularArr] +
                         inputData.getMatrix()[ArrhythmiaType::VentricularArr][ArrhythmiaType::UnknownArr];

    m_PVCPrediction.fp = inputData.getMatrix()[ArrhythmiaType::NormalArr][ArrhythmiaType::VentricularArr] +
                         inputData.getMatrix()[ArrhythmiaType::SupraventricularArr][ArrhythmiaType::VentricularArr] +
                         inputData.getMatrix()[ArrhythmiaType::UnknownArr][ArrhythmiaType::VentricularArr];

    m_PVCPrediction.tn = inputData.getMatrix()[ArrhythmiaType::NormalArr][ArrhythmiaType::NormalArr] +
                         inputData.getMatrix()[ArrhythmiaType::NormalArr][ArrhythmiaType::SupraventricularArr]+
                         inputData.getMatrix()[ArrhythmiaType::NormalArr][ArrhythmiaType::UnknownArr]+
                         inputData.getMatrix()[ArrhythmiaType::NormalArr][ArrhythmiaType::FusionArr]+
                         inputData.getMatrix()[ArrhythmiaType::SupraventricularArr][ArrhythmiaType::NormalArr]+
                         inputData.getMatrix()[ArrhythmiaType::SupraventricularArr][ArrhythmiaType::FusionArr]+
                         inputData.getMatrix()[ArrhythmiaType::SupraventricularArr][ArrhythmiaType::SupraventricularArr]+
                         inputData.getMatrix()[ArrhythmiaType::SupraventricularArr][ArrhythmiaType::UnknownArr]+
                         inputData.getMatrix()[ArrhythmiaType::UnknownArr][ArrhythmiaType::NormalArr]+
                         inputData.getMatrix()[ArrhythmiaType::UnknownArr][ArrhythmiaType::FusionArr]+
                         inputData.getMatrix()[ArrhythmiaType::UnknownArr][ArrhythmiaType::SupraventricularArr]+
                         inputData.getMatrix()[ArrhythmiaType::UnknownArr][ArrhythmiaType::UnknownArr];
    m_PVCPrediction.calcParams();
}

void BeatKPI::run()
{
    calcNormalKPI();
    calcPVCKPI();
}
