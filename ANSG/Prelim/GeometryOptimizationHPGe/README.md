# GeometryOptimizationHPGe — Germanium Sample Geometry for Gamma Production

Fires neutrons (energy sampled from a thermal spectrum histogram) at a germanium sample with configurable geometry (thickness and width are constructor parameters). Records energy deposited in the Ge sample and in the surrounding air shell.

## What it studies
Optimizing the **Ge sample geometry** to maximize production of the gamma rays of interest (~68.75 keV) from thermal neutron activation. The output `optimized.root` with gamma positions and momenta is used as input for the CdTe, SiLi, and CZT detector simulations.

## Physics
High-precision hadron elastic and inelastic physics (QGSP_BIC_AllHP), EM standard (option 4), radioactive decay. Also uses Ge crystal lattice data.

## Output
ROOT ntuples recording energy deposition and particle position/momentum in both the Ge target and surrounding air. Runs 32 threads.

