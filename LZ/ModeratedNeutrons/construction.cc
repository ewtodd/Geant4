#include "construction.hh"
#include "G4UnitsTable.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  // Create world volume - increased size to accommodate the lead wall
  G4Box *solidWorld = new G4Box("solidWorld", 0.2 * m, 0.2 * m, 0.3 * m);
  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");
  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

  // Define moderator material
  G4Material *moderatorMaterial =
      worldMat; // nist->FindOrBuildMaterial("G4_POLYETHYLENE");

  G4double modBoxHalfX = 15 * cm;
  G4double modBoxHalfY = 15 * cm;
  G4double modBoxHalfZ = (8.0 / 2) * cm;

  G4double offset = 1 * cm;

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

  // Define lead wall material and dimensions
  G4Material *leadMaterial = nist->FindOrBuildMaterial("G4_Pb");
  G4double leadWallHalfX = 15 * cm;
  G4double leadWallHalfY = 15 * cm;
  G4double leadWallHalfZ = 4.5 * cm; // 9cm thick wall (half-length)
  G4double holeRadius = 1.0 * cm;    // Radius of the hole for neutrons

  // Position of the lead wall - right after the moderator
  G4double leadWallPosZ = modBoxHalfZ * 2 + offset + leadWallHalfZ;

  // Create the lead wall with a hole
  // First create a solid lead wall
  G4Box *solidLeadWall =
      new G4Box("LeadWall", leadWallHalfX, leadWallHalfY, leadWallHalfZ);

  // Create a cylinder for the hole
  G4Tubs *solidHole =
      new G4Tubs("Hole", 0, holeRadius, leadWallHalfZ + 1 * mm, 0, 360 * deg);

  // Subtract the hole from the lead wall
  G4SubtractionSolid *leadWallWithHole =
      new G4SubtractionSolid("LeadWallWithHole", solidLeadWall, solidHole);

  // Create logical volume for the lead wall with hole
  G4LogicalVolume *logicLeadWall =
      new G4LogicalVolume(leadWallWithHole, leadMaterial, "LeadWall");

  // Set visual attributes for the lead wall
  G4VisAttributes *leadVisAttr =
      new G4VisAttributes(G4Colour(0.5, 0.5, 0.5)); // Gray
  logicLeadWall->SetVisAttributes(leadVisAttr);

  // Place the lead wall in the world volume
  G4VPhysicalVolume *physLeadWall =
      new G4PVPlacement(0, G4ThreeVector(0., 0., leadWallPosZ), logicLeadWall,
                        "LeadWall", logicWorld, false, 0, true);

  // Calculate new position offset for detectors (after the lead wall)
  G4double newOffset = leadWallPosZ + leadWallHalfZ;

  G4Material *LiF = nist->FindOrBuildMaterial("G4_LITHIUM_FLUORIDE");
  //  1. Create the Li-7 isotope
  G4Isotope *Li7 = new G4Isotope("Li7", 3, 7, 7.016 * g / mole);

  // 2. Create an element made only of Li-7
  G4Element *elLi7 = new G4Element("Lithium7", "Li7", 1);
  elLi7->AddIsotope(Li7, 100. * perCent);

  G4Box *solidLiF = new G4Box("LiF", 0.5 * cm, 0.5 * cm, 0.5 * cm);
  G4LogicalVolume *logicLiF = new G4LogicalVolume(solidLiF, LiF, "LiF");

  G4Material *LaBr3 = new G4Material("LaBr3Ce", 5.08 * g / cm3, 3);
  LaBr3->AddElement(nist->FindOrBuildElement("Br"), 0.640569);
  LaBr3->AddElement(nist->FindOrBuildElement("La"), 0.357575);
  LaBr3->AddElement(nist->FindOrBuildElement("Ce"), 0.001856);
  G4double inch = 2.54 * cm;
  G4Tubs *solidLaBr3 =
      new G4Tubs("LaBr3", 0, 1.5 * inch / 2, 1.5 * inch / 2, 0, 360 * deg);

  fScoringVolumeLaBr3 = new G4LogicalVolume(solidLaBr3, LaBr3, "LaBr3");

  G4Material *CeBr3 = new G4Material("CeBr3", 5.2 * g / cm3, 2);
  CeBr3->AddElement(nist->FindOrBuildElement("Ce"), 0.36893); // 36.893% cerium
  CeBr3->AddElement(nist->FindOrBuildElement("Br"), 0.63107); // 63.107% bromine

  G4Tubs *solidCeBr3 =
      new G4Tubs("CeBr3", 0, 1 * inch / 2, 1 * inch / 2, 0, 360 * deg);
  fScoringVolumeCeBr3 = new G4LogicalVolume(solidCeBr3, CeBr3, "CeBr3");

  G4double halfcm = 0.5 * cm;
  // Place LiF after the lead wall
  G4VPhysicalVolume *physLiF = new G4PVPlacement(
      0, G4ThreeVector(0, 0, newOffset + (1.5 * inch) / 2 + 0.01 * cm),
      logicLiF, "LiF", logicWorld, false, 0, true);

  // Place LaBr3 detector after the lead wall
  G4VPhysicalVolume *physLaBr3 = new G4PVPlacement(
      0,
      G4ThreeVector(0, (1.5 * inch) / 2 + halfcm + inch,
                    newOffset + (1.5 * inch) / 2 + 0.01 * cm),
      fScoringVolumeLaBr3, "LaBr3", logicWorld, false, 0, true);

  // Place CeBr3 detector after the lead wall
  G4VPhysicalVolume *physCeBr3 = new G4PVPlacement(
      0,
      G4ThreeVector(0, -((1 * inch) / 2 + halfcm + inch),
                    newOffset + (1.5 * inch) / 2 + 0.01 * cm),
      fScoringVolumeCeBr3, "CeBr3", logicWorld, false, 0, true);

  // Visualization attributes
  G4VisAttributes *liFVis =
      new G4VisAttributes(G4Colour(0.8, 0.8, 0.0)); // Yellow
  logicLiF->SetVisAttributes(liFVis);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  // LaBr3 sensitive detector
  SensitiveDetector *LaBr3SD = new SensitiveDetector("LaBr3");
  fScoringVolumeLaBr3->SetSensitiveDetector(LaBr3SD);
  // LaBr3 sensitive detector
  SensitiveDetector *CeBr3SD = new SensitiveDetector("CeBr3");
  fScoringVolumeCeBr3->SetSensitiveDetector(CeBr3SD);
}
