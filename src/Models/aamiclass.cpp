#include "aamiclass.h"

#include <QtGlobal>

AamiClass aamiClassOf(int annotationCode)
{
    // ANSI/AAMI EC57 mapping adopted by IEC 60601-2-47:2012.
    // N: normal, bundle-branch, and escape beats.
    // S: supraventricular premature beats.
    // V: ventricular ectopic beats, including escape and R-on-T.
    // F: fusion of a ventricular beat and a normal beat.
    // Q: paced, paced-fusion, and unclassifiable beats.
    // Fusion (F) and unclassifiable (Q) beats are excluded from VEB false positives.
    switch (annotationCode) {
    case NORMAL:
    case LBBB:
    case RBBB:
    case BBB:
    case NESC:
    case AESC:
    case SVESC:
        return AamiN;
    case ABERR:
    case NPC:
    case APC:
    case SVPB:
        return AamiS;
    case PVC:
    case VESC:
    case RONT:
        return AamiV;
    case FUSION:
        return AamiF;
    case PACE:
    case PFUS:
    case UNKNOWN:
        return AamiQ;
    default:
        return AamiNotQrs;
    }
}

bool isQrsBeat(int annotationCode)
{
    return aamiClassOf(annotationCode) != AamiNotQrs;
}

int matchWindowSamples(int sampleRate)
{
    if (sampleRate <= 0)
        return 1;
    return qMax(1, qRound(sampleRate * IecMatchWindowSeconds));
}

int runLengthBin(int beatCount)
{
    if (beatCount <= 0)
        return 0;
    if (beatCount >= IecRunBinCount - 1)
        return IecRunBinCount - 1;
    return beatCount;
}

const char *aamiClassName(int aamiClass)
{
    switch (aamiClass) {
    case AamiN: return "N";
    case AamiS: return "S";
    case AamiV: return "V";
    case AamiF: return "F";
    case AamiQ: return "Q";
    case AamiO: return "O";
    case AamiX: return "X";
    default: return "?";
    }
}
