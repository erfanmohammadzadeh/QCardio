# QCardio

Qt desktop app for viewing PhysioNet WFDB ECG records, exporting them, and comparing algorithm CSV output against a reference annotation set (MIT-BIH, AHA, and similar PhysioNet-style databases).

## Features

- Read WFDB records (`.hea` / `.dat`) and beat annotations (`.atr`)
- Plot up to three leads with annotation labels
- Resample to a target frequency, with gain and offset
- Export a single record or the full list as RC7 or raw samples (optional CSV)
- Compare two CSV directories and write `Report.csv`
- Footer progress bar for **export-all** and **compare** (`Compare current/total`)

## Architecture

Application code lives under `src/` in a standard MVC + service layout. The PhysioNet WFDB C library is not mixed with app modules; it is vendored under `src/ThirdParty/wfdb/`.

```
src/
  main.cpp
  QCardio.pro
  Controllers/     AppController, SignalViewController
  Views/           MainWindow, SignalViewWidget
  Models/          ECG/CSV types, sheet analysis, settings DTO
  Services/        WFDB, annotation, export, analyse, settings, log
  ThirdParty/wfdb/ PhysioNet WFDB C sources
```

| Layer | Responsibility |
|---|---|
| **Views** | UI only. Emits requests (read, export, compare). |
| **Controllers** | Wire views to services. `AppController` owns the main window and runs background jobs. |
| **Services** | I/O and business logic (`WfdbService`, `AnnotationService`, `ExportService`, `AnalyseService`, `SettingsService`, `LogService`). |
| **Models** | Data structures (`MIT_BIH_ECGData`, `AnalyseCfg`, …) and sheet/KPI helpers. |
| **ThirdParty** | Unmodified WFDB C library used by `WfdbService` / `AnnotationService`. |

Flow: **View** → **AppController** → **Service** → **Model**; results go back to the view (plot, log, progress bar).

## Build

- Qt 6.8 (Widgets + Concurrent) and a C++17 compiler (MinGW 64-bit is used in the current kit)
- libcurl (Windows path in `QCardio.pro`: `C:/curl-8.20.0_5`)

Open `src/QCardio.pro` in Qt Creator and build, or from a kit that already has `qmake` and the compiler on `PATH`:

```bash
cd src
qmake QCardio.pro
make
```

## Usage

### View and export records

1. Set the database directory (folder that contains `.hea` files).
2. Select a record and click **Read**.
3. Choose RC7 or raw sample, output folder, and **Export** (or export all).

See also [sample/README.md](sample/README.md).

### Compare CSV outputs

1. Open the compare tab.
2. **Select** the reference CSV directory (Data 1) and the algorithm CSV directory (Data 2). Both must contain the same number of files.
3. Click **Compare** and choose the result directory.
4. The footer bar shows `Compare n/m` while each pair is processed. Read / Export / Compare stay disabled until the job finishes.
5. `Report.csv` is written in the result directory.

AHA records can be converted to WFDB first; see [scripts/README.md](scripts/README.md).

## License / data

MIT-BIH and other PhysioNet databases are used under their own terms. WFDB is the PhysioNet WFDB Software Package (see [PhysioNet WFDB](https://www.physionet.org/content/wfdb/10.7.0/)).
