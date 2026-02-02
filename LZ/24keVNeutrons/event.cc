#include "event.hh"

EventAction::EventAction(RunAction *) {
  fEdepNaI_Gamma = fEdepNaI_Neutron = 0.;
  fEdepEJ309_Gamma = fEdepEJ309_Neutron = 0.;
  fTimeNaI_Gamma = fTimeNaI_Neutron = -1.;
  fTimeEJ309_Gamma = fTimeEJ309_Neutron = -1.;
}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event *) {
  // Reset all counters at the beginning of each event
  fEdepNaI_Gamma = fEdepNaI_Neutron = 0.;
  fEdepEJ309_Gamma = fEdepEJ309_Neutron = 0.;
  fTimeNaI_Gamma = fTimeNaI_Neutron = -1.;
  fTimeEJ309_Gamma = fTimeEJ309_Neutron = -1.;
}

void EventAction::AddEdepNaI(G4double edep, G4double time, G4bool isNeutron) {
  if (isNeutron) {
    fEdepNaI_Neutron += edep;
    if (fTimeNaI_Neutron < 0)
      fTimeNaI_Neutron = time; // Store first neutron hit time
  } else {
    fEdepNaI_Gamma += edep;
    if (fTimeNaI_Gamma < 0)
      fTimeNaI_Gamma = time; // Store first gamma hit time
  }
}

void EventAction::AddEdepEJ309(G4double edep, G4double time, G4bool isNeutron) {
  if (isNeutron) {
    fEdepEJ309_Neutron += edep;
    if (fTimeEJ309_Neutron < 0)
      fTimeEJ309_Neutron = time; // Store first neutron hit time
  } else {
    fEdepEJ309_Gamma += edep;
    if (fTimeEJ309_Gamma < 0)
      fTimeEJ309_Gamma = time; // Store first gamma hit time
  }
}

void EventAction::EndOfEventAction(const G4Event *) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  // Only write if there's any energy deposit in any detector
  G4bool hasDeposit = (fEdepNaI_Gamma > 1e-9 || fEdepNaI_Neutron > 1e-9 ||
                       fEdepEJ309_Gamma > 1e-9 || fEdepEJ309_Neutron > 1e-9);

  if (hasDeposit) {
    // Ntuple 0: NaI(Tl) Gamma hits
    man->FillNtupleDColumn(0, 0, fEdepNaI_Gamma / MeV);
    man->FillNtupleDColumn(0, 1, fTimeNaI_Gamma / ns);
    man->AddNtupleRow(0);

    // Ntuple 1: NaI(Tl) Neutron hits
    man->FillNtupleDColumn(1, 0, fEdepNaI_Neutron / MeV);
    man->FillNtupleDColumn(1, 1, fTimeNaI_Neutron / ns);
    man->AddNtupleRow(1);

    // Ntuple 2: EJ-309 Gamma hits
    man->FillNtupleDColumn(2, 0, fEdepEJ309_Gamma / MeV);
    man->FillNtupleDColumn(2, 1, fTimeEJ309_Gamma / ns);
    man->AddNtupleRow(2);

    // Ntuple 3: EJ-309 Neutron hits
    man->FillNtupleDColumn(3, 0, fEdepEJ309_Neutron / MeV);
    man->FillNtupleDColumn(3, 1, fTimeEJ309_Neutron / ns);
    man->AddNtupleRow(3);
  }
}
