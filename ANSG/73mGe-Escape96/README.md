# 73mGe-Escape96

Cd K X-ray escape from the 95.902 keV ¹¹³Cd(n,γ) line, for the 73mGe
analysis in [UM-ANSG](https://github.com/ewtodd/UM-ANSG) (`73mGe/`). Both
applications are Post-OSU-Escape-Peaks with their own geometry and
`generator.cc`:

- `czt96/`: the OSU M400 CZT array, four 20.75 × 20.75 × 10 mm crystals.
- `cdte96/`: the PSI Amptek CdTe, a 25 mm² × 1 mm disk.

The generator reads three environment variables:

| Variable | Meaning |
|---|---|
| `ESC96_MODE` | `surface`: captures within a mean free path of every surface; `beam`: an external 95.902 keV photon beam |
| `ESC96_MFP_MM` | capture mean free path in mm (0.25 for CZT, 0.22 for CdTe) |
| `ESC96_CONVERSION` | `1` emits the transition's internal conversion (Cd K X-rays), `0` does not |

## Running

```bash
nix develop .#73mGe-Escape96          # from the repo root
ANSG/73mGe-Escape96/run_escape96.sh   # all five runs, or name some: czt96_beam ...
```

Each run is 20 M primaries on 32 threads (`run96.mac`; `test96.mac` is a
20 k-event check on 4 threads). The per-thread files are merged at the end of
the run into one `output0.root` per run, written to
`~/UM-ANSG/73mGe/root_files/geant4/<geometry>_<run>/` (override with
`ESC96_OUT`). Plot them with `geant4/Escape96Plots.cpp` in UM-ANSG's 73mGe
shell.
