#include "physics.hh"

PhysicsList::PhysicsList() {
  // EM Physics
  RegisterPhysics(new G4EmStandardPhysics());

  // Decay Physics (includes radioactive decay)
  RegisterPhysics(new G4DecayPhysics());

  // Radioactive Decay Physics
  RegisterPhysics(new G4RadioactiveDecayPhysics());

  // Hadronic Physics - High Precision for neutrons
  RegisterPhysics(new G4HadronElasticPhysicsHP());
  RegisterPhysics(new G4HadronPhysicsFTFP_BERT_HP());

  // Ion Physics (for fission products)
  RegisterPhysics(new G4IonPhysics());

  // Stopping Physics (for particle capture)
  RegisterPhysics(new G4StoppingPhysics());

  // Optional: Optical Physics if you want scintillation
  // RegisterPhysics(new G4OpticalPhysics());
}

PhysicsList::~PhysicsList() {}
