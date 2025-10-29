#include "detector.hh"

SensitiveDetector::SensitiveDetector(G4String name)
    : G4VSensitiveDetector(name), fDetectorName(name) {}
SensitiveDetector::~SensitiveDetector() {}
G4bool SensitiveDetector::ProcessHits(G4Step *aStep,
                                      G4TouchableHistory *ROhist) {
  G4Track *track = aStep->GetTrack();
  G4String particleName = track->GetParticleDefinition()->GetParticleName();
  G4double kineticEnergy = track->GetKineticEnergy();
  G4AnalysisManager *man = G4AnalysisManager::Instance();
  // Storing neutron energies
  if (particleName == "neutron") {
    if (fDetectorName == "WrapPost") {

      // Retrieve position and momentum
      G4ThreeVector position = track->GetPosition();
      G4ThreeVector momentum = track->GetMomentumDirection();

      // Fill ntuple columns
      man->FillNtupleDColumn(1, 0, kineticEnergy);
      man->FillNtupleDColumn(1, 1, position.x() / cm);
      man->FillNtupleDColumn(1, 2, position.y() / cm);
      man->FillNtupleDColumn(1, 3, position.z() / cm);
      man->FillNtupleDColumn(1, 4, momentum.x());
      man->FillNtupleDColumn(1, 5, momentum.y());
      man->FillNtupleDColumn(1, 6, momentum.z());
      man->AddNtupleRow(1);
      man->AddNtupleRow(1);

    } else if (fDetectorName == "WrapPre") {
      man->FillNtupleDColumn(0, 0, kineticEnergy);
      man->AddNtupleRow(0); // Adding row for PreEnergy ntuple
    }
  }

  return true;
}
