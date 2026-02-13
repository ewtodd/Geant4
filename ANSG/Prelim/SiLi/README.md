# SiLi — Gamma Response of Lithium-Drifted Silicon Detector

Simulates the response of a SiLi detector (5 mm radius, 5 mm thick) to ~68.75 keV gamma rays from thermal neutron activation of germanium.

## How it works
Gamma positions and momenta are read from `optimized.root` (from GeometryOptimizationHPGe). Energy is sampled from a Gaussian around 68.75 keV with SiLi-appropriate resolution (0.165 keV FWHM @ 5.9 keV reference).

## Physics
EM standard (option 4) with Auger and fluorescence.

## Output
ROOT ntuple with energy deposited per event.

