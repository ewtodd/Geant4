
#include "generator.hh"

PrimaryGenerator::PrimaryGenerator() {
  fParticleGun = new G4ParticleGun(1);
  fParticleGun->SetParticlePosition(G4ThreeVector(0, 0, 0));
  // Don't set particle definition here
}

PrimaryGenerator::~PrimaryGenerator() { delete fParticleGun; }

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {
  // Define ion here instead
  G4int Z = 17, A = 37;
  G4double ionCharge = 17. * eplus;
  G4double excitEnergy = 0. * keV;

  G4ParticleDefinition *ion =
      G4IonTable::GetIonTable()->GetIon(Z, A, excitEnergy);

  fParticleGun->SetParticleDefinition(ion);
  fParticleGun->SetParticleCharge(ionCharge);
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, 1));

  G4double energy = 92.0 * MeV;
  fParticleGun->SetParticleEnergy(energy);

  fParticleGun->GeneratePrimaryVertex(anEvent);
}
