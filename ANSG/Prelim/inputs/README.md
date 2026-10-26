# Prelim inputs

Files the Prelim generators read from their working directory. Each
project's CMake copies the one it needs into its `build/`, and copies it
again whenever it changes here.

| File | Read by | Made by |
|---|---|---|
| `optimized.root` | `CZT`, `CdTe`, `SiLi`: the positions and momenta of the gammas leaving the Ge sample (tree `Energy`) | a `GeometryOptimizationHPGe` run of the optimized sample geometry |
| `thermalEnergySpectra.root` | `GeometryOptimizationHPGe`: the thermal neutron spectrum it samples (`ThermalEnergyHistWater`) | `ANSG/DTModeratorOptimization/macros/AnalyzeAndPlotSpectra.cpp`, which writes it to that project's `root_files/` |

To update either, replace the file here and rebuild the projects that read it.
