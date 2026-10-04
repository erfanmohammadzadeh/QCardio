# Changelog

All notable changes to QCardio are recorded here. The application version is `MAJOR.MINOR.PATCH` from `src/Models/define.h`.

## 1.0.0 — 2026-10-03

First release of the desktop application.

### Viewing

- Read local WFDB records (`.hea` / `.dat`) and beat annotations (`.atr`).
- Plot up to three leads, mapped to I, II, V, or none, with annotation labels.
- Remember the database path, database name, lead mapping, and export settings between sessions.

### Export

- Resample from a source frequency to a target frequency. Defaults are 360 Hz to 178 Hz, gain 1.0, and DC offset 0.
- Database presets: MIT-BIH uses 360 Hz; AHA uses 250 Hz and a compare offset of 460; ESC and CU use a compare offset of 4.
- Export the selected record, or every record in the list, as raw samples (`.bin`) or Rec7 (`.rc7`).
- Optionally write a beat CSV next to the signal file.
- Show footer progress while export-all is running. Read, Export, and Compare stay disabled until the job finishes.

### Compare

- Compare a reference CSV directory (Data 1) with an algorithm CSV directory (Data 2). Both directories must contain the same number of `.csv` files. Files are paired in list order.
- Ignore the first *N* beats, where *N* is **Compare Ofset**, when counting detection and classification results.
- Write `Report.csv`, `Report.csva.csv`, `Report.csvb.csv`, and one alignment CSV per pair into a `Compare` folder created beside the parent of the algorithm directory.
- Show footer progress as `Compare current/total`.

### Tests

- Qt Test target `tst_qcardio` covers directory checks, CSV read and write, beat KPIs, compare report output, raw export, and the log service.
- WFDB record reading is not part of this test target.

### Known limitations

- CSV pairs follow the order of each file list. The lists are not sorted by file name, so corresponding records need to appear in the same order in both directories.
- In `Report.csvb.csv`, the summary cell **Gross QRS** contains the average normal-beat positive predictive value, and **Gross Normal** contains the average QRS positive predictive value. **Gross PVC** is the average PVC positive predictive value.
- In `Report.csvb.csv`, the percentage printed with **Q_FN** is the normal-beat false-negative rate. The integer in that cell is the QRS false-negative count.
- The Compression checkbox is stored with the export settings. Rec7 output is always written with Qt compression.
