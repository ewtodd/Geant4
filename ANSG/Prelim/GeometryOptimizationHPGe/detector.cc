#include "detector.hh"

SensitiveDetector::SensitiveDetector(G4String name)
    : G4VSensitiveDetector(name) {}
SensitiveDetector::~SensitiveDetector() {}

// Process hits for different detectors
G4bool SensitiveDetector::ProcessHits(G4Step *aStep,
                                      G4TouchableHistory *ROhist) {
  G4Track *track = aStep->GetTrack();
  const G4String &volumeName = track->GetVolume()->GetName();
  if (volumeName == "physGe") {
    // Handle the germanium volume hits
    if (track->GetParticleDefinition() == G4Gamma::Definition()) {
      G4double kineticEnergy = track->GetKineticEnergy() / keV;

      if (std::abs(kineticEnergy - 68.75) < 0.1) {
        G4ThreeVector momentumDirection = track->GetMomentumDirection();
        G4ThreeVector position = track->GetPosition();

        G4AnalysisManager *man = G4AnalysisManager::Instance();

        man->FillNtupleDColumn(0, 0, kineticEnergy);
        man->FillNtupleDColumn(0, 1, momentumDirection[0]);
        man->FillNtupleDColumn(0, 2, momentumDirection[1]);
        man->FillNtupleDColumn(0, 3, momentumDirection[2]);
        man->FillNtupleDColumn(0, 4, position[0] / cm);
        man->FillNtupleDColumn(0, 5, position[1] / cm);
        man->FillNtupleDColumn(0, 6, position[2] / cm);
        man->AddNtupleRow(0);
        man->FillNtupleIColumn(1, 0, 1); // Identifier for detected gamma
        man->AddNtupleRow(1);
      }
    }
  } else if (volumeName == "physAirLayer") {
    if (track->GetParticleDefinition() == G4Gamma::Definition()) {
      G4double kineticEnergy = track->GetKineticEnergy() / keV;
      if (std::abs(kineticEnergy - 68.75) < 0.1) {
        G4AnalysisManager *man = G4AnalysisManager::Instance();
        man->FillNtupleIColumn(2, 0, 1); // Marked as escaped
        man->AddNtupleRow(2);
      }
    }
  }

  return true;
}
