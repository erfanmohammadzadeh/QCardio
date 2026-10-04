# AHA to WFDB conversion

QCardio 1.0.0 reads WFDB records (`.hea`, `.dat`, `.atr`). The American Heart Association ECG database is distributed as `.CMP` and `.ANO` files. `scripts/convert_AHA2MIT.sh` runs the PhysioNet command-line converters and collects the WFDB files in one output directory.

The application already includes WFDB library sources under `src/ThirdParty/wfdb/` so it can read records. Those sources are not the `ad2m` and `a2m` programs. This script needs the WFDB **command-line** tools.

## Requirements

- A Unix shell (`bash`)
- WFDB Software Package 10.7.0 command-line tools, including `ad2m` (AHA signal to MIT format) and `a2m` (AHA annotation to MIT format)

Install instructions: [WFDB on PhysioNet](https://www.physionet.org/content/wfdb/10.7.0/).

Confirm both programs are on `PATH` before you run the script:

```sh
command -v ad2m
command -v a2m
```

## Configure paths

Open `convert_AHA2MIT.sh` and set:

| Variable | Meaning |
|---|---|
| `SRC` | Directory that contains the AHA `.CMP` and `.ANO` files |
| `OUT` | Directory where `.hea`, `.dat`, and `.atr` files are moved |

The copy in this repository contains example paths. Replace both of them. `OUT` may be a new folder; the script creates it.

## Run

From the `scripts` directory:

```sh
chmod +x ./convert_AHA2MIT.sh
./convert_AHA2MIT.sh
```

The script:

1. Runs `ad2m -i` on each `.CMP` file in `SRC`.
2. Runs `a2m -i` on each `.ANO` file in `SRC`.
3. Moves `*.atr`, `*.dat`, and `*.hea` from `SRC` into `OUT`.

Point QCardio at `OUT` on the Database tab. That folder is the database directory in the [user guide](../docs/user-guide.md). On the Export tab, choose the database name **AHA** so the source frequency is 250 Hz and the compare offset is 460.
