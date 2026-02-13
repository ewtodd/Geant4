#include "generator.hh"

PrimaryGenerator::PrimaryGenerator() {
  G4int numParticles = 1;
  fParticleGun = new G4ParticleGun(numParticles);

  fParticleGun->SetParticleEnergy(1.0 * MeV);
  fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., 0.));
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
}

PrimaryGenerator::~PrimaryGenerator() { delete fParticleGun; }

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {
  G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
  G4IonTable *ionTable = particleTable->GetIonTable();

  G4ParticleDefinition *cf252 = ionTable->GetIon(98, 252, 0.0);

  fParticleGun->SetParticleDefinition(cf252);
  fParticleGun->GeneratePrimaryVertex(anEvent);
}
