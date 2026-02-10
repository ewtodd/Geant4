#include "construction.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {

  G4NistManager *nist = G4NistManager::Instance();

  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  // Create world volume

  G4Box *solidWorld = new G4Box("solidWorld", 0.1 * m, 0.1 * m, 0.1 * m);

  G4LogicalVolume *logicWorld =

      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

  G4VPhysicalVolume *physWorld = new G4PVPlacement(

      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

  // Define CZT material (Cd:Zn:Te = 1:1:1)
  G4Element *Cd = nist->FindOrBuildElement("Cd");
  G4Element *Zn = nist->FindOrBuildElement("Zn");
  G4Element *Te = nist->FindOrBuildElement("Te");

  G4double densityCZT = 5.78 * g / cm3; // Approximate density of CZT
  G4Material *CZT = new G4Material("CZT", densityCZT, 3);
  CZT->AddElement(Cd, 1);
  CZT->AddElement(Zn, 1);
  CZT->AddElement(Te, 1);

  // Create CZT detector - 19 cm³ crystal
  // Assuming proportional dimensions based on 2.25:2.25:4.0 ratio
  // Let the longest dimension be 'a', then: a × (2.25/4.0)a × (2.25/4.0)a = 19 c
  // ³ a³ × (2.25/4.0)² = 19 a³ × 0.31640625 = 19 a = ∛(19/0.31640625) ≈ 3.805 cm
  // 

  G4double ratio = 2.25 / 4.0;
  G4double longDim = std::pow(19.0 / (ratio * ratio), 1.0 / 3.0) * cm;
  G4double shortDim = longDim * ratio;

  G4double CZTHalfX = shortDim / 2.0; // ~1.07 cm
  G4double CZTHalfY = shortDim / 2.0; // ~1.07 cm
  G4double CZTHalfZ = longDim / 2.0;  // ~1.90 cm

  G4Box *solidCZT = new G4Box("CZT", CZTHalfX, CZTHalfY, CZTHalfZ);

  fScoringVolumeCZT = new G4LogicalVolume(solidCZT, CZT, "CZT");

  G4double CZTPosZ = 5 * cm;

  // Place CZT detector
  G4VPhysicalVolume *physCZT =

      new G4PVPlacement(0, G4ThreeVector(0, 0, CZTPosZ),

                        fScoringVolumeCZT, "CZT", logicWorld, false, 0, true);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {

  // CZT sensitive detector

  SensitiveDetector *CZTSD = new SensitiveDetector("CZT");

  fScoringVolumeCZT->SetSensitiveDetector(CZTSD);
}
