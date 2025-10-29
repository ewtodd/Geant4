#include "construction.hh"
#include "G4UnitsTable.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  // Define moderator material
  G4Material *moderatorMaterial = nist->FindOrBuildMaterial(
      "G4_AIR"); // nist->FindOrBuildMaterial("G4_POLYETHYLENE");

  G4double modBoxHalfX = 15 * cm;
  G4double modBoxHalfY = 15 * cm;
  G4double modBoxHalfZ = (8.0 / 2) * cm; // 4 cm half-height

  G4double offset = 1 * cm;

  // HPGe detector parameters
  G4double radius = 20 * mm;
  G4double halfHeight = 4.75 * mm;

  // CZT detector: 1x1x1 cm³
  G4double czt_half_size = 0.5 * cm;

  // Calculate geometry extents for minimal world volume
  G4double moderator_back_face = 2 * modBoxHalfZ + offset;          // 9 cm
  G4double hpge_z_position = moderator_back_face + 1 * cm + radius; // 11 cm
  G4double czt_x_position = halfHeight + 2 * mm + czt_half_size;    // 1.175 cm

  // Calculate minimal world dimensions with 2cm safety margin
  G4double safety_margin = 2 * cm;
  G4double world_half_x = modBoxHalfX + safety_margin;              // 17 cm
  G4double world_half_y = modBoxHalfY + safety_margin;              // 17 cm
  G4double world_half_z = hpge_z_position + radius + safety_margin; // 15 cm

  // Create optimally-sized world volume
  G4Box *solidWorld =
      new G4Box("solidWorld", world_half_x, world_half_y, world_half_z);
  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");
  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

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

  // HPGe detector parameters
  G4Material *Ge = nist->FindOrBuildMaterial("G4_Ge");

  // Create HPGe - the G4Tubs axis is along z by default
  G4Tubs *solidGe =
      new G4Tubs("solidGe", 0., radius, halfHeight, 0 * deg, 360 * deg);

  logicDetectorHPGe = new G4LogicalVolume(solidGe, Ge, "logicDetectorHPGe");

  G4VisAttributes *GeVisAttr = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0));
  logicDetectorHPGe->SetVisAttributes(GeVisAttr);

  // Create rotation matrix to orient HPGe with diameter facing the beam
  // Rotate 90 degrees around Y-axis so the diameter faces in +z direction
  G4RotationMatrix *rotGe = new G4RotationMatrix();
  rotGe->rotateY(90 * deg);

  G4VPhysicalVolume *physGe = new G4PVPlacement(
      rotGe, G4ThreeVector(0., 0., hpge_z_position), logicDetectorHPGe,
      "physGe", logicWorld, false, 0, true);

  // Define G4Material for Cadmium Zinc Telluride (CZT)
  G4double density_CZT = 5.78 * g / cm3;
  G4int ncomponents = 3; // Cd, Zn, Te elements
  G4Material *CZT = new G4Material("CZT", density_CZT, ncomponents);

  // Add elements by weight fraction
  G4Element *Cd = new G4Element("Cadmium", "Cd", 48, 112.411 * g / mole);
  G4Element *Zn = new G4Element("Zinc", "Zn", 30, 65.380 * g / mole);
  G4Element *Te = new G4Element("Tellurium", "Te", 52, 127.600 * g / mole);

  CZT->AddElement(Cd, 0.368095); // 36.8095% by mass
  CZT->AddElement(Zn, 0.214084); // 21.4084% by mass
  CZT->AddElement(Te, 0.417821); // 41.7821% by mass

  G4Box *solidCZT =
      new G4Box("solidCZT", czt_half_size, czt_half_size, czt_half_size);

  logicDetectorCZT = new G4LogicalVolume(solidCZT, CZT, "logicDetectorCZT");

  G4VPhysicalVolume *physCZT = new G4PVPlacement(
      0, G4ThreeVector(czt_x_position, 0., hpge_z_position), logicDetectorCZT,
      "physCZT", logicWorld, false, 0, true);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  // Create sensitive detectors for both CZT and HPGe
  SensitiveDetector *sensDetCZT = new SensitiveDetector("CZT");
  SensitiveDetector *sensDetHPGe = new SensitiveDetector("HPGe");

  logicDetectorCZT->SetSensitiveDetector(sensDetCZT);
  logicDetectorHPGe->SetSensitiveDetector(sensDetHPGe);
}
