#include "physics.hh"

#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4GenericIon.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"
#include "G4IonPhysics.hh"
#include "G4NuclideTable.hh"
#include "G4PhysicsListHelper.hh"
#include "G4Radioactivation.hh"
#include "G4StoppingPhysics.hh"
#include "G4SystemOfUnits.hh"

PhysicsList::PhysicsList() {
  G4NuclideTable::GetInstance()->SetMeanLifeThreshold(1.0 * nanosecond);

  RegisterPhysics(new G4EmStandardPhysics());
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4HadronElasticPhysicsHP());
  RegisterPhysics(new G4HadronPhysicsFTFP_BERT_HP());
  RegisterPhysics(new G4IonPhysics());
  RegisterPhysics(new G4StoppingPhysics());
}

PhysicsList::~PhysicsList() {}

void PhysicsList::ConstructParticle() {
  G4VModularPhysicsList::ConstructParticle();
}

void PhysicsList::ConstructProcess() {
  G4VModularPhysicsList::ConstructProcess();

  G4Radioactivation *radioactiveDecay = new G4Radioactivation();
  G4PhysicsListHelper *ph = G4PhysicsListHelper::GetPhysicsListHelper();
  ph->RegisterProcess(radioactiveDecay, G4GenericIon::GenericIon());
}
