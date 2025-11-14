#ifndef PHYSICS_HH
#define PHYSICS_HH

#include "G4DecayPhysics.hh"
#include "G4EmExtraPhysics.hh"
#include "G4EmParameters.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4IonPhysics.hh"
#include "G4VModularPhysicsList.hh"

class PhysicsList : public G4VModularPhysicsList {
public:
  PhysicsList();
  ~PhysicsList();
};

#endif
