#include "generator.hh"

PrimaryGenerator::PrimaryGenerator() {
  // Create two separate particle guns
  fNeutronGun = new G4ParticleGun(1);
  fGammaGun = new G4ParticleGun(1);

  // Set the fraction of events that are thermal neutrons
  fThermalNeutronFraction = 0.5; // 50% neutrons, 50% gammas

  // Setup neutron gun
  fNeutronGun->SetParticleDefinition(G4Neutron::Neutron());
  fNeutronGun->SetParticleEnergy(0.025 *
                                 eV); // Thermal energy at room temperature
  fNeutronGun->SetParticlePosition(G4ThreeVector(0, 0, 0)); // Origin
  fNeutronGun->SetParticleMomentumDirection(
      G4ThreeVector(0, 0, 1)); // Forward direction

  // Setup gamma gun
  fGammaGun->SetParticleDefinition(G4Gamma::Gamma());
  fGammaGun->SetParticleEnergy(2.223 * MeV); // Neutron capture gamma energy
  fGammaGun->SetParticlePosition(
      G4ThreeVector(0, 0, 5 * cm)); // 5cm along z-axis
}

PrimaryGenerator::~PrimaryGenerator() {
  delete fNeutronGun;
  delete fGammaGun;
}

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {
  if (G4UniformRand() < fThermalNeutronFraction) {
    // Generate thermal neutron at origin with forward direction
    fNeutronGun->GeneratePrimaryVertex(anEvent);
  } else {
    // Generate gamma at (0,0,5cm) with random direction
    G4ThreeVector randomDirection = G4RandomDirection();
    fGammaGun->SetParticleMomentumDirection(randomDirection);
    fGammaGun->GeneratePrimaryVertex(anEvent);
  }
}
