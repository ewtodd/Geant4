#include "generator.hh"

// The source is declared entirely in the macro (/gps/...): particle, energy,
// and the position of the Pb X-ray source relative to the CZT. See pb.mac.
PrimaryGenerator::PrimaryGenerator() { fGPS = new G4GeneralParticleSource(); }

PrimaryGenerator::~PrimaryGenerator() { delete fGPS; }

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {
  fGPS->GeneratePrimaryVertex(anEvent);
}
