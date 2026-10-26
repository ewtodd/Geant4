#include "physics.hh"

#include "G4EmParameters.hh"

PhysicsList::PhysicsList() {
  SetVerboseLevel(0);

  // EM physics only: this build fires Pb characteristic X-rays into the M400
  // CZT to study the Cd/Te K X-ray escape peaks. No neutron/hadronic/decay
  // physics -- that lives in the Neutron-Activation build.
  RegisterPhysics(new G4EmStandardPhysics_option4());

  // Atomic de-excitation must be ON and NOT gated by the production cut, or the
  // Cd/Te Kalpha fluorescence (23-27 keV) is dumped locally and no escape peak
  // forms. The escape peak is photoELECTRIC fluorescence (fluo) competing with
  // Auger (auger) for the K-vacancy -- that fixes the fluorescence yield.
  //
  // PIXE is OFF on purpose: it models charged-particle-induced X-rays (the
  // photoelectron's track), which range only ~tens of um deep and are
  // reabsorbed locally -- negligible for escape. Worse, PIXE-on with the
  // Penelope ionisation model disables the standard de-excitation and routes it
  // through the PIXE statistical interface, which is not what we want here.
  G4EmParameters *em = G4EmParameters::Instance();
  em->SetFluo(true);
  em->SetAuger(true);
  em->SetPixe(false);
  em->SetDeexcitationIgnoreCut(true);
}

PhysicsList::~PhysicsList() {}
