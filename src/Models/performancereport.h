#ifndef PERFORMANCEREPORT_H
#define PERFORMANCEREPORT_H

#include "Models/global_qcardio.h"

#include <QString>

namespace PerformanceReport {

// Writes the IEC 60601-2-47:2012 Table 201.103 / 201.104 evaluation files:
//   <report>                 record-by-record beat matrix, TP/FN/FP/TN, Se, +P, FPR
//   <report base>_shutdown   shutdown (Table AA.5)
//   <report base>_runs       ventricular and supraventricular couplet / run statistics
bool write(const QString &reportPath, const AnalyseFileProcessResult &result);

}

#endif // PERFORMANCEREPORT_H
