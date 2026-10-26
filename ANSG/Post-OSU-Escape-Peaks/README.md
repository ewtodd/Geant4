# Post-OSU-Escape-Peaks — Cd/Te K X-ray Escape Peaks in CZT

Fires Pb K X-rays into one crystal of the H3D M400 CZT array (four 20.75 mm x 20.75 mm x 10 mm crystals) and records the energy deposited in each crystal.

## What it studies
The Cd and Te K X-ray **escape peaks**: when a photoelectric absorption's K X-ray leaves the crystal, the deposit is the photon energy minus the X-ray's. For Pb K-alpha2 (74.97 keV) the Cd K-alpha escape sits at 51.80 keV and the Te K-alpha escape at 47.50 keV.

## How it works
The source is a G4GeneralParticleSource defined entirely in the macro. `pb.mac` fires a 74.9694 keV pencil beam straight down into crystal CZT_3 from 3 mm above its top face, 20M events on 32 threads.

## Physics
EM standard (option 4) only, with fluorescence and Auger forced on, PIXE off, and de-excitation not gated by the production cut. Without that, the Cd/Te K-alpha is deposited locally and no escape peak forms.

## Output
One ROOT ntuple per crystal (`CZT_0` to `CZT_3`) with the energy deposited per event (keV).

## Analysis
`macros/SimHistogram.cpp` adds per-crystal and combined histograms to a run's file. `macros/EnergyDeposition.cpp` plots the four crystals' deposits from 50 to 110 keV with the ¹¹³Cd(n,γ) lines marked, raw and broadened.
