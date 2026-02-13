#include "physics.hh"

PhysicsList::PhysicsList() {
  RegisterPhysics(new G4EmStandardPhysics());

  RegisterPhysics(new G4DecayPhysics());

  RegisterPhysics(new G4RadioactiveDecayPhysics());

  RegisterPhysics(new G4HadronElasticPhysicsHP());
  RegisterPhysics(new G4HadronPhysicsFTFP_BERT_HP());

  RegisterPhysics(new G4IonPhysics());

  RegisterPhysics(new G4StoppingPhysics());

  RegisterPhysics(new G4OpticalPhysics());
}

PhysicsList::~PhysicsList() {}
