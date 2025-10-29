#include "run.hh"

RunAction::RunAction() {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  // CZT detector ntuple
  man->CreateNtuple("CZT", "CZT Detector Data");
  man->CreateNtupleDColumn("Edep"); // Column 0: Total energy deposited (keV)
  man->CreateNtupleIColumn(
      "NeutronCount"); // Column 1: Number of neutrons in this event
  man->CreateNtupleDColumn(
      "NeutronEnergy"); // Column 2: Individual neutron kinetic energy (keV)
  man->FinishNtuple(0);

  // HPGe detector ntuple
  man->CreateNtuple("HPGe", "HPGe Detector Data");
  man->CreateNtupleDColumn("Edep"); // Column 0: Total energy deposited (keV)
  man->CreateNtupleIColumn(
      "NeutronCount"); // Column 1: Number of neutrons in this event
  man->CreateNtupleDColumn(
      "NeutronEnergy"); // Column 2: Individual neutron kinetic energy (keV)
  man->FinishNtuple(1);
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
  man->CloseFile("");
}
