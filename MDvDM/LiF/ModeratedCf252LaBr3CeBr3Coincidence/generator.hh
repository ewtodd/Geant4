#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4Gamma.hh"
#include "G4Neutron.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4RandomDirection.hh"
#include "G4SystemOfUnits.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "Randomize.hh"

class PrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
  PrimaryGenerator();
  ~PrimaryGenerator();

  virtual void GeneratePrimaries(G4Event *anEvent);

private:
  G4ParticleGun *fParticleGun;

  G4double SampleCf252Spectrum(); // Keep for potential future use
};

#endif
