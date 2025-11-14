#include "construction.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *worldMat = nist->FindOrBuildMaterial("G4_Galactic");

  G4Material *SiMat = nist->FindOrBuildMaterial("G4_Si");
  G4Material *TiMat = nist->FindOrBuildMaterial("G4_Ti");

  G4Box *solidWorld = new G4Box("solidWorld", 5 * cm, 5 * cm, 5 * cm);

  G4double radius = 6.909882989 * mm;
  G4double SiHalfHeight = 25 * um;
  G4double TiEnterHalfHeight = (1.9916351 / 2) * um; // 0.9 mg/cm^2
  G4double TiExitHalfHeight = (2.8768063 / 2) * um;  // 1.3 mg/cm^2

  // make Si diode - ORTEC TD-030-150-50
  G4Tubs *solidDet =
      new G4Tubs("solidSi", 0., radius, SiHalfHeight, 0 * deg, 360 * deg);

  // Make logical world
  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);
  logicDetector = new G4LogicalVolume(solidDet, SiMat, "logicDetector");

  G4VPhysicalVolume *physDet =
      new G4PVPlacement(0, G4ThreeVector(0., 0., 2.54 * cm), logicDetector,
                        "physDet", logicWorld, false, 0, true);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  SensitiveDetector *sensDet = new SensitiveDetector("Det");
  logicDetector->SetSensitiveDetector(sensDet);
}
