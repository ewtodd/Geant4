#include "construction.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  G4Material *detMat = nist->FindOrBuildMaterial("G4_Si");
  // constructor arguments are name and half of x,y,z length (in m)
  G4Box *solidWorld = new G4Box("solidWorld", 0.25 * m, 0.25 * m, 0.25 * m);

  G4double radius = 11.968 * mm;
  G4double halfHeight = 150 * um;
  // make Si diode - same size as Canberra partially depleted
  G4Tubs *solidDet =
      new G4Tubs("solidSi", 0., radius, halfHeight, 0 * deg, 360 * deg);

  // Make logical world
  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);
  logicDetector = new G4LogicalVolume(solidDet, detMat, "logicDetector");

  // place SiLi Detector
  G4RotationMatrix *rotationMatrix = new G4RotationMatrix();
  rotationMatrix->rotateY(60. * deg);
  G4VPhysicalVolume *physDet =
      new G4PVPlacement(rotationMatrix, G4ThreeVector(0., 0., 2.54 * cm),
                        logicDetector, "physDet", logicWorld, false, 0, true);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  SensitiveDetector *sensDet = new SensitiveDetector("Det");
  logicDetector->SetSensitiveDetector(sensDet);
}
