#ifndef BEATMATCHER_H
#define BEATMATCHER_H

#include "Models/aamiclass.h"

#include <QVector>

struct AnnotatedBeat {
    int sample = 0;
    int type = 0;
};

struct AlignedPair {
    int refSample = -1;
    int testSample = -1;
    int refType = -1;
    int testType = -1;
    int sampleDelta = -1;
    bool typeMatch = false;
};

struct BeatMatchResult {
    QVector<AlignedPair> pairs;
    QVector<AnnotatedBeat> referenceBeats;
    QVector<AnnotatedBeat> testBeats;
};

// Beat-by-beat pairing of IEC 60601-2-47:2012 clause 201.12.1.101.2.3.2.
// learningEndSample is the first sample of the test period (0 keeps the whole record).
BeatMatchResult matchBeats(const QVector<int> &refSamples,
                           const QVector<quint8> &refTypes,
                           const QVector<int> &testSamples,
                           const QVector<quint8> &testTypes,
                           int sampleRate,
                           int learningEndSample);

#endif // BEATMATCHER_H
