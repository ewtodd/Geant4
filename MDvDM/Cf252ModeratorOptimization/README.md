# Cf252ModeratorOptimization — Moderator Optimization for Cf-252 Neutrons

Samples neutrons from a **Watt fission spectrum** (a = 1.025 MeV, b = 2.926 MeV⁻¹ — realistic for Cf-252 spontaneous fission) and fires them at a polyethylene moderator slab (100 cm x 100 cm x 8 cm). Thin detection shells before and after the moderator record each neutron's kinetic energy, position, and momentum direction.

## What it studies
How well the polyethylene moderator thermalizes fission-spectrum neutrons.

## Physics
QGSP_BIC_HP hadronic physics with high-precision neutron elastic scattering, EM standard.

## Output
ROOT ntuples with "PreEnergy" and "PostEnergy" trees. PostEnergy includes kinetic energy, 3D position, and momentum direction. Runs 1M events on 32 threads.
