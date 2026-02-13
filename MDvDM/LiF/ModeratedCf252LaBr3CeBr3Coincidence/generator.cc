#include "generator.hh"

PrimaryGenerator::PrimaryGenerator() {
  G4int numParticles = 1;
  fParticleGun = new G4ParticleGun(numParticles);
  fParticleGun->SetParticleDefinition(G4Neutron::Neutron());
}

PrimaryGenerator::~PrimaryGenerator() { delete fParticleGun; }

G4double PrimaryGenerator::SampleCf252Spectrum() {
  // Watt spectrum parameters for Cf-252
  const G4double a = 1.025; // MeV
  const G4double b = 2.926; // MeV^-1
  const G4double maxP = 1.04;

  G4double E, P;
  do {
    E = 15.0 * MeV * G4UniformRand(); // Reduced upper limit
    P = std::exp(-E / a) * std::sinh(std::sqrt(b * E));
  } while (G4UniformRand() > P / maxP); // Better efficiency

  return E;
}

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {
  fParticleGun->SetParticlePosition(G4ThreeVector(0, 0, 0));
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, 1));
  //  Sample energy from Cf-252 spectrum
  G4double energy = SampleCf252Spectrum();
  fParticleGun->SetParticleEnergy(energy);

  fParticleGun->GeneratePrimaryVertex(anEvent);
}
