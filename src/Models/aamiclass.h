#ifndef AAMICLASS_H
#define AAMICLASS_H

#include "wfdb/ecgcodes.h"

// Beat classes used by IEC 60601-2-47:2012 Table 201.105 and Annex AA.
// A confusion-matrix row is the reference label and a column is the algorithm label.
enum AamiClass {
    AamiN = 0,
    AamiS = 1,
    AamiV = 2,
    AamiF = 3,
    AamiQ = 4,
    AamiO = 5, // unmatched beat: a miss or an extra detection
    AamiX = 6, // beat during shutdown / unreadable interval
    AamiClassCount = 7,
    AamiNotQrs = -1
};

// Run-length bins of IEC 60601-2-47:2012 Table 201.108 / 201.109.
// 0 = none, 1 = isolated beat, 2 = couplet, 3..5 = short run, 6 = long run (6+).
constexpr int IecRunBinCount = 7;

// IEC 60601-2-47:2012 clause 201.12.1.101.1.1.1: the first 5 minutes are the learning period.
constexpr int IecLearningPeriodSeconds = 5 * 60;

// Match window of clause 201.12.1.101.2.3.1: 150 ms.
constexpr double IecMatchWindowSeconds = 0.150;

AamiClass aamiClassOf(int annotationCode);
bool isQrsBeat(int annotationCode);
int matchWindowSamples(int sampleRate);
int runLengthBin(int beatCount);
const char *aamiClassName(int aamiClass);

// Percent in the range 0..100. Returns -1 when the denominator is zero (undefined).
inline float ec57Percent(int numerator, int denominator)
{
    if (denominator <= 0)
        return -1.0f;
    return 100.0f * static_cast<float>(numerator) / static_cast<float>(denominator);
}

#endif // AAMICLASS_H
