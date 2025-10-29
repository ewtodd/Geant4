#include "stepping.hh"

SteppingAction::SteppingAction(EventAction *eventAction) {
  fEventAction = eventAction;
}

SteppingAction::~SteppingAction() {}

void SteppingAction::UserSteppingAction(const G4Step *step) {
  G4LogicalVolume *volume = step->GetPreStepPoint()
                                ->GetTouchableHandle()
                                ->GetVolume()
                                ->GetLogicalVolume();

  const DetectorConstruction *detectorConstruction =
      static_cast<const DetectorConstruction *>(
          G4RunManager::GetRunManager()->GetUserDetectorConstruction());

  G4LogicalVolume *fScoringVolumeCZT = detectorConstruction->GetScoringVolume();
  G4LogicalVolume *fScoringVolumeHPGe = detectorConstruction->GetHPGeVolume();

  G4double edep = step->GetTotalEnergyDeposit();

  // Route energy deposition to appropriate detector
  if (volume == fScoringVolumeCZT) {
    fEventAction->AddEdepCZT(edep);
  } else if (volume == fScoringVolumeHPGe) {
    fEventAction->AddEdepHPGe(edep);
  }
}
