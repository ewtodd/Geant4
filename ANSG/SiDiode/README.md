# SiDiode — Alpha Particle Energy Deposition in Silicon

Simulates 5.486 MeV alpha particles hitting a silicon diode detector (modeled after a Canberra partially-depleted detector: 11.968 mm radius, 300 um thick). The detector is rotated 60 degrees and placed 2.54 cm from the source in air.

## What it studies
The effects of **air** (energy loss before reaching the detector) and **angle of incidence** on the shape and position of the energy deposition peak.

## Physics
EM standard (option 4) with Auger cascade and fluorescence enabled.

## Output
ROOT ntuple with energy deposited per event (keV). Analysis script applies Gaussian broadening (16 keV FWHM @ 5486 keV) to simulate detector resolution.

**No flake.nix yet.**
