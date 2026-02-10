#include "physics.hh"

PhysicsList::PhysicsList() {
  SetVerboseLevel(0);
  RegisterPhysics(new G4EmStandardPhysics());
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4RadioactiveDecayPhysics());
  RegisterPhysics(new G4HadronPhysicsQGSP_BIC_AllHP);
}
PhysicsList::~PhysicsList() {}
