#include "run.hh"

RunAction::RunAction() {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  // Ntuple 0: NaI(Tl) Gamma hits
  man->CreateNtuple("NaI_Gamma", "Gamma hits in NaI(Tl)");
  man->CreateNtupleDColumn("Edep"); // Column 0
  man->CreateNtupleDColumn("Time"); // Column 1
  man->FinishNtuple(0);

  // Ntuple 1: NaI(Tl) Neutron hits
  man->CreateNtuple("NaI_Neutron", "Neutron hits in NaI(Tl)");
  man->CreateNtupleDColumn("Edep"); // Column 0
  man->CreateNtupleDColumn("Time"); // Column 1
  man->FinishNtuple(1);

  // Ntuple 2: EJ-309 Gamma hits
  man->CreateNtuple("EJ309_Gamma", "Gamma hits in EJ-309");
  man->CreateNtupleDColumn("Edep"); // Column 0
  man->CreateNtupleDColumn("Time"); // Column 1
  man->FinishNtuple(2);

  // Ntuple 3: EJ-309 Neutron hits
  man->CreateNtuple("EJ309_Neutron", "Neutron hits in EJ-309");
  man->CreateNtupleDColumn("Edep"); // Column 0
  man->CreateNtupleDColumn("Time"); // Column 1
  man->FinishNtuple(3);
}
RunAction::~RunAction() {}
void RunAction::BeginOfRunAction(const G4Run *run) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();
  G4int runNumber = run->GetRunID();
  std::stringstream strRunID;
  strRunID << runNumber;
  man->OpenFile("output" + strRunID.str() + ".root");
}
void RunAction::EndOfRunAction(const G4Run *) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  man->Write();
  man->CloseFile();
}
