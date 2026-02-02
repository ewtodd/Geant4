#include "stepping.hh"

SteppingAction::SteppingAction(EventAction *eventAction) {
  fEventAction = eventAction;
}

SteppingAction::~SteppingAction() {}

void SteppingAction::UserSteppingAction(const G4Step *step) {
  // Get the logical volume of current step
  G4LogicalVolume *volume = step->GetPreStepPoint()
                                ->GetTouchableHandle()
                                ->GetVolume()
                                ->GetLogicalVolume();

  // Get detector construction to access scoring volumes
  const DetectorConstruction *detectorConstruction =
      static_cast<const DetectorConstruction *>(
          G4RunManager::GetRunManager()->GetUserDetectorConstruction());

  // Get energy deposit and time
  G4double edep = step->GetTotalEnergyDeposit();
  G4double time = step->GetPreStepPoint()->GetGlobalTime();

  // Skip if no energy deposited
  if (edep <= 0.)
    return;

  // Get the particle type that deposited energy
  G4ParticleDefinition *particle = step->GetTrack()->GetDefinition();
  G4bool isNeutron = (particle == G4Neutron::Neutron());
  G4bool isGamma = (particle == G4Gamma::Gamma());

  // We can also track other particles (protons, alphas, electrons, etc.)
  // For now, we classify as "neutron" or "not neutron" (gamma + secondaries)

  // Get scoring volumes
  G4LogicalVolume *fScoringVolumeNaI =
      detectorConstruction->GetScoringVolumeNaI();
  G4LogicalVolume *fScoringVolumeEJ309 =
      detectorConstruction->GetScoringVolumeEJ309();

  // Check if step is in NaI(Tl) detector
  if (volume == fScoringVolumeNaI) {
    fEventAction->AddEdepNaI(edep, time, isNeutron);
  }

  // Check if step is in EJ-309 detector
  if (volume == fScoringVolumeEJ309) {
    fEventAction->AddEdepEJ309(edep, time, isNeutron);
  }
}
