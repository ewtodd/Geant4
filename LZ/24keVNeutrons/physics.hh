#ifndef PHYSICS_HH
#define PHYSICS_HH

#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"
#include "G4IonPhysics.hh"
#include "G4OpticalPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4SystemOfUnits.hh"
#include "G4VModularPhysicsList.hh"

class PhysicsList : public G4VModularPhysicsList {
public:
  PhysicsList();
  ~PhysicsList();
};

#endif
