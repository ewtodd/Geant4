#include "event.hh"

EventAction::EventAction(RunAction *) {
  fEdepLaBr3 = fEdepCeBr3 = 0.;
  fTimeLaBr3 = fTimeCeBr3 = -1.;
}
EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event *) {
  fEdepLaBr3 = fEdepCeBr3 = 0.;
  fTimeLaBr3 = fTimeCeBr3 = -1.;
}

void EventAction::AddEdepLaBr3(G4double edep, G4double time) {
  fEdepLaBr3 += edep;
  if (fTimeLaBr3 < 0)
    fTimeLaBr3 = time; // Store first hit time
}

void EventAction::AddEdepCeBr3(G4double edep, G4double time) {
  fEdepCeBr3 += edep;
  if (fTimeCeBr3 < 0)
    fTimeCeBr3 = time;
}

void EventAction::EndOfEventAction(const G4Event *) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();
  if (fEdepLaBr3 > 1e-7 || fEdepCeBr3 > 1e-7) {
    // Column 0: LaBr3 Edep
    man->FillNtupleDColumn(0, 0, fEdepLaBr3 / MeV);
    // Column 1: LaBr3 Time
    man->FillNtupleDColumn(0, 1, fTimeLaBr3 / ns);
    man->AddNtupleRow(0);
    // Column 2: CeBr3 Edep
    man->FillNtupleDColumn(1, 0, fEdepCeBr3 / MeV);
    // Column 3: CeBr3 Time
    man->FillNtupleDColumn(1, 1, fTimeCeBr3 / ns);
    man->AddNtupleRow(1);
  }
}
