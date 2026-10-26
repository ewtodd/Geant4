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

  G4LogicalVolume *fScoringVolumeCZT =
      detectorConstruction->GetScoringVolumeCZT();

  if (volume == fScoringVolumeCZT) {
    G4int copyNo = step->GetPreStepPoint()
                       ->GetTouchableHandle()
                       ->GetCopyNumber();
    G4double edep = step->GetTotalEnergyDeposit();
    fEventAction->AddEdepCZT(copyNo, edep);
  }
}
