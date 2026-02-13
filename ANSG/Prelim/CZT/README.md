# CZT — Gamma Response of Cadmium-Zinc-Telluride Detector

Simulates the response of a CZT detector to ~68.75 keV gamma rays from thermal neutron activation of germanium.

## How it works
Gamma positions and momenta are read from `optimized.root` (from GeometryOptimizationHPGe). Energy is sampled from a Gaussian around 68.75 keV with CZT-appropriate resolution (1.8 keV FWHM @ 59.5 keV reference).

## Physics
EM standard (option 4) with Auger and fluorescence.

## Output
ROOT ntuple with energy deposited per event.

