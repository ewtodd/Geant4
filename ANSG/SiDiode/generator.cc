#include "generator.hh"

PrimaryGenerator::PrimaryGenerator() {
  fParticleGun = new G4ParticleGun(1);
  G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName = "alpha";
  G4ParticleDefinition *particle = particleTable->FindParticle(particleName);
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticlePosition(G4ThreeVector(0, 0, 0));
}

PrimaryGenerator::~PrimaryGenerator() { delete fParticleGun; }

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {

  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, 1));

  G4double energy = 5486 * keV;

  fParticleGun->SetParticleEnergy(energy);

  fParticleGun->GeneratePrimaryVertex(anEvent);
}
