#include "detector.hh"

SensitiveDetector::SensitiveDetector(G4String name)
    : G4VSensitiveDetector(name) {}

SensitiveDetector::~SensitiveDetector() {}

G4bool SensitiveDetector::ProcessHits(G4Step *aStep,
                                      G4TouchableHistory *ROhist) {
  G4Track *track = aStep->GetTrack();

  // Check if this is a neutron for counting purposes
  if (track->GetDefinition()->GetParticleName() == "neutron") {
    G4double kineticEnergy = track->GetKineticEnergy();
    G4String detectorName = GetName();

    G4AnalysisManager *man = G4AnalysisManager::Instance();
    EventAction *eventAction =
        (EventAction *)G4RunManager::GetRunManager()->GetUserEventAction();

    // Fill appropriate detector ntuple with neutron energy
    if (detectorName == "CZT") {
      man->FillNtupleDColumn(0, 0, -100.0); // Energy dep placeholder
      man->FillNtupleIColumn(0, 1, 1);      // This neutron hit
      man->FillNtupleDColumn(0, 2,
                             kineticEnergy / keV); // Neutron kinetic energy
      man->AddNtupleRow(0);
      eventAction->IncrementNeutronCZT();
    } else if (detectorName == "HPGe") {
      man->FillNtupleDColumn(1, 0, -100.0); // Energy dep placeholder
      man->FillNtupleIColumn(1, 1, 1);      // This neutron hit
      man->FillNtupleDColumn(1, 2,
                             kineticEnergy / keV); // Neutron kinetic energy
      man->AddNtupleRow(1);
      eventAction->IncrementNeutronHPGe();
    }
  }

  return true;
}
