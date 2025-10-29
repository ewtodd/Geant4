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
  G4double edep = step->GetTotalEnergyDeposit();
  G4double time = step->GetPreStepPoint()->GetGlobalTime();

  G4LogicalVolume *fScoringVolumeLaBr3 =
      detectorConstruction->GetScoringVolumeLaBr3();
  G4LogicalVolume *fScoringVolumeCeBr3 =
      detectorConstruction->GetScoringVolumeCeBr3();

  if (volume == fScoringVolumeLaBr3) {
    fEventAction->AddEdepLaBr3(edep, time);
  }
  if (volume == fScoringVolumeCeBr3) {
    fEventAction->AddEdepCeBr3(edep, time);
  }
}
