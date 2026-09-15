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

public:
    void insertBeat(const int &ref, int detected);
    int totalBeats() const
    {
        int total = 0;
        for (const auto& row : matrixBeatType)
            for (const auto& cell : row)
                total += cell;
        return total;
    }
};

#endif // RESULTMATRIX_H
