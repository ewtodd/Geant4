#include "physics.hh"

PhysicsList::PhysicsList() {

  RegisterPhysics(new G4EmStandardPhysics());
  RegisterPhysics(new G4HadronPhysicsQGSP_BIC_HP());
  RegisterPhysics(new G4HadronElasticPhysicsHP());
}
PhysicsList::~PhysicsList() {}
