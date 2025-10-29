#include "run.hh"

RunAction::RunAction() {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  man->CreateNtuple("CdTe", "CdTe");
  man->CreateNtupleDColumn("fEDep");
  man->FinishNtuple(0);
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
  man->CloseFile("output.root");
}
