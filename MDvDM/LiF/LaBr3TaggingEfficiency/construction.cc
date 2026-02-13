#include "construction.hh"
#include "G4UnitsTable.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  // World volume
  G4Box *solidWorld = new G4Box("World", 0.25 * m, 0.25 * m, 0.25 * m);
  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "World");
  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(), logicWorld, "World", 0, false, 0, true);

  G4Material *LiF = nist->FindOrBuildMaterial("G4_LITHIUM_FLUORIDE");

  G4Box *solidLiF = new G4Box("LiF", 0.5 * cm, 0.5 * cm, 0.5 * cm);
  G4LogicalVolume *logicLiF = new G4LogicalVolume(solidLiF, LiF, "LiF");

  G4Box *solidShell = new G4Box("AirShell",
                                0.5 * cm + 1 * mm,  // X half-length
                                0.5 * cm + 1 * mm,  // Y half-length
                                0.5 * cm + 1 * mm); // Z half-length

  fScoringVolumeShell = new G4LogicalVolume(solidShell, worldMat, "AirShell");

  G4VPhysicalVolume *physLiF =
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), // Relative to AirShell
                        logicLiF, "LiF",
                        fScoringVolumeShell, // Mother volume
                        false, 0, true);
  G4VPhysicalVolume *physShell =
      new G4PVPlacement(0, G4ThreeVector(0, 0, 10 * cm), fScoringVolumeShell,
                        "AirShell", logicWorld, false, 0, true);

  G4Element *La = nist->FindOrBuildElement("La");
  G4Element *Br = nist->FindOrBuildElement("Br");
  G4Material *LaBr3 = new G4Material("LaBr3", 5.29 * g / cm3, 2);
  LaBr3->AddElement(La, 1);
  LaBr3->AddElement(Br, 3);
  G4double inch = 2.54 * cm;
  G4Tubs *solidLaBr3 =
      new G4Tubs("LaBr3", 0, 1.5 * inch / 2, 1.5 * inch / 2, 0, 360 * deg);

  fScoringVolumeLaBr3 = new G4LogicalVolume(solidLaBr3, LaBr3, "LaBr3");
  G4double halfcm = 0.5 * cm;
  G4VPhysicalVolume *physLaBr3 = new G4PVPlacement(
      0, G4ThreeVector(0, (1.5 * inch) / 2 + halfcm + inch, 10 * cm),
      fScoringVolumeLaBr3, "LaBr3", logicWorld, false, 0, true);
  // Visualization attributes
  G4VisAttributes *liFVis =
      new G4VisAttributes(G4Colour(0.8, 0.8, 0.0)); // Yellow
  logicLiF->SetVisAttributes(liFVis);

  G4VisAttributes *shellVis =
      new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.3)); // Cyan, translucent
  fScoringVolumeShell->SetVisAttributes(shellVis);

  G4VisAttributes *LaBr3Vis =
      new G4VisAttributes(G4Colour(0.0, 0.0, 1.0)); // Blue
  fScoringVolumeLaBr3->SetVisAttributes(LaBr3Vis);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  // Air shell sensitive detector
  SensitiveDetector *shellSD = new SensitiveDetector("AirShell");
  fScoringVolumeShell->SetSensitiveDetector(shellSD);

  // LaBr3 sensitive detector
  SensitiveDetector *LaBr3SD = new SensitiveDetector("LaBr3");
  fScoringVolumeLaBr3->SetSensitiveDetector(LaBr3SD);
}
