#include "construction.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  // density value taken from Properties of Narrow Gap Cadmium-Based Compounds
  G4Material *CZT = new G4Material("CdZnTe", 5.811 * g / cm3, 3);
  CZT->AddElement(nist->FindOrBuildElement("Cd"), 1);
  CZT->AddElement(nist->FindOrBuildElement("Zn"), 1);
  CZT->AddElement(nist->FindOrBuildElement("Te"), 1);

  // constructor arguments are name and half of x,y,z length (in m)
  G4Box *solidWorld = new G4Box("solidWorld", 0.25 * m, 0.25 * m, 0.25 * m);

  // make the CZT 1x1x1 cm3
  G4Box *solidCZT = new G4Box("solidCZT", 0.5 * cm, 0.5 * cm, 0.5 * cm);

  // Make logical world
  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);
  logicDetectorCZT = new G4LogicalVolume(solidCZT, CZT, "logicDetectorCZT");

  G4VPhysicalVolume *physCZT = new G4PVPlacement(
      0, G4ThreeVector(0.5 * cm, 0., 1 * cm), logicDetectorCZT, "physCZT",
      logicWorld, false, 0, true);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  SensitiveDetector *sensDetCZT = new SensitiveDetector("CZT Det");
  logicDetectorCZT->SetSensitiveDetector(sensDetCZT);
}
