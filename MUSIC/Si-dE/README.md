# Si-dE — Heavy Ion Energy Loss in Silicon

Simulates **92 MeV fully-stripped Chlorine-37 ions** passing through a thin silicon detector stack modeled after an ORTEC TD-030-150-50 diode. Measures differential energy loss (dE/dx) for heavy ion identification.

## Detector Stack (beam order)
1. **Ti entrance window** (~1 um, 0.9 mg/cm²)
2. **Ti exit window** (~1.4 um, 1.3 mg/cm²)
3. **Si front dead layer** (100 nm) — not recorded
4. **Si active region** (~24.7 um) — energy deposition recorded here
5. **Si rear dead layer** (200 nm) — not recorded

Total Si thickness: 50 um. The detector sits in galactic vacuum (no air).

## Physics
EM standard (option 4) with Auger cascade, fluorescence, and ion physics. No hadronic physics needed at this beam energy.

## Output
ROOT ntuple with energy deposited (MeV) in the active region per event. Runs 100k events on 32 threads. Analysis scripts merge outputs and optionally apply Gaussian resolution broadening.

