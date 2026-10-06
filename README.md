# QCardio 

Qt desktop application for viewing PhysioNet WFDB electrocardiogram records, exporting them, and comparing an algorithm’s beat CSV files with a reference annotation set.

**Version:** 1.0.0  
**Status:** Release  
**Platform:** Windows desktop (Qt 6.8, MinGW 64-bit). A Unix `qmake` build is also defined in the project file.

The version shown on the Info tab is `SOFTWARE_VERSION_STR` from [`src/Models/define.h`](src/Models/define.h).

## What you can do

- Open a local WFDB database (`.hea`, `.dat`, and beat annotations in `.atr`).
- Plot up to three leads and show annotation labels.
- Resample to a target sampling frequency, and apply gain and DC offset.
- Export one record or every record in the list as raw samples or Rec7, with an optional beat CSV.
- Compare a reference CSV directory with an algorithm CSV directory and write a `Compare` report folder.
- Show footer progress for export-all and compare. Compare uses the label `Compare current/total`.

Supported database names in the Export tab are MIT-BIH, AHA, ESC, and CU.

## Documentation

| Document | Contents |
|---|---|
| [docs/user-guide.md](docs/user-guide.md) | Viewing, export, compare, CSV layout, and report files |
| [docs/build.md](docs/build.md) | Requirements, build, and automated tests |
| [sample/README.md](sample/README.md) | Illustrated walkthrough |
| [scripts/README.md](scripts/README.md) | Convert AHA records to WFDB with the PhysioNet command-line tools |
| [CHANGELOG.md](CHANGELOG.md) | 1.0.0 release notes |

The in-application **Help** button opens the bundled PhysioNet help PDF.

## Quick start

1. Build the application. See [docs/build.md](docs/build.md).
2. On the Database tab, set the folder that contains `.hea` files, select a record, and click **Read**.
3. On the Export tab, choose raw data or Rec7, an output folder, and click **Export**.
4. On the Compare tab, select the reference CSV folder and the algorithm CSV folder, then click **Compare**.

Results are written to a `Compare` directory next to the parent of the algorithm folder. Details are in the [user guide](docs/user-guide.md).

## Layout

Application code is under `src/` in an MVC layout. The PhysioNet WFDB C library is vendored under `src/ThirdParty/wfdb/` and is not mixed into the application modules.

```
src/
  main.cpp
  QCardio.pro
  Controllers/     AppController, SignalViewController
  Views/           MainWindow, SignalViewWidget
  Models/          ECG and CSV types, sheet analysis, settings
  Services/        WFDB, annotation, export, analyse, settings, log
  ThirdParty/wfdb/ PhysioNet WFDB 10.7.0 C sources
  tests/           Qt Test target tst_qcardio
```

| Layer | Responsibility |
|---|---|
| Views | User interface. Emits read, export, and compare requests. |
| Controllers | Connect views to services. `AppController` owns the main window and runs background jobs. |
| Services | File I/O and processing: `WfdbService`, `AnnotationService`, `ExportService`, `AnalyseService`, `SettingsService`, `LogService`. |
| Models | Data structures (`MIT_BIH_ECGData`, `AnalyseCfg`, and related types) and sheet and KPI helpers. |
| ThirdParty | Unmodified WFDB C library used by `WfdbService` and `AnnotationService`. |

Flow: view to `AppController` to service to model. Results return to the view as a plot, a log line, or a progress bar.

## Third-party software and data

- WFDB 10.7.0 is the PhysioNet WFDB Software Package, Copyright (C) 1983–2013 George B. Moody, under the GNU Library General Public License, version 2 or later. See `src/ThirdParty/wfdb/wfdb.h` and [PhysioNet WFDB](https://www.physionet.org/content/wfdb/10.7.0/).
- Windows builds link libcurl. The project file expects headers and libraries at `C:/curl-8.20.0_5`. A curl 8.20.0 tree is also present under `thirdparty/curl-8.20.0_5/`.
- MIT-BIH, AHA, and other PhysioNet databases are not included in this repository. Use them under the terms published with each database. MIT-BIH Arrhythmia Database: [physionet.org/content/mitdb/1.0.0](https://physionet.org/content/mitdb/1.0.0/).

This repository does not add a separate license file for the QCardio application sources.
