#include "event.hh"

EventAction::EventAction(RunAction *) {
  fEdepGe = fEdepCdTe = fEdepNaI = 0.;
  fTimeGe = fTimeCdTe = fTimeNaI = -1.;
}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event *) {
  fEdepGe = fEdepCdTe = fEdepNaI = 0.;
  fTimeGe = fTimeCdTe = fTimeNaI = -1.;
}

void EventAction::AddEdepGe(G4double edep, G4double time) {
  fEdepGe += edep;
  if (fTimeGe < 0)
    fTimeGe = time; // Store first hit time
}

void EventAction::AddEdepCdTe(G4double edep, G4double time) {
  fEdepCdTe += edep;
  if (fTimeCdTe < 0)
    fTimeCdTe = time;
}

void EventAction::AddEdepNaI(G4double edep, G4double time) {
  fEdepNaI += edep;
  if (fTimeNaI < 0)
    fTimeNaI = time;
}

void EventAction::EndOfEventAction(const G4Event *) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  if (fEdepGe > 1e-7 || fEdepCdTe > 1e-7 || fEdepNaI > 1e-7) {
    // Ntuple 0: Ge
    man->FillNtupleDColumn(0, 0, fEdepGe / MeV);
    man->FillNtupleDColumn(0, 1, fTimeGe / ns);
    man->AddNtupleRow(0);

    // Ntuple 1: CdTe
    man->FillNtupleDColumn(1, 0, fEdepCdTe / MeV);
    man->FillNtupleDColumn(1, 1, fTimeCdTe / ns);
    man->AddNtupleRow(1);

    // Ntuple 2: NaI
    man->FillNtupleDColumn(2, 0, fEdepNaI / MeV);
    man->FillNtupleDColumn(2, 1, fTimeNaI / ns);
    man->AddNtupleRow(2);
  }
}
