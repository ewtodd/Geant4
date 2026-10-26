#include "construction.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {

  G4NistManager *nist = G4NistManager::Instance();

  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  // Create world volume
  G4Box *solidWorld = new G4Box("solidWorld", 30.0 * mm, 30.0 * mm, 15.0 * mm);

  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

  // Define CZT material: Cd_0.9 Zn_0.1 Te (by atom count)
  G4Element *Cd = nist->FindOrBuildElement("Cd");
  G4Element *Zn = nist->FindOrBuildElement("Zn");
  G4Element *Te = nist->FindOrBuildElement("Te");

  G4double densityCZT = 5.78 * g / cm3;
  G4Material *CZT = new G4Material("CZT", densityCZT, 3);
  CZT->AddElement(Cd, 9);  // 9 parts Cd
  CZT->AddElement(Zn, 1);  // 1 part Zn
  CZT->AddElement(Te, 10); // 10 parts Te

  // H3D M400 crystal: 20 mm x 20 mm x 15 mm each
  G4double crystalHalfX = 10.375 * mm;
  G4double crystalHalfY = 10.375 * mm;
  G4double crystalHalfZ = 5 * mm;

  G4Box *solidCZT =
      new G4Box("solidCZT", crystalHalfX, crystalHalfY, crystalHalfZ);

  fScoringVolumeCZT = new G4LogicalVolume(solidCZT, CZT, "logicCZT");

  // Crystal center positions derived from measured pixel centers.
  // Negative-side pixels: -20.769 to -1.743 → center = (-20.769 + -1.743)/2 =
  // -11.256 mm Positive-side pixels:  +1.743 to +20.769 → center = +11.256 mm
  // Same for both X and Y axes.
  G4double cNeg = -11.256 * mm;
  G4double cPos = +11.256 * mm;

  G4double CZTPosZ = 0.0;

  // Layout (looking down +z):
  //   CZT_2 (-x,+y) | CZT_3 (+x,+y)
  //   ---------------+---------------
  //   CZT_0 (-x,-y) | CZT_1 (+x,-y)

  G4ThreeVector pos[4] = {
      G4ThreeVector(cNeg, cNeg, CZTPosZ), // CZT_0
      G4ThreeVector(cPos, cNeg, CZTPosZ), // CZT_1
      G4ThreeVector(cNeg, cPos, CZTPosZ), // CZT_2
      G4ThreeVector(cPos, cPos, CZTPosZ), // CZT_3
  };

  for (G4int i = 0; i < 4; i++) {
    new G4PVPlacement(0, pos[i], fScoringVolumeCZT, "physCZT", logicWorld,
                      false, i, true);
  }

  G4double singleVol =
      (2.0 * crystalHalfX) * (2.0 * crystalHalfY) * (2.0 * crystalHalfZ);
  G4double totalVol = 4.0 * singleVol;
  std::cout << "CZT total volume: " << totalVol / cm3 << " cm3 "
            << "(4 x " << singleVol / cm3 << " cm3)" << std::endl;

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  SensitiveDetector *CZTSD = new SensitiveDetector("CZT");
  fScoringVolumeCZT->SetSensitiveDetector(CZTSD);
}
