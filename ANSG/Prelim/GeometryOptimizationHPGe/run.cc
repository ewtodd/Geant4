#include "run.hh"

RunAction::RunAction() {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  man->CreateNtuple("Energy", "Energy");
  man->CreateNtupleDColumn("fEnergy");
  man->CreateNtupleDColumn("fpx");
  man->CreateNtupleDColumn("fpy");
  man->CreateNtupleDColumn("fpz");
  man->CreateNtupleDColumn("fx");
  man->CreateNtupleDColumn("fy");
  man->CreateNtupleDColumn("fz");
  man->FinishNtuple(0);
  man->CreateNtuple("Counts Produced", "Counts Produced");
  man->CreateNtupleIColumn("Produced");
  man->FinishNtuple(1);
  man->CreateNtuple("Counts Escaped", "Counts Escaped");
  man->CreateNtupleIColumn("Escaped");
  man->FinishNtuple(2);
}
RunAction::~RunAction() {}
void RunAction::BeginOfRunAction(const G4Run *run) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();
  std::string runnumber = std::to_string(run->GetRunID());
  G4String fileName = "output" + runnumber + ".root";
  man->OpenFile(fileName);
}
void RunAction::EndOfRunAction(const G4Run *) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  man->Write();
  man->CloseFile("output.root");
}
