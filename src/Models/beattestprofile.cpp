#include "beattestprofile.h"

#include "wfdb/ecgcodes.h"

bool BeatTestProfile::anyTable() const
{
    return reportBeats || reportQrs || reportShutdown
        || reportVentricularRuns || reportSupraventricularRuns;
}

bool BeatTestProfile::classSelected(int beatClass) const
{
    switch (beatClass) {
    case ClassNormal:
        return groupNormal;
    case ClassBundle:
        return groupNormal && separateBundle;
    case ClassSupra:
        return groupNormal && separateSupra;
    case ClassPvc:
        return groupPvc;
    case ClassFusion:
        return groupPvc && separateFusion;
    case ClassEscape:
        return groupPvc && separateEscape;
    case ClassInterpolated:
        return groupPvc && separateInterpolated;
    case ClassUnknown:
        return groupUnknown;
    default:
        return false;
    }
}

QVector<int> BeatTestProfile::activeClasses() const
{
    QVector<int> classes;
    const int order[] = {
        ClassNormal, ClassBundle, ClassSupra,
        ClassPvc, ClassFusion, ClassEscape, ClassInterpolated,
        ClassUnknown
    };
    for (int beatClass : order) {
        if (classSelected(beatClass))
            classes.append(beatClass);
    }
    return classes;
}

int leafClassOf(int annotationCode)
{
    switch (annotationCode) {
    case NORMAL:
    case NESC:
    case AESC:
    case SVESC:
        return ClassNormal;
    case LBBB:
    case RBBB:
    case BBB:
        return ClassBundle;
    case ABERR:
    case NPC:
    case APC:
    case SVPB:
        return ClassSupra;
    case PVC:
    case RONT:
        return ClassPvc;
    case FUSION:
        return ClassFusion;
    case VESC:
        return ClassEscape;
    case kInterpolatedAnnotation:
        return ClassInterpolated;
    case UNKNOWN:
    case PACE:
    case PFUS:
        return ClassUnknown;
    default:
        return ClassExcluded;
    }
}

int resolveTestClass(int annotationCode, const BeatTestProfile &profile)
{
    const int leaf = leafClassOf(annotationCode);
    if (leaf == ClassExcluded)
        return ClassExcluded;

    switch (leaf) {
    case ClassNormal:
        if (!profile.groupNormal)
            return ClassExcluded;
        return ClassNormal;
    case ClassBundle:
        if (!profile.groupNormal)
            return ClassExcluded;
        return profile.separateBundle ? ClassBundle : ClassNormal;
    case ClassSupra:
        if (!profile.groupNormal)
            return ClassExcluded;
        return profile.separateSupra ? ClassSupra : ClassNormal;
    case ClassPvc:
        if (!profile.groupPvc)
            return ClassExcluded;
        return ClassPvc;
    case ClassFusion:
        if (!profile.groupPvc)
            return ClassExcluded;
        return profile.separateFusion ? ClassFusion : ClassPvc;
    case ClassEscape:
        if (!profile.groupPvc)
            return ClassExcluded;
        return profile.separateEscape ? ClassEscape : ClassPvc;
    case ClassInterpolated:
        if (!profile.groupPvc)
            return ClassExcluded;
        return profile.separateInterpolated ? ClassInterpolated : ClassPvc;
    case ClassUnknown:
        return profile.groupUnknown ? ClassUnknown : ClassExcluded;
    default:
        return ClassExcluded;
    }
}

const char *testClassLetter(int beatClass)
{
    switch (beatClass) {
    case ClassNormal: return "N";
    case ClassBundle: return "B";
    case ClassSupra: return "S";
    case ClassPvc: return "V";
    case ClassFusion: return "F";
    case ClassEscape: return "E";
    case ClassInterpolated: return "I";
    case ClassUnknown: return "Q";
    case ClassMiss: return "o";
    default: return "?";
    }
}

QString testClassName(int beatClass)
{
    switch (beatClass) {
    case ClassNormal: return QStringLiteral("Normal");
    case ClassBundle: return QStringLiteral("Bundle branch");
    case ClassSupra: return QStringLiteral("Supraventricular");
    case ClassPvc: return QStringLiteral("PVC");
    case ClassFusion: return QStringLiteral("Fusion");
    case ClassEscape: return QStringLiteral("Escape");
    case ClassInterpolated: return QStringLiteral("Interpolated");
    case ClassUnknown: return QStringLiteral("Unknown");
    default: return QString();
    }
}

void accumulateTestBeat(quint32 matrix[TestBeatClassCount][TestBeatClassCount],
                        int refType,
                        int testType,
                        const BeatTestProfile &profile)
{
    const int refClass = refType < 0 ? ClassMiss : resolveTestClass(refType, profile);
    const int detClass = testType < 0 ? ClassMiss : resolveTestClass(testType, profile);
    if (refClass == ClassExcluded || detClass == ClassExcluded)
        return;
    if (refClass == ClassMiss && detClass == ClassMiss)
        return;
    if (refClass < 0 || detClass < 0 || refClass >= TestBeatClassCount || detClass >= TestBeatClassCount)
        return;
    ++matrix[refClass][detClass];
}

TestClassCounts scoreTestClass(const quint32 matrix[TestBeatClassCount][TestBeatClassCount],
                               int beatClass,
                               const QVector<int> &active)
{
    QVector<int> indexes = active;
    indexes.append(ClassMiss);

    int total = 0;
    int rowSum = 0;
    int colSum = 0;
    for (int row : indexes) {
        for (int column : indexes) {
            const int value = static_cast<int>(matrix[row][column]);
            total += value;
            if (row == beatClass)
                rowSum += value;
            if (column == beatClass)
                colSum += value;
        }
    }

    TestClassCounts counts;
    counts.tp = static_cast<int>(matrix[beatClass][beatClass]);
    counts.fn = rowSum - counts.tp;
    counts.fp = colSum - counts.tp;
    counts.tn = total - rowSum - colSum + counts.tp;
    return counts;
}
