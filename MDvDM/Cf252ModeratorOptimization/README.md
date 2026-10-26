# Cf252ModeratorOptimization — Moderator Optimization for Cf-252 Neutrons

Samples neutrons from a **Watt fission spectrum** (a = 1.025 MeV, b = 2.926 MeV⁻¹ — realistic for Cf-252 spontaneous fission) and fires them at a polyethylene moderator slab (100 cm x 100 cm, 8 cm thick by default). Thin detection shells before and after the moderator record each neutron's kinetic energy, position, and momentum direction.

## What it studies
How well the polyethylene moderator thermalizes fission-spectrum neutrons.

## Physics
QGSP_BIC_HP hadronic physics with high-precision neutron elastic scattering, EM standard.

## Output
ROOT ntuples with "PreEnergy" and "PostEnergy" trees, one row per neutron crossing each shell. PostEnergy includes kinetic energy, 3D position, and momentum direction. Each run is written to `Poly<t>cm.root`, named after the moderator thickness. `iso.mac` runs 1M events on 32 threads at 8 cm.

## Thickness scan
`/moderator/thickness <value> cm` sets the slab thickness and rebuilds the geometry before the next run. `scan.mac` runs 1M events at each thickness from 5 to 12 cm, leaving `Poly5cm.root` to `Poly12cm.root`; `macros/ThermalNeutrons.cpp` plots the thermal-neutron yield against thickness from them.

## Analysis
`macros/ThermalNeutrons.cpp` plots the thermal-neutron yield against moderator thickness and the thermal spectrum at 8 cm; `macros/WattSpectrum.cpp` draws the Cf-252 Watt spectrum.
