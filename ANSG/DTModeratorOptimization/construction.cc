#include "construction.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  // Create world volume
  G4Box *solidWorld = new G4Box("solidWorld", 5.0 * m, 5.0 * m, 5.0 * m);
  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");
  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

  // Define moderator material
  G4Material *moderatorMaterial = nist->FindOrBuildMaterial("G4_POLYETHYLENE");
  // nist->FindOrBuildMaterial("G4_POLYETHYLENE");
  // Define dimensions of the moderator slab
  G4double modBoxHalfX = 50 * cm;
  G4double modBoxHalfY = 50 * cm;
  G4double modBoxHalfZ = (10.0 / 2) * cm;

  G4double offset = 10 * cm;

  // Create the moderator box
  G4Box *modBox = new G4Box("ModBox", modBoxHalfX, modBoxHalfY, modBoxHalfZ);
  G4LogicalVolume *logicModBox =
      new G4LogicalVolume(modBox, moderatorMaterial, "ModBox");
  // Create and set visual attributes for the moderator box
  G4VisAttributes *moderatorVisAttr =
      new G4VisAttributes(G4Colour(0.0, 1.0, 0.0)); // Green
  logicModBox->SetVisAttributes(moderatorVisAttr);
  // Place the moderator box in the world volume
  G4VPhysicalVolume *physModBox =
      new G4PVPlacement(0, G4ThreeVector(0., 0., modBoxHalfZ + offset),
                        logicModBox, "ModBox", logicWorld, false, 0, true);

  // Define shell dimensions (detector)
  G4double shellOuterX = modBoxHalfX;
  G4double shellOuterY = modBoxHalfY;
  G4double shellOuterZ = 1 * cm; // Thin layer for detection

  // Create the shells
  G4Box *shellBox =
      new G4Box("ShellBox", shellOuterX, shellOuterY, shellOuterZ);
  fScoringVolumeWrapPre = new G4LogicalVolume(shellBox, worldMat, "ShellPre");
  fScoringVolumeWrapPost = new G4LogicalVolume(shellBox, worldMat, "ShellPost");
  // Create and set visual attributes for the shell
  G4VisAttributes *shellVisAttr =
      new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // Red
  G4VisAttributes *preVisAttr = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0));

  fScoringVolumeWrapPost->SetVisAttributes(shellVisAttr);
  fScoringVolumeWrapPre->SetVisAttributes(preVisAttr);
  G4VPhysicalVolume *physShellPost = new G4PVPlacement(
      0, G4ThreeVector(0., 0., offset + 2 * modBoxHalfZ + shellOuterZ),
      fScoringVolumeWrapPost, "ShellPost", logicWorld, false, 0, true);
  G4VPhysicalVolume *physShellPre = new G4PVPlacement(
      0, G4ThreeVector(0., 0., offset - shellOuterZ), fScoringVolumeWrapPre,
      "ShellPre", logicWorld, false, 0, true);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  SensitiveDetector *sensDetWrapPost = new SensitiveDetector("WrapPost");
  fScoringVolumeWrapPost->SetSensitiveDetector(sensDetWrapPost);
  SensitiveDetector *sensDetWrapPre = new SensitiveDetector("WrapPre");
  fScoringVolumeWrapPre->SetSensitiveDetector(sensDetWrapPre);
}
