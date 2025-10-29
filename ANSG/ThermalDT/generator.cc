#include "generator.hh"

PrimaryGenerator::PrimaryGenerator() {
  G4int numParticles = 1;
  fParticleGun = new G4ParticleGun(numParticles);
  fParticleGun->SetParticleDefinition(G4Neutron::Neutron());
}

PrimaryGenerator::~PrimaryGenerator() { delete fParticleGun; }

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {
  fParticleGun->SetParticlePosition(G4ThreeVector(0, 0, 0));
  fParticleGun->SetParticleMomentumDirection(G4RandomDirection());
  fParticleGun->SetParticleEnergy(14.1 * MeV); /// constant energy
  fParticleGun->GeneratePrimaryVertex(anEvent);
}
