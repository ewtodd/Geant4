// include detector construction files
#include "construction.hh"

// include physics files
#include "physics.hh"

// include actions
#include "action.hh"

// skeleton files
#include "G4MTRunManager.hh"
#include "G4RunManager.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4VisManager.hh"

int main(int argc, char **argv) {

#ifdef G4MULTITHREADED
  G4MTRunManager *runManager = new G4MTRunManager();
#else
  G4RunManager *runManager = new G4RunManager();
#endif

  G4double len_wid = (10.0 / 2) * cm;
  G4double thick = (0.5 / 2) * cm;

  if (argc > 1) {
    thick = std::stod(argv[1]) / 2 * cm;
    len_wid = std::stod(argv[2]) / 2 * cm;
  }
  runManager->SetUserInitialization(new PhysicsList());
  runManager->SetUserInitialization(new DetectorConstruction(thick, len_wid));
  runManager->SetUserInitialization(new ActionInitialization());

  G4UIExecutive *ui = 0;

  if (argc == 1) {
    ui = new G4UIExecutive(argc, argv);
  }

  G4VisManager *visManager = nullptr;
  G4UImanager *uiManager = G4UImanager::GetUIpointer();
  uiManager->ApplyCommand(G4String("/process/verbose       0"));
  uiManager->ApplyCommand(G4String("/process/em/verbose    0"));
  uiManager->ApplyCommand(G4String("/process/had/verbose   0"));
  uiManager->ApplyCommand(G4String("/process/eLoss/verbose 0"));

  uiManager->ApplyCommand(G4String("/control/verbose  0"));
  uiManager->ApplyCommand(G4String("/run/verbose      0"));
  uiManager->ApplyCommand(G4String("/event/verbose    0"));
  uiManager->ApplyCommand(G4String("/hits/verbose     0"));
  uiManager->ApplyCommand(G4String("/tracking/verbose 0"));
  uiManager->ApplyCommand(G4String("/stepping/verbose 0"));
  if (ui) {
    runManager->Initialize();
    visManager = new G4VisExecutive();
    visManager->Initialize();
    uiManager->ApplyCommand("/control/execute vis.mac");
    ui->SessionStart();
    delete ui;
  } else {
    G4String command = "/control/execute ";
    G4String fileName = argv[3];
    uiManager->ApplyCommand(command + fileName);
  }

  delete visManager;
  delete runManager;

  return 0;
}
