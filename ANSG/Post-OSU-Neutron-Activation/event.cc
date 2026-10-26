#include "event.hh"

EventAction::EventAction(RunAction *) {
  for (G4int i = 0; i < 4; i++)
    fEdepCZT[i] = 0.;
}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event *) {
  for (G4int i = 0; i < 4; i++)
    fEdepCZT[i] = 0.;
}

void EventAction::AddEdepCZT(G4int crystalID, G4double edep) {
  if (crystalID >= 0 && crystalID < 4)
    fEdepCZT[crystalID] += edep;
}

void EventAction::EndOfEventAction(const G4Event *) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  for (G4int i = 0; i < 4; i++) {
    if (fEdepCZT[i] > 1e-9) {
      man->FillNtupleDColumn(i, 0, fEdepCZT[i] / keV);
      man->AddNtupleRow(i);
    }
  }
}
