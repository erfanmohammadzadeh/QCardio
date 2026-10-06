#ifndef EC57METRICS_H
#define EC57METRICS_H

#include "Models/aamiclass.h"
#include "Models/beatmatcher.h"
#include "Models/global_qcardio.h"

struct Ec57BeatScores {
    Predicting qrs;
    Predicting normal;
    Predicting veb;
    Predicting sveb;

    static Ec57BeatScores fromMatrix(const quint32 matrix[AamiClassCount][AamiClassCount]);
};

struct Ec57RunScores {
    Predicting couplet;
    Predicting shortRun;
    Predicting longRun;

    static Ec57RunScores fromMatrices(const quint32 sensitivity[IecRunBinCount][IecRunBinCount],
                                      const quint32 predictivity[IecRunBinCount][IecRunBinCount]);
};

// Fills the run sensitivity matrix (reference runs) and the positive-predictivity
// matrix (algorithm runs) defined in IEC 60601-2-47:2012 clause 201.12.1.101.2.4.
void fillRunMatrices(const QVector<AnnotatedBeat> &reference,
                     const QVector<AnnotatedBeat> &test,
                     int sampleRate,
                     bool ventricular,
                     quint32 sensitivity[IecRunBinCount][IecRunBinCount],
                     quint32 predictivity[IecRunBinCount][IecRunBinCount]);

int matrixSum(const quint32 matrix[AamiClassCount][AamiClassCount], int row, int column);
int referenceClassCount(const quint32 matrix[AamiClassCount][AamiClassCount], int referenceClass);

#endif // EC57METRICS_H
