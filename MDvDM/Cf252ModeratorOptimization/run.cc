#include "run.hh"

RunAction::RunAction() {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  man->CreateNtuple("PreEnergy", "PreEnergy");
  man->CreateNtupleDColumn("fPreEnergy");
  man->FinishNtuple(0);

  man->CreateNtuple("PostEnergy", "PostEnergy");
  man->CreateNtupleDColumn("fPostEnergy");
  man->FinishNtuple(1);

  // Additional columns for position (x, y, z) and momentum (px, py, pz)
  man->CreateNtupleDColumn("fPostPosX");
  man->CreateNtupleDColumn("fPostPosY");
  man->CreateNtupleDColumn("fPostPosZ");
  man->CreateNtupleDColumn("fPostMomX");
  man->CreateNtupleDColumn("fPostMomY");
  man->CreateNtupleDColumn("fPostMomZ");
}
RunAction::~RunAction() {}
void RunAction::BeginOfRunAction(const G4Run *run) {
  if (IsMaster())
    OutputMerger::MarkRunStart();
  G4AnalysisManager *man = G4AnalysisManager::Instance();
  G4int runNumber = run->GetRunID();
  std::stringstream strRunID;
  strRunID << runNumber;
  man->OpenFile("output" + strRunID.str() + ".root");
}
void RunAction::EndOfRunAction(const G4Run *run) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  man->Write();
  man->CloseFile();

  if (IsMaster() && G4Threading::IsMultithreadedApplication()) {
    G4int nThreads = G4RunManager::GetRunManager()->GetNumberOfThreads();
    OutputMerger::MergeThreadOutputs(Form("output%d", run->GetRunID()),
                                     nThreads);
  }
}
