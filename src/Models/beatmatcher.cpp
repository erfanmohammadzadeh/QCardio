#include "beatmatcher.h"

#include <QtGlobal>

#include <cstdlib>
#include <limits>

namespace {

bool closerOrEqual(int distance, int otherDistance, int window)
{
    return distance <= otherDistance && distance <= window;
}

void appendQrs(const QVector<int> &samples,
               const QVector<quint8> &types,
               int firstSample,
               int lastSample,
               QVector<AnnotatedBeat> &out)
{
    const int count = qMin(samples.size(), types.size());
    for (int i = 0; i < count; ++i) {
        const int type = static_cast<int>(types.at(i));
        if (!isQrsBeat(type))
            continue;
        if (samples.at(i) < firstSample || samples.at(i) > lastSample)
            continue;
        out.append(AnnotatedBeat{samples.at(i), type});
    }
}

} // namespace

BeatMatchResult matchBeats(const QVector<int> &refSamples,
                           const QVector<quint8> &refTypes,
                           const QVector<int> &testSamples,
                           const QVector<quint8> &testTypes,
                           int sampleRate,
                           int learningEndSample)
{
    BeatMatchResult result;
    const int window = matchWindowSamples(sampleRate);
    const int learningEnd = qMax(0, learningEndSample);
    const int graceStart = qMax(0, learningEnd - window);

    appendQrs(refSamples, refTypes, learningEnd, std::numeric_limits<int>::max(), result.referenceBeats);

    QVector<AnnotatedBeat> testStream;
    appendQrs(testSamples, testTypes, graceStart, std::numeric_limits<int>::max(), testStream);
    appendQrs(testSamples, testTypes, learningEnd, std::numeric_limits<int>::max(), result.testBeats);

    const int refCount = result.referenceBeats.size();
    const int testCount = testStream.size();
    int refIndex = 0;
    int testIndex = 0;

    auto emitPair = [&](int refAt, int testAt) {
        AlignedPair pair;
        if (refAt >= 0) {
            pair.refSample = result.referenceBeats.at(refAt).sample;
            pair.refType = result.referenceBeats.at(refAt).type;
        }
        if (testAt >= 0) {
            pair.testSample = testStream.at(testAt).sample;
            pair.testType = testStream.at(testAt).type;
        }
        if (refAt >= 0 && testAt >= 0) {
            pair.sampleDelta = std::abs(pair.refSample - pair.testSample);
            pair.typeMatch = aamiClassOf(pair.refType) == aamiClassOf(pair.testType);
        }
        result.pairs.append(pair);
    };

    auto countTestBeat = [&](int testAt) {
        return testStream.at(testAt).sample >= learningEnd;
    };

    while (refIndex < refCount || testIndex < testCount) {
        if (refIndex >= refCount) {
            if (countTestBeat(testIndex))
                emitPair(-1, testIndex);
            ++testIndex;
            continue;
        }
        if (testIndex >= testCount) {
            emitPair(refIndex, -1);
            ++refIndex;
            continue;
        }

        const int referenceTime = result.referenceBeats.at(refIndex).sample;
        const int testTime = testStream.at(testIndex).sample;

        if (testTime < referenceTime) {
            const int nextTestTime = (testIndex + 1 < testCount)
                                         ? testStream.at(testIndex + 1).sample
                                         : std::numeric_limits<int>::max();
            const int distance = referenceTime - testTime;
            const int nextDistance = std::abs(nextTestTime - referenceTime);
            if (closerOrEqual(distance, nextDistance, window)) {
                emitPair(refIndex, testIndex);
                ++refIndex;
                ++testIndex;
            } else {
                if (countTestBeat(testIndex))
                    emitPair(-1, testIndex);
                ++testIndex;
            }
        } else {
            const int nextReferenceTime = (refIndex + 1 < refCount)
                                              ? result.referenceBeats.at(refIndex + 1).sample
                                              : std::numeric_limits<int>::max();
            const int distance = testTime - referenceTime;
            const int nextDistance = std::abs(nextReferenceTime - testTime);
            if (closerOrEqual(distance, nextDistance, window)) {
                emitPair(refIndex, testIndex);
                ++refIndex;
                ++testIndex;
            } else {
                emitPair(refIndex, -1);
                ++refIndex;
            }
        }
    }

    return result;
}
