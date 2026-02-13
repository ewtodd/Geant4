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

  G4LogicalVolume *fScoringVolumeGe =
      detectorConstruction->GetScoringVolumeGe();
  G4LogicalVolume *fScoringVolumeCdTe =
      detectorConstruction->GetScoringVolumeCdTe();
  G4LogicalVolume *fScoringVolumeNaI =
      detectorConstruction->GetScoringVolumeNaI();

  if (volume == fScoringVolumeGe) {
    fEventAction->AddEdepGe(edep, time);
  }
  if (volume == fScoringVolumeCdTe) {
    fEventAction->AddEdepCdTe(edep, time);
  }
  if (volume == fScoringVolumeNaI) {
    fEventAction->AddEdepNaI(edep, time);
  }
}
