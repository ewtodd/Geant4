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
  // Get step points to check boundary status
  G4StepPoint *preStepPoint = aStep->GetPreStepPoint();
  G4StepPoint *postStepPoint = aStep->GetPostStepPoint();

  if (fDetectorName == "AirShell") {
    if (postStepPoint->GetStepStatus() == fGeomBoundary) {

      if (particleName == "gamma") {
        man->FillNtupleDColumn(0, 0, kineticEnergy);
      } else {
        man->FillNtupleDColumn(0, 1, kineticEnergy);
      }
      man->AddNtupleRow(0);
    }
  }
  if (fDetectorName == "NaI" &&
      preStepPoint->GetStepStatus() == fGeomBoundary) {
    if (particleName == "gamma") {
      man->FillNtupleDColumn(2, 0, kineticEnergy);
    } else {
      man->FillNtupleDColumn(2, 1, kineticEnergy);
    }
    man->AddNtupleRow(2);
  }
  return true;
}
