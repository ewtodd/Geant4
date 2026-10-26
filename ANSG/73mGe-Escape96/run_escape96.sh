#!/usr/bin/env bash
# The 95.902 keV 113Cd(n,g) line's K X-ray escapes, simulated in the OSU CZT
# array (czt96, four 20.75 x 20.75 x 10 mm crystals) and in the PSI Amptek
# CdTe (cdte96, a 25 mm2 x 1 mm disk). Both are the Post-OSU-Escape-Peaks
# application with their own geometry and generator.cc:
#   surface         captures within a mean free path of every surface, with
#                   the transition's internal conversion (Cd K X-rays);
#   surface_noconv  the same without internal conversion;
#   beam            an external 95.902 keV photon beam onto the crystal.
# Each run is 20 M primaries on 32 threads. The sim merges the per-thread
# files at the end of the run into one output0.root (one ntuple per crystal,
# energy deposit in keV) in ~/UM-ANSG/73mGe/root_files/geant4/<geometry>_<run>,
# or in $ESC96_OUT/<geometry>_<run> when ESC96_OUT is set.
# Builds and runs in this repo's 73mGe-Escape96 shell; plots with
# Escape96Plots.cpp in UM-ANSG's 73mGe shell.
#
# Usage: run_escape96.sh [geometry_run ...]   (default: all five)
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(git -C "$here" rev-parse --show-toplevel)"
out_root="${ESC96_OUT:-$HOME/UM-ANSG/73mGe/root_files/geant4}"

runs=("$@")
if [ ${#runs[@]} -eq 0 ]; then
  runs=(cdte96_surface cdte96_surface_noconv cdte96_beam czt96_surface czt96_beam)
fi

for run in "${runs[@]}"; do
  geometry="${run%%_*}"
  mode="${run#*_}"
  conversion=1
  if [ "$mode" = "surface_noconv" ]; then
    mode=surface
    conversion=0
  fi
  mfp=0.25
  if [ "$geometry" = "cdte96" ]; then
    mfp=0.22
  fi
  mkdir -p "$out_root/$run"
  cp "$here/$geometry/run96.mac" "$out_root/$run/"
  (cd "$repo_root" && nix develop ".#73mGe-Escape96" --command bash -c "
    set -e
    mkdir -p '$here/$geometry/build'
    cd '$here/$geometry/build'
    [ -f Makefile ] || cmake .. > cmake.log 2>&1
    make -j16 > make.log 2>&1
    cd '$out_root/$run'
    env -u DISPLAY ESC96_MODE=$mode ESC96_MFP_MM=$mfp ESC96_CONVERSION=$conversion \
      '$here/$geometry/build/sim' run96.mac > sim.log 2>&1
  ")
  if [ -f "$out_root/$run/output0.root" ] && ! compgen -G "$out_root/$run/output0_t*.root" > /dev/null; then
    echo "$run done: $out_root/$run/output0.root"
  else
    echo "$run: per-thread files not merged; see $out_root/$run/sim.log"
  fi
done
