#include "construction.hh"

#include "G4Tubs.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

// One Amptek-like planar CdTe crystal (25 mm2, 1 mm thick, as in
// Prelim/CdTe), its axis along z, centred at the origin, in air.
G4VPhysicalVolume *DetectorConstruction::Construct() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");
  G4Material *cdte = nist->FindOrBuildMaterial("G4_CADMIUM_TELLURIDE");

  G4Box *solidWorld = new G4Box("solidWorld", 30.0 * mm, 30.0 * mm, 30.0 * mm);
  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");
  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

  G4Tubs *solidDet =
      new G4Tubs("solidCdTe", 0., 2.82 * mm, 0.5 * mm, 0 * deg, 360 * deg);
  fScoringVolumeCZT = new G4LogicalVolume(solidDet, cdte, "logicCdTe");
  new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), fScoringVolumeCZT, "physCdTe",
                    logicWorld, false, 0, true);
  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  SensitiveDetector *det = new SensitiveDetector("CdTe");
  fScoringVolumeCZT->SetSensitiveDetector(det);
}
