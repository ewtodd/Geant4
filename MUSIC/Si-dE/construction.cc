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

  // Dead layer thicknesses
  G4double frontDeadHalfHeight = (0.1 / 2) * um; // 100 nm front dead layer
  G4double rearDeadHalfHeight = (0.2 / 2) * um;  // 200 nm rear dead layer
  G4double activeHalfHeight =
      SiHalfHeight - frontDeadHalfHeight - rearDeadHalfHeight;

  // make Si diode - ORTEC TD-030-150-50
  G4Tubs *solidActiveSi = new G4Tubs("solidActiveSi", 0., radius,
                                     activeHalfHeight, 0 * deg, 360 * deg);

  G4Tubs *solidFrontDead = new G4Tubs("solidFrontDead", 0., radius,
                                      frontDeadHalfHeight, 0 * deg, 360 * deg);

  G4Tubs *solidRearDead = new G4Tubs("solidRearDead", 0., radius,
                                     rearDeadHalfHeight, 0 * deg, 360 * deg);

  G4Tubs *enterTi =
      new G4Tubs("enterTi", 0., radius, TiEnterHalfHeight, 0 * deg, 360 * deg);
  G4Tubs *exitTi =
      new G4Tubs("exitTi", 0., radius, TiExitHalfHeight, 0 * deg, 360 * deg);

  // Make logical world
  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

  // Logical volumes
  logicDetector = new G4LogicalVolume(solidActiveSi, SiMat, "logicDetector");
  G4LogicalVolume *logicFrontDead =
      new G4LogicalVolume(solidFrontDead, SiMat, "logicFrontDead");
  G4LogicalVolume *logicRearDead =
      new G4LogicalVolume(solidRearDead, SiMat, "logicRearDead");
  G4LogicalVolume *logicEnterTi =
      new G4LogicalVolume(enterTi, TiMat, "logicEnterTi");
  G4LogicalVolume *logicExitTi =
      new G4LogicalVolume(exitTi, TiMat, "logicExitTi");

  // Position Ti windows (both in front)
  G4double enterTiZ =
      1 * cm - SiHalfHeight - 2 * TiExitHalfHeight - TiEnterHalfHeight;
  G4VPhysicalVolume *physEnterTi =
      new G4PVPlacement(0, G4ThreeVector(0., 0., enterTiZ), logicEnterTi,
                        "physEnterTi", logicWorld, false, 0, true);

  G4double exitTiZ = 1 * cm - SiHalfHeight - TiExitHalfHeight;
  G4VPhysicalVolume *physExitTi =
      new G4PVPlacement(0, G4ThreeVector(0., 0., exitTiZ), logicExitTi,
                        "physExitTi", logicWorld, false, 0, true);

  // Position front dead layer (front face of Si)
  G4double frontDeadZ = 1 * cm - SiHalfHeight + frontDeadHalfHeight;
  G4VPhysicalVolume *physFrontDead =
      new G4PVPlacement(0, G4ThreeVector(0., 0., frontDeadZ), logicFrontDead,
                        "physFrontDead", logicWorld, false, 0, true);

  // Position active Si detector
  G4double activeSiZ =
      1 * cm - SiHalfHeight + 2 * frontDeadHalfHeight + activeHalfHeight;
  G4VPhysicalVolume *physDet =
      new G4PVPlacement(0, G4ThreeVector(0., 0., activeSiZ), logicDetector,
                        "physDet", logicWorld, false, 0, true);

  // Position rear dead layer (back face of Si)
  G4double rearDeadZ = 1 * cm + SiHalfHeight - rearDeadHalfHeight;
  G4VPhysicalVolume *physRearDead =
      new G4PVPlacement(0, G4ThreeVector(0., 0., rearDeadZ), logicRearDead,
                        "physRearDead", logicWorld, false, 0, true);

  // Add colors
  G4VisAttributes *blueColor =
      new G4VisAttributes(G4Colour(0.0, 0.0, 1.0)); // Blue for all Si
  G4VisAttributes *grayColor =
      new G4VisAttributes(G4Colour(0.5, 0.5, 0.5)); // Gray for Ti enter
  G4VisAttributes *redColor =
      new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // Red for Ti exit

  logicDetector->SetVisAttributes(blueColor);
  logicFrontDead->SetVisAttributes(blueColor); // Same color as active Si
  logicRearDead->SetVisAttributes(blueColor);  // Same color as active Si
  logicEnterTi->SetVisAttributes(grayColor);
  logicExitTi->SetVisAttributes(redColor);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  SensitiveDetector *sensDet = new SensitiveDetector("Det");
  logicDetector->SetSensitiveDetector(
      sensDet); // Only active region is sensitive
}
