# User guide

This guide describes QCardio 1.0.0: opening a WFDB database, exporting records, and comparing beat CSV files. Screenshots of the main screens are in [sample/README.md](../sample/README.md).

## Before you start

- Install a QCardio 1.0.0 build. See [build.md](build.md).
- Keep the WFDB database on a local disk. A record is a header (`.hea`), a signal file (`.dat`), and, for annotated records, an annotation file (`.atr`).
- MIT-BIH can be downloaded from [PhysioNet](https://physionet.org/content/mitdb/1.0.0/). AHA records can be converted to WFDB first; see [scripts/README.md](../scripts/README.md).

The Info tab shows the running version (`1.0.0`). **Help** on that tab opens the bundled PhysioNet help PDF.

Read, Export, and Compare are disabled while an export-all or compare job is running.

## View a record

1. Open the **Database** tab.
2. Set the database directory. It must be the folder that contains `.hea` files. The record list includes headers found in subfolders. Each base name is listed once.
3. Select a record and click **Read**.

The plot shows the selected leads and annotation symbols. Map each of the three signal slots to **I**, **II**, **V**, or **None** on the Export tab before you export. Those choices are saved with the other interface settings.

## Export

Open the **Export** tab.

| Control | 1.0.0 behavior |
|---|---|
| Source Default Frequency | Sampling rate of the WFDB record used for resampling. Default 360 Hz. Choosing **MIT-BIH** sets 360. Choosing **AHA** sets 250. |
| Target Frequency | Rate written into the export and used later as the compare sample rate. Default 178 Hz. Allowed range 50–1000. |
| Target Gain | Multiplier applied to samples. Default 1.0. Minimum 0.1. |
| Target DC Offset | Offset added to samples. Default 0. Range −2000 to 2000. |
| Database Name | `MIT-BIH`, `AHA`, `ESC`, or `CU`. This name is the prefix of exported file names. |
| Export Format | **Raw Data** or **Rec7**. |
| Export CSV | When checked (the default), a beat CSV is written next to the signal file. |
| Export All Record | When checked, every record in the database list is exported. The footer progress bar advances as each file finishes. |

Click **Export** and choose the output folder.

### Output files

For a record `100` and database name `MIT-BIH`:

| Format | Signal file | Optional annotations |
|---|---|---|
| Raw Data | `MIT-BIH_100.bin` | `MIT-BIH_100.csv` |
| Rec7 | `MIT-BIH_100.rc7` | `MIT-BIH_100.csv` |

Raw files are a Qt `QDataStream` (version Qt 5.15) sequence of samples. Each sample is three 16-bit leads (I, II, V), a 16-bit status, an 8-bit battery level, and an 8-bit lead-fail flag. Unmapped leads are zero.

Rec7 is a packet file: 840 samples per packet, five bytes per sample, header `0xAA`, footer `0xCC`, then compressed with Qt `qCompress`. The file name ends in `.rc7`.

The beat CSV has no header. Each line is:

```text
sampleIndex,annotationCode
```

`sampleIndex` is the annotation time in samples. `annotationCode` is the WFDB annotation type (`ecgcodes.h`), stored as an integer. Rhythm markers and other non-beat symbols (including `[`, `]`, `(`, `)`, `p`, `t`, `+`, and `~`) are omitted. Lines that are empty, have fewer than two fields, or are not integers are ignored when a CSV is read back.

## Compare CSV files

Open the **Compare** tab.

1. Click **Select** under **Reference Standard Dataset** and choose the reference folder (Data 1).
2. Click **Select** under **Selected Dataset For Compare** and choose the algorithm folder (Data 2).
3. Set **Compare Ofset** if the first beats of each file should be left out of the scores.
4. Click **Compare**.

Both folders must contain the same number of `.csv` files. Search includes subfolders. Files are paired by position in the two lists, not by matching file names, and the lists are not sorted. Put corresponding records in the same order in both trees.

The result directory is created automatically. If the algorithm folder is `D:\work\algorithm`, the report folder is `D:\work\Compare` (the `Compare` directory next to the parent of Data 2). The footer shows `Compare n/m` until every pair has been processed.

### What is compared

Each CSV is a list of `(sample index, annotation code)` rows, as in the export format above.

A reference beat matches a detection when the absolute sample difference is from 0 up to, and not including, 100, and detections are taken in time order. Beats at index `0` through **Compare Ofset** are aligned in the per-record file but are left out of the counts. Changing the database name sets this offset: AHA sets 460, ESC and CU set 4, and MIT-BIH leaves the current offset unchanged while setting the source frequency to 360.

QRS sensitivity and positive predictive value use those matched, missed, and extra beats. Normal and ventricular scores use the beat-type matrix. WFDB codes in the normal set (`NORMAL`, bundle-branch blocks, `SVPB`, and related codes in that set) count as normal when both sides fall in the set. Ventricular, fusion, paced, ventricular escape, and the other codes in the ventricular set count as ventricular when both sides fall in that set.

Sensitivity (`Se`) is the percentage of reference beats of that class that were detected as the same class. Positive predictive value (`+P`) is the percentage of detections of that class that match the reference class. If a class has no reference beats, ventricular sensitivity is written as `nan`.

**Target Frequency** on the Export tab is the sample rate used to turn a run of missed beats into **total shudown time**.

### Report files

All of these files are written in the `Compare` folder.

#### `Report.csv`

One row per record.

| Column | Meaning |
|---|---|
| FileName | `referenceBaseVSalgorithmBase` |
| Nn, Vn, On | Reference normal, ventricular, or other, detected as normal |
| Nv, Vv, Ov | Reference normal, ventricular, or other, detected as ventricular |
| Q Se, Q+P | QRS sensitivity and positive predictive value, percent |
| N Se, N+P | Normal-beat sensitivity and positive predictive value, percent |
| V Se, V+P | Ventricular sensitivity and positive predictive value, percent. `V Se` is `nan` when it cannot be computed |
| VTN | Ventricular true negatives |
| V FPR | Ventricular false-positive rate, percent |

#### `Report.csva.csv`

Missed beats for the same records. The file name is `Report.csv` with `a.csv` appended.

| Column | Meaning |
|---|---|
| FileName | Same pair name as in `Report.csv` |
| Nx, Vx, Qx | Reference normal, ventricular, or other, with no matching detection (counted as unknown) |
| beats missed | Reference beats with no detection, after the compare offset |
| %N missed, %V missed | Missed normal and ventricular beats as a percentage of the reference beat count |
| total shudown time | Duration of gaps between missed beats, `hh:mm:ss`, using Target Frequency |

#### `Report.csvb.csv`

Per-record detection counts. Each value is `count (percent)`. The file name is `Report.csv` with `b.csv` appended.

Columns: `FileName`, then `N_TP(%)`, `V_TP(%)`, `Q_TP(%)`, `N_FN(%)`, `V_FN(%)`, `Q_FN(%)`, `N_FP(%)`, `V_FP(%)`, `Q_FP(%)`, `N_TN(%)`, `V_TN(%)`, `Q_TN(%)`.

`N` is normal, `V` is ventricular, and `Q` is QRS detection. TP, FN, FP, and TN are true positive, false negative, false positive, and true negative.

A summary block follows the record rows:

| Column | Meaning |
|---|---|
| Avg Se QRS, Avg Se PVC, Avg Se Normal | Mean sensitivity across records whose sensitivity is greater than zero |
| Gross QRS, Gross PVC, Gross Normal | Mean positive predictive value. See the note below |

In this release, **Gross QRS** is filled with the average normal-beat positive predictive value, and **Gross Normal** is filled with the average QRS positive predictive value. **Gross PVC** is the average ventricular positive predictive value. The percentage shown in **Q_FN** is the normal-beat false-negative rate; the count in that cell is the QRS false-negative count.

#### Alignment CSV

One file per pair, named `referenceBaseVSalgorithmBase.csv`. There is no header. Columns, in order:

1. Reference sample index, or `-1` if this row is an extra detection
2. Algorithm sample index, or `-1` if this row is a missed reference beat
3. Absolute sample difference, or `-1` if the beats were not paired
4. `QRS Macth` or `QRS not Macth` (the spelling in the file). A paired row is labeled a match when the difference divided by 178, using integer division, is less than 3
5. Reference annotation code, or `-1`
6. Algorithm annotation code, or `-1`
7. `Type Macth` or `Type Not Macth`, using the same normal / ventricular / noise groups as the report

## Presets

| Database name | Source frequency | Compare offset |
|---|---|---|
| MIT-BIH | 360 Hz | unchanged |
| AHA | 250 Hz | 460 |
| ESC | unchanged | 4 |
| CU | unchanged | 4 |

Target frequency stays at the value in the spin box (178 Hz until you change it) for every database name.
