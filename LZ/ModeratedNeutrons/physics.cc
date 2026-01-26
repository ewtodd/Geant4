#include "physics.hh"

PhysicsList::PhysicsList() {
  SetVerboseLevel(0);

  // Elastic collisions
  RegisterPhysics(new G4HadronElasticPhysicsHP());

  // Inelastic physics
  RegisterPhysics(new G4HadronPhysicsQGSP_BIC_AllHP());

  // EM physics
  RegisterPhysics(new G4EmStandardPhysics_option4());

  // Radioactive decay
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4RadioactiveDecayPhysics());
}

PhysicsList::~PhysicsList() {}
