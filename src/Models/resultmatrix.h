#ifndef RESULTMATRIX_H
#define RESULTMATRIX_H
#include <QList>
#include <Models/global_qcardio.h>


class ResultMatrix
{
public:
    ResultMatrix(MatrixType type = MatrixType::BeatType);

    BeatMatrixType& getMatrix();
    RunMatrixType& getRunMatrix();
    quint32* getDif();

private:
    BeatMatrixType matrixBeatType = {{0}};
    quint32 matrixRun[6][6] = {{0}};
    quint32 m_difBeatType[ArrhythmiaType::ArrhythmiaTypeCount];
    MatrixType m_matrixType;
    quint32 m_aami[AamiClassCount][AamiClassCount] = {};

public:
    void insertBeat(const int &ref, int detected);
    // refAnn or detAnn below zero is an unmatched beat (class O).
    void addComparison(int refAnn, int detAnn);
    void copyAami(quint32 dest[AamiClassCount][AamiClassCount]) const;
    int totalBeats() const
    {
        int total = 0;
        for (const auto &row : m_aami)
            for (quint32 cell : row)
                total += static_cast<int>(cell);
        return total;
    }
};

#endif // RESULTMATRIX_H
