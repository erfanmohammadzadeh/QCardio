# Build and test

QCardio 1.0.0 is a Qt Widgets application. The project file is [`src/QCardio.pro`](../src/QCardio.pro).

## Requirements

- Qt 6.8 with the Widgets and Concurrent modules
- A C++17 compiler. The kit used for this release is MinGW 64-bit with Qt 6.8.2
- libcurl
  - Windows: headers and import library at `C:/curl-8.20.0_5` (`include` and `lib`), as set in `QCardio.pro`
  - Unix: `libcurl` visible to `pkg-config`

`qmake` must be able to run the compiler. On Windows, a normal PowerShell session does not have MinGW on `PATH`.

## Build the application (Windows)

From a shell where `qmake` and `mingw32-make` are on `PATH` (Qt Creator’s Desktop Qt 6.8.2 MinGW 64-bit kit, or the paths below):

```powershell
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.8.2\mingw_64\bin;" + $env:PATH
cd src
qmake QCardio.pro -spec win32-g++ "CONFIG+=release"
mingw32-make
```

The executable is written to `bin/` under the build directory (`DESTDIR` in the project file). You can also open `src/QCardio.pro` in Qt Creator and build with the same kit.

Release builds open full screen. Debug builds open in a normal window.

## Build on Unix

`QCardio.pro` links libcurl with `pkg-config` on Unix. From `src`:

```sh
qmake QCardio.pro
make
```

Install the result with `make install` when the generated install rule applies. The project file uses `/opt/QCardio/bin` for non-Android Unix installs.

## Automated tests

Tests live in `src/tests/` and produce `tst_qcardio`. They cover directory validation, CSV I/O, beat sensitivity and precision, compare report files, raw-sample export, and the log service. They do not read WFDB records.

### Windows script

From `src/tests`:

```powershell
.\run_tests.ps1
```

The script adds the Qt 6.8.2 MinGW kit to `PATH`, then builds and runs the tests. If Qt is not under `C:\Qt\6.8.2` and `C:\Qt\Tools\mingw1310_64`, set:

```powershell
$env:QT_MINGW_BIN = "C:\Qt\6.8.2\mingw_64\bin"
$env:MINGW_BIN    = "C:\Qt\Tools\mingw1310_64\bin"
.\run_tests.ps1
```

### Manual Windows build

```powershell
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.8.2\mingw_64\bin;" + $env:PATH
cd src\tests
qmake tests.pro -spec win32-g++ "CONFIG+=debug"
mingw32-make
.\debug\tst_qcardio.exe
```

You can also open `src/tests/tests.pro` in Qt Creator and run it with the Desktop Qt 6.8.2 MinGW 64-bit kit.

If `qmake` cannot find the compiler, the message is:

`Project ERROR: Cannot run compiler 'g++'. Maybe you forgot to setup the environment?`
