#include "generator.hh"
#include <G4AutoLock.hh>
#include <G4Event.hh>
#include <G4Neutron.hh>
#include <G4SystemOfUnits.hh>
#include <TFile.h>
#include <TH1D.h>
#include <TRandom3.h>
#include <TTree.h>
#include <mutex>
#include <unistd.h>

// Initialize the static mutex
G4Mutex PrimaryGenerator::rootMutex = G4MUTEX_INITIALIZER;

PrimaryGenerator::PrimaryGenerator()
    : G4VUserPrimaryGeneratorAction(), fParticleGun(nullptr),
      fEnergyHist(nullptr), fileEnergy(nullptr), fRand(nullptr) {
  G4int numParticles = 1;
  fParticleGun = new G4ParticleGun(numParticles);
  fParticleGun->SetParticleDefinition(G4Neutron::Neutron());

  // Initialize the random number generator
  fRand = new TRandom3();

  // ROOT operations must be thread-safe
  {
    G4AutoLock lock(&rootMutex);

    // Open the energy histogram ROOT file
    fileEnergy = TFile::Open("thermalEnergySpectra.root", "READ");
    if (!fileEnergy || fileEnergy->IsZombie()) {
      G4cerr << "Error opening ROOT file thermalEnergySpectra.root!" << G4endl;
      fileEnergy = nullptr;
      fEnergyHist = nullptr;
      throw std::runtime_error("Failed to open the energy file");
    }

    // Get the histogram for energy
    fEnergyHist =
        dynamic_cast<TH1D *>(fileEnergy->Get("ThermalEnergyHistWater"));
    if (!fEnergyHist) {
      G4cerr << "Error retrieving energy histogram from ROOT file!" << G4endl;
      fileEnergy->Close();
      fileEnergy = nullptr;
      throw std::runtime_error("Failed to get the energy histogram");
    }
  }
}

PrimaryGenerator::~PrimaryGenerator() {
  delete fParticleGun;
  delete fRand;

  if (fileEnergy) {
    G4AutoLock lock(&rootMutex);
    fileEnergy->Close();
    delete fileEnergy;
  }
}

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {

  // Set the particle's position from the data
  fParticleGun->SetParticlePosition(G4ThreeVector(0, 0, 0));

  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, 1));

  // Sample energy from the histogram
  double energy = 0.025 * eV; // Default energy in case histogram not available
  if (fEnergyHist) {
    G4AutoLock lock(&rootMutex);
    energy = fEnergyHist->GetRandom() * eV;
  }

  fParticleGun->SetParticleEnergy(energy);

  fParticleGun->GeneratePrimaryVertex(anEvent);
}
