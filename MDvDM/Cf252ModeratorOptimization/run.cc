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

// Named after the moderator thickness, Poly<t>cm, so each run of a thickness
// scan (scan.mac) leaves its own file, under the names
// macros/ThermalNeutrons.cpp reads.
G4String RunAction::OutputStem() const {
  const DetectorConstruction *detector =
      static_cast<const DetectorConstruction *>(
          G4RunManager::GetRunManager()->GetUserDetectorConstruction());
  return Form("Poly%gcm", detector->GetModeratorThickness() / cm);
}

void RunAction::BeginOfRunAction(const G4Run *) {
  if (IsMaster())
    OutputMerger::MarkRunStart();
  G4AnalysisManager *man = G4AnalysisManager::Instance();
  man->OpenFile(OutputStem() + ".root");
}
void RunAction::EndOfRunAction(const G4Run *) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  man->Write();
  man->CloseFile();

  if (IsMaster() && G4Threading::IsMultithreadedApplication()) {
    G4int nThreads = G4RunManager::GetRunManager()->GetNumberOfThreads();
    OutputMerger::MergeThreadOutputs(OutputStem(), nThreads);
  }
}
