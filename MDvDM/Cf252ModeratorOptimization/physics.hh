#ifndef PHYSICS_HH
#define PHYSICS_HH

#include "G4EmStandardPhysics.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4HadronPhysicsQGSP_BIC_HP.hh"
#include "G4VModularPhysicsList.hh"

class PhysicsList : public G4VModularPhysicsList {
public:
  PhysicsList();
  ~PhysicsList();
};

#endif
