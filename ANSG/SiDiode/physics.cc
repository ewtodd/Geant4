#include "physics.hh"

PhysicsList::PhysicsList() {
  RegisterPhysics(new G4EmStandardPhysics_option4());
  RegisterPhysics(new G4EmExtraPhysics());
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4IonPhysics());

  G4EmParameters *param = G4EmParameters::Instance();
  param->SetAugerCascade(true);
  param->SetDeexcitationIgnoreCut(true);
  param->SetFluo(true);
  param->SetAuger(true);
}
PhysicsList::~PhysicsList() {}
