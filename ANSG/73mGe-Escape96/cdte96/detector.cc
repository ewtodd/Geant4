#include "detector.hh"

SensitiveDetector::SensitiveDetector(G4String name)
    : G4VSensitiveDetector(name) {}
SensitiveDetector::~SensitiveDetector() {}

G4bool SensitiveDetector::ProcessHits(G4Step *aStep,
                                      G4TouchableHistory *ROhist) {
  return true;
}
