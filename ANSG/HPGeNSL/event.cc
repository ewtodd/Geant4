#include "event.hh"

EventAction::EventAction(RunAction *) {
  fEdepCZT = 0.;
  fEdepHPGe = 0.;
  fCZTNeutronCount = 0;
  fHPGeNeutronCount = 0;
}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event *) {
  fEdepCZT = 0.;
  fEdepHPGe = 0.;
  fCZTNeutronCount = 0;
  fHPGeNeutronCount = 0;
}

void EventAction::EndOfEventAction(const G4Event *) {
  // This function should NOT fill neutron energy data!
  // Neutron energies are filled individually in detector.cc when each neutron
  // hits

  // Only fill per-event summary data if there was energy deposition
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  if (fEdepCZT > 0.0000001) {
    man->FillNtupleDColumn(0, 0, fEdepCZT / keV); // CZT energy deposition
    man->FillNtupleIColumn(
        0, 1, fCZTNeutronCount); // CZT neutron count for this event
    man->FillNtupleDColumn(
        0, 2, -100.0); // -1 indicates this is event summary, not neutron data
    man->AddNtupleRow(0);
  }

  if (fEdepHPGe > 0.0000001) {
    man->FillNtupleDColumn(1, 0, fEdepHPGe / keV); // HPGe energy deposition
    man->FillNtupleIColumn(
        1, 1, fHPGeNeutronCount); // HPGe neutron count for this event
    man->FillNtupleDColumn(
        1, 2, -100.0); // -1 indicates this is event summary, not neutron data
    man->AddNtupleRow(1);
  }
}
