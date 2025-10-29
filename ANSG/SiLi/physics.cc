#include "physics.hh"

PhysicsList::PhysicsList() {
  RegisterPhysics(new G4EmStandardPhysics());
  RegisterPhysics(new G4EmExtraPhysics());
  RegisterPhysics(new G4DecayPhysics());

  G4EmParameters *param = G4EmParameters::Instance();
  param->SetAugerCascade(true);
  param->SetDeexcitationIgnoreCut(true);
  param->SetFluo(true);
  param->SetAuger(true);
}
PhysicsList::~PhysicsList() {}
