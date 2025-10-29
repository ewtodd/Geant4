#include "generator.hh"

PrimaryGenerator::PrimaryGenerator() {
  fParticleGun = new G4ParticleGun(1);
  G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName = "gamma";
  G4ParticleDefinition *particle = particleTable->FindParticle(particleName);
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticlePosition(G4ThreeVector(0, 0, 0));
  LoadStoredData();
}

PrimaryGenerator::~PrimaryGenerator() {
  delete fParticleGun;
  if (fRootFile) {
    fRootFile->Close();
    delete fRootFile;
  }
}

void PrimaryGenerator::LoadStoredData() {
  fRootFile = new TFile("optimized.root", "READ");
  if (!fRootFile || fRootFile->IsZombie()) {
    G4Exception("PrimaryGenerator::LoadStoredData", "FileNotFound",
                FatalException, "Cannot open optimized.root file");
  }

  fTree = (TTree *)fRootFile->Get("Energy");
  if (!fTree) {
    G4Exception("PrimaryGenerator::LoadStoredData", "TreeNotFound",
                FatalException, "Cannot find 'Energy' tree in the ROOT file");
  }

  // Set branch addresses for both momentum and position
  fTree->SetBranchAddress("fpx", &fpx);
  fTree->SetBranchAddress("fpy", &fpy);
  fTree->SetBranchAddress("fpz", &fpz);
  fTree->SetBranchAddress("fx", &fx);
  fTree->SetBranchAddress("fy", &fy);
  fTree->SetBranchAddress("fz", &fz);

  fEntries = fTree->GetEntries();
  if (fEntries == 0) {
    G4Exception("PrimaryGenerator::LoadStoredData", "NoEntries", FatalException,
                "No entries found in the 'Energy' tree");
  }

  // Find minimum z position to shift coordinates
  Double_t zMin = DBL_MAX;
  for (Long64_t i = 0; i < fEntries; i++) {
    fTree->GetEntry(i);
    if (fz < zMin)
      zMin = fz;
  }
  fZOffset = zMin;
}

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {
  Long64_t entry = G4UniformRand() * fEntries;
  fTree->GetEntry(entry);

  // Set position, shifting z to start at origin
  G4ThreeVector position(fz - fZOffset, fy, fx);

 // Bias the z-component (originally x) towards one direction
    if (position.z() < 0) {
        position.setZ(-position.z());
    }
    if (position.y() < 0) {
        position.setY(-position.y());
    }

  fParticleGun->SetParticlePosition(position);

  // Set momentum direction
  G4ThreeVector momentum(fpz, fpy, fpx);
 // Bias the z-component (originally x) towards one direction
    if (momentum.z() < 0) {
        momentum.setZ(-momentum.z());
    }    
  fParticleGun->SetParticleMomentumDirection(momentum.unit());

    // Mean energy = 68.75 keV
    // Resolution = 1.8/59.5 ≈ 0.03025
    // Sigma = mean_energy * resolution
  G4double mean_energy = 68.752 * keV;
  G4double resolution = 1.8 / 59.5;
  G4double sigma = mean_energy * resolution;

  // Sample from Gaussian distribution
  G4double energy;
  do {
    energy = G4RandGauss::shoot(mean_energy, sigma);
  } while (energy <= 0); // Ensure positive energy

  fParticleGun->SetParticleEnergy(energy);

  fParticleGun->GeneratePrimaryVertex(anEvent);
}
