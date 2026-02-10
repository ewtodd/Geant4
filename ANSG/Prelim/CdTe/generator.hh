#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "Randomize.hh"
#include "TFile.h"
#include "TTree.h"

class PrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
  PrimaryGenerator();
  ~PrimaryGenerator();

  virtual void GeneratePrimaries(G4Event *);

private:
  G4ParticleGun *fParticleGun;
  void LoadStoredData();
  TFile *fRootFile;
  TTree *fTree;
  Long64_t fEntries;
  Double_t fpx, fpy, fpz; // momentum components
  Double_t fx, fy, fz;    // position components
  Double_t fZOffset;      // minimum z value for shifting
};

#endif
