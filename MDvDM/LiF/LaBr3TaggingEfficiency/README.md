# LaBr3TaggingEfficiency — Neutron Tagging with LiF + LaBr3

Shoots thermal neutrons (0.025 eV) at a 1 cm³ LiF crystal paired with a LaBr3(Ce) gamma detector (1.5" x 1.5" cylinder). Records energy and timing in both detectors.

## What it studies
The efficiency of tagging neutron capture events using the LiF scintillator in coincidence with the LaBr3(Ce) gamma detector.

## Physics
Hadron elastic (HP) + inelastic (QGSP_BIC_AllHP), EM standard (option 4), radioactive decay.

## Output
ROOT ntuples with energy deposition and timing for both detectors.

## Analysis
`macros/PhotopeakEfficiency.cpp` broadens the LaBr3 spectrum (2.9% FWHM @ 662 keV), fits the 2 MeV photopeak on a linear background, and prints the photopeak efficiency with its uncertainty.
