#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4Neutron.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4RandomDirection.hh"
#include "G4SystemOfUnits.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include <G4AutoLock.hh>
#include <G4Threading.hh>
#include <TFile.h>
#include <TH1D.h>
#include <TRandom3.h>
#include <TTree.h>
#include <vector>

class PrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
  PrimaryGenerator();
  ~PrimaryGenerator();

  virtual void GeneratePrimaries(G4Event *anEvent);

private:
  G4ParticleGun *fParticleGun;
  TH1D *fEnergyHist;
  TFile *fileEnergy;

  TRandom3 *fRand;

  Double_t fPostEnergy;

  static G4Mutex rootMutex;
};

#endif
