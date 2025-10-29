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

  // Sampling using rejection method
  G4double E, P, R;
  do {
    // Sample in range 0-20 MeV (covers most of spectrum)
    E = 20.0 * G4UniformRand() * MeV;
    // Watt spectrum: N(E) ~ exp(-E/a) * sinh(sqrt(b*E))
    P = std::exp(-E / a) * std::sinh(std::sqrt(b * E));
    R = G4UniformRand();
  } while (R > P / 4.0);

  return E;
}

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {
  fParticleGun->SetParticlePosition(G4ThreeVector(0, 0, 0));
  fParticleGun->SetParticleMomentumDirection(G4RandomDirection());
  // Sample energy from Cf-252 spectrum
  G4double energy = SampleCf252Spectrum();
  fParticleGun->SetParticleEnergy(energy);

  fParticleGun->GeneratePrimaryVertex(anEvent);
}
