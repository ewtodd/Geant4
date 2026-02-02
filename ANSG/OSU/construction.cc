#include "construction.hh"

#include "G4UnitsTable.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {

  G4NistManager *nist = G4NistManager::Instance();

  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  // Create world volume

  G4Box *solidWorld = new G4Box("solidWorld", 0.1 * m, 0.1 * m, 0.2 * m);

  G4LogicalVolume *logicWorld =

      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

  G4VPhysicalVolume *physWorld = new G4PVPlacement(

      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

  G4double inch = 2.54 * cm;

  // Define CdTe material using GEANT4 built-in material

  G4Material *CdTe = nist->FindOrBuildMaterial("G4_CADMIUM_TELLURIDE");

  // Create CdTe detector - cylindrical shape with dimensions from uploaded file

  // radius = 2.82 mm, halfHeight = 0.5 mm (1 mm total thickness)

  G4double CdTeRadius = 2.82 * mm;

  G4double CdTeHalfHeight = 0.5 * mm;

  G4Tubs *solidCdTe =

      new G4Tubs("CdTe", 0., CdTeRadius, CdTeHalfHeight, 0 * deg, 360 * deg);

  fScoringVolumeCdTe = new G4LogicalVolume(solidCdTe, CdTe, "CdTe");

  G4double CdTePosZ = 5 * cm; // same z position as Ge center

  G4RotationMatrix *rotCdTe = new G4RotationMatrix();

  rotCdTe->rotateX(90 * deg);

  // Place CdTe detector below the beamline

  G4VPhysicalVolume *physCdTe =

      new G4PVPlacement(rotCdTe, G4ThreeVector(0, 0, CdTePosZ),

                        fScoringVolumeCdTe, "CdTe", logicWorld, false, 0, true);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {

  // CdTe sensitive detector

  SensitiveDetector *CdTeSD = new SensitiveDetector("CdTe");

  fScoringVolumeCdTe->SetSensitiveDetector(CdTeSD);
}
