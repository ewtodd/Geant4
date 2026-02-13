# Coincidence — Thermal Neutron Coincidence Detection
<!---->
Simulates thermal neutrons (0.025 eV) hitting a multi-detector system and records energy deposition and timing in each detector for coincidence analysis.
<!---->
## Detector Setup
- **Germanium sample**: 1 inch diameter, 2 mm thick disc — energy deposition is also tracked here, but is not actually a detector.
- **CdTe detector**: 2.82 mm radius, 1 mm thick — offset below beam, surrounded by 4 mm lead shielding.
- **Aluminum layer**: 5 mm thick, 1 inch diameter — between Ge and NaI.
- **NaI scintillator**: 3" x 3" cylinder — coincidence detector for tagging 5.8 MeV gamma from Ge.
<!---->
World volume is air (40 x 40 x 60 cm).
<!---->
## Physics
Thermal neutron support, HP hadron elastic + inelastic (QGSP_BIC_AllHP), EM standard (option 4), radioactive decay.
<!---->
## Output
Three ROOT ntuples (Ge, CdTe, NaI) with energy (MeV) and time (ns) per event.
Runs 1M events on 12 threads.
Analysis includes coincidence patterns and resolution-broadened energy spectra.
