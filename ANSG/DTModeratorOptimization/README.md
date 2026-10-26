# DTModeratorOptimization — Polyethylene Moderator Thickness for DT Neutrons

Fires 14.1 MeV neutrons (the energy from deuterium-tritium fusion) into a polyethylene moderator slab (100 cm x 100 cm, variable thickness) and measures neutron energy before and after using thin detection shells on each side.

## What it studies
Finding the **optimal polyethylene thickness** to thermalize DT fusion neutrons down to thermal energies.

## Physics
QGSP_BIC_HP for hadronic interactions, high-precision neutron elastic scattering, EM standard physics.

## Output
ROOT ntuples with pre- and post-moderation neutron energies. Runs 32 threads.

## Analysis
`macros/AnalyzeAndPlotSpectra.cpp` counts the thermal neutrons behind 10 to 17 cm of water and polyethylene, plots the yield against thickness and the thermal spectra, and writes the normalized water spectrum to `root_files/thermalEnergySpectra.root`, the source GeometryOptimizationHPGe samples.
