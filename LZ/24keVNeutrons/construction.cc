#include "construction.hh"
#include "G4UnitsTable.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4Material *DetectorConstruction::CreateNaITl() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *NaITl = new G4Material("NaITl", 3.667 * g / cm3, 3);
  NaITl->AddElement(nist->FindOrBuildElement("Na"), 0.152956);
  NaITl->AddElement(nist->FindOrBuildElement("I"), 0.844324);
  NaITl->AddElement(nist->FindOrBuildElement("Tl"), 0.002720);
  return NaITl;
}

G4Material *DetectorConstruction::CreateEJ309() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *EJ309 = new G4Material("EJ309", 0.959 * g / cm3, 2);
  EJ309->AddElement(nist->FindOrBuildElement("H"), 0.555);
  EJ309->AddElement(nist->FindOrBuildElement("C"), 0.445);
  return EJ309;
}

G4Material *DetectorConstruction::CreateBoratedPolyethylene() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *BoratedPoly = new G4Material("BoratedPoly", 0.95 * g / cm3, 3);
  BoratedPoly->AddElement(nist->FindOrBuildElement("H"), 0.116);
  BoratedPoly->AddElement(nist->FindOrBuildElement("C"), 0.834);
  BoratedPoly->AddElement(nist->FindOrBuildElement("B"), 0.050);
  return BoratedPoly;
}

G4VPhysicalVolume *DetectorConstruction::Construct() {
  G4NistManager *nist = G4NistManager::Instance();

  // Create materials
  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");
  G4Material *ironMat = nist->FindOrBuildMaterial("G4_Fe");
  G4Material *leadMat = nist->FindOrBuildMaterial("G4_Pb");
  G4Material *NaITl = CreateNaITl();
  G4Material *EJ309 = CreateEJ309();
  G4Material *BoratedPoly = CreateBoratedPolyethylene();

  // World volume - enlarged to fit all components
  G4Box *solidWorld = new G4Box("solidWorld", 0.5 * m, 0.5 * m, 1.0 * m);
  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");
  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

  // ========== GEOMETRY SETUP ==========
  // Source is at origin (0,0,0)

  // Dimensions
  G4double inch = 2.54 * cm;

  // Borated polyethylene layer
  G4double boratedPolyThickness = 1.0 * inch;

  // NaI(Tl) surrounding the source
  G4double NaIInnerRadius = boratedPolyThickness + 0.1 * cm;
  G4double NaIOuterRadius = NaIInnerRadius + 3.0 * inch;
  G4double NaIHalfHeight = 6.0 * inch;

  // Iron filter - positioned after NaI with clearance
  G4double ironFilterThickness = 7.5 * inch;
  G4double ironFilterHalfX = 25.0 * cm;
  G4double ironFilterHalfY = 25.0 * cm;
  G4double ironFilterPosZ = NaIHalfHeight + ironFilterThickness / 2 + 2.0 * cm;

  // EJ-309 detector - after iron filter with clearance
  G4double EJ309Radius = 2.0 * inch;
  G4double EJ309HalfHeight = 2.0 * inch;
  G4double EJ309PosZ =
      ironFilterPosZ + ironFilterThickness / 2 + EJ309HalfHeight + 2.0 * cm;

  // Lead shielding around EJ-309
  G4double leadShieldThickness = 2.0 * inch;
  G4double leadShieldInnerRadius = EJ309Radius + 0.5 * cm;
  G4double leadShieldOuterRadius = leadShieldInnerRadius + leadShieldThickness;
  G4double leadShieldHalfHeight = EJ309HalfHeight + 2.0 * cm;

  // ========== 1. BORATED POLYETHYLENE (around source) ==========
  G4Tubs *solidBoratedPoly = new G4Tubs("BoratedPoly", 0, boratedPolyThickness,
                                        NaIHalfHeight, 0, 360 * deg);

  G4LogicalVolume *logicBoratedPoly =
      new G4LogicalVolume(solidBoratedPoly, BoratedPoly, "BoratedPoly");

  G4VisAttributes *boratedPolyVis =
      new G4VisAttributes(G4Colour(0.8, 0.9, 1.0));
  logicBoratedPoly->SetVisAttributes(boratedPolyVis);

  new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicBoratedPoly, "BoratedPoly",
                    logicWorld, false, 0, true);

  // ========== 2. NaI(Tl) DETECTOR (cylindrical shell around source) ==========
  G4Tubs *solidNaI = new G4Tubs("NaI", NaIInnerRadius, NaIOuterRadius,
                                NaIHalfHeight, 0, 360 * deg);

  fScoringVolumeNaI = new G4LogicalVolume(solidNaI, NaITl, "NaI");

  G4VisAttributes *NaIVis = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0));
  fScoringVolumeNaI->SetVisAttributes(NaIVis);

  new G4PVPlacement(0, G4ThreeVector(0, 0, 0), fScoringVolumeNaI, "NaI",
                    logicWorld, false, 0, true);

  // ========== 3. IRON FILTER (for neutrons) ==========
  G4Box *solidIronFilter = new G4Box("IronFilter", ironFilterHalfX,
                                     ironFilterHalfY, ironFilterThickness / 2);

  G4LogicalVolume *logicIronFilter =
      new G4LogicalVolume(solidIronFilter, ironMat, "IronFilter");

  G4VisAttributes *ironVis = new G4VisAttributes(G4Colour(0.6, 0.6, 0.6));
  logicIronFilter->SetVisAttributes(ironVis);

  new G4PVPlacement(0, G4ThreeVector(0, 0, ironFilterPosZ), logicIronFilter,
                    "IronFilter", logicWorld, false, 0, true);

  // ========== 4. LEAD SHIELDING (cylindrical, around EJ-309 position)
  // ==========
  G4Tubs *solidLeadShield =
      new G4Tubs("LeadShield", leadShieldInnerRadius, leadShieldOuterRadius,
                 leadShieldHalfHeight, 0, 360 * deg);

  G4LogicalVolume *logicLeadShield =
      new G4LogicalVolume(solidLeadShield, leadMat, "LeadShield");

  G4VisAttributes *leadVis = new G4VisAttributes(G4Colour(0.3, 0.3, 0.3));
  logicLeadShield->SetVisAttributes(leadVis);

  new G4PVPlacement(0, G4ThreeVector(0, 0, EJ309PosZ), logicLeadShield,
                    "LeadShield", logicWorld, false, 0, true);

  // ========== 5. EJ-309 DETECTOR (inside lead shield) ==========
  G4Tubs *solidEJ309 =
      new G4Tubs("EJ309", 0, EJ309Radius, EJ309HalfHeight, 0, 360 * deg);

  fScoringVolumeEJ309 = new G4LogicalVolume(solidEJ309, EJ309, "EJ309");

  G4VisAttributes *EJ309Vis = new G4VisAttributes(G4Colour(0.0, 1.0, 0.5));
  fScoringVolumeEJ309->SetVisAttributes(EJ309Vis);

  new G4PVPlacement(0, G4ThreeVector(0, 0, EJ309PosZ), fScoringVolumeEJ309,
                    "EJ309", logicWorld, false, 0, true);

  // Print geometry information
  G4cout << "\n=== Geometry Summary ===" << G4endl;
  G4cout << "NaI(Tl) center: (0, 0, 0)" << G4endl;
  G4cout << "Iron filter Z position: " << ironFilterPosZ / cm << " cm"
         << G4endl;
  G4cout << "EJ-309 Z position: " << EJ309PosZ / cm << " cm" << G4endl;
  G4cout << "========================\n" << G4endl;

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  // NaI(Tl) sensitive detector
  SensitiveDetector *NaISD = new SensitiveDetector("NaI");
  fScoringVolumeNaI->SetSensitiveDetector(NaISD);

  // EJ-309 sensitive detector
  SensitiveDetector *EJ309SD = new SensitiveDetector("EJ309");
  fScoringVolumeEJ309->SetSensitiveDetector(EJ309SD);
}
