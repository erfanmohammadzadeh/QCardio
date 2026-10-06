#ifndef BEATTESTPROFILE_H
#define BEATTESTPROFILE_H

#include <QString>
#include <QVector>

// WFDB has no standard code for an interpolated ventricular beat.
// CSV files can mark one with this application code (valid user annotation range is 42..49).
constexpr int kInterpolatedAnnotation = 42;

// Labels that can appear in the compare report.
// ClassMiss is an unmatched beat (a miss or an extra detection), not a user-selectable type.
enum TestBeatClass {
    ClassNormal = 0,
    ClassBundle = 1,
    ClassSupra = 2,
    ClassPvc = 3,
    ClassFusion = 4,
    ClassEscape = 5,
    ClassInterpolated = 6,
    ClassUnknown = 7,
    ClassMiss = 8,
    TestBeatClassCount = 9,
    ClassExcluded = -1
};

// Which tables and beat types a compare run scores.
// A subtype that is not scored separately is counted as its group.
// With only the Normal and PVC groups selected, fusion counts as PVC,
// and a fusion beat matched to a PVC is a true positive.
struct BeatTestProfile {
    bool reportBeats = true;
    bool reportQrs = true;
    bool reportShutdown = true;
    bool reportVentricularRuns = true;
    bool reportSupraventricularRuns = true;

    bool groupNormal = true;
    bool separateNormal = false;
    bool separateBundle = false;
    bool separateSupra = false;

    bool groupPvc = true;
    bool separatePvc = false;
    bool separateFusion = false;
    bool separateEscape = false;
    bool separateInterpolated = false;

    bool groupUnknown = true;

    bool anyTable() const;
    bool classSelected(int beatClass) const;
    QVector<int> activeClasses() const;
};

struct TestClassCounts {
    int tp = 0;
    int fn = 0;
    int fp = 0;
    int tn = 0;
};

int leafClassOf(int annotationCode);
int resolveTestClass(int annotationCode, const BeatTestProfile &profile);
const char *testClassLetter(int beatClass);
QString testClassName(int beatClass);

void accumulateTestBeat(quint32 matrix[TestBeatClassCount][TestBeatClassCount],
                        int refType,
                        int testType,
                        const BeatTestProfile &profile);

TestClassCounts scoreTestClass(const quint32 matrix[TestBeatClassCount][TestBeatClassCount],
                               int beatClass,
                               const QVector<int> &active);

#endif // BEATTESTPROFILE_H
