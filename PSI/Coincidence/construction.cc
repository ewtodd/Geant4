#include "construction.hh"

#include "G4UnitsTable.hh"

DetectorConstruction::DetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {

  G4NistManager *nist = G4NistManager::Instance();

  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  // Create world volume

  G4Box *solidWorld = new G4Box("solidWorld", 0.2 * m, 0.2 * m, 0.3 * m);

  G4LogicalVolume *logicWorld =

      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

  G4VPhysicalVolume *physWorld = new G4PVPlacement(

      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

  // Define lead wall material and dimensions

  G4Material *leadMaterial = nist->FindOrBuildMaterial("G4_AIR");

  // Calculate position offset for detectors (after the lead wall)

  G4double detectorStartZ = 5 * cm;

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

  // Define Germanium material for the detector in beamline

  G4Material *Ge = nist->FindOrBuildMaterial("G4_Ge");

  // Create Germanium detector - 1 inch diameter by 2mm thick cylinder

  // The cylinder needs to be oriented so the disc faces the beam

  // G4Tubs has its axis along z by default, so we rotate it 90 degrees around Y

  G4double GeRadius = 1.0 * inch / 2;

  G4double GeHalfThickness = 1.0 * mm; // 2mm thick, so half is 1mm

  G4Tubs *solidGe =

      new G4Tubs("Ge", 0, GeRadius, GeHalfThickness, 0, 360 * deg);

  fScoringVolumeGe = new G4LogicalVolume(solidGe, Ge, "Ge");

  // Position Germanium detector in beamline

  // Place it further from lead wall to ensure clearance

  G4double GePosZ = detectorStartZ + 5.0 * cm;

  // Rotation matrix to orient Ge disc perpendicular to beam

  // Rotate 90 degrees around Y axis so disc faces along z-axis

  G4RotationMatrix *rotGe = new G4RotationMatrix();

  rotGe->rotateX(90 * deg);

  // Place Germanium detector (disc facing the beam, perpendicular to z-axis)

  G4VPhysicalVolume *physGe =

      new G4PVPlacement(rotGe, G4ThreeVector(0, 0, GePosZ), fScoringVolumeGe,

                        "Ge", logicWorld, false, 0, true);

  G4double CdTePosY = -GeHalfThickness - CdTeHalfHeight - 0.5 * cm;

  G4double CdTePosZ = GePosZ; // same z position as Ge center

  G4RotationMatrix *rotCdTe = new G4RotationMatrix();

  rotCdTe->rotateX(90 * deg);

  // Place CdTe detector below the beamline

  G4VPhysicalVolume *physCdTe =

      new G4PVPlacement(rotCdTe, G4ThreeVector(0, CdTePosY, CdTePosZ),

                        fScoringVolumeCdTe, "CdTe", logicWorld, false, 0, true);

  // Create cylindrical lead shielding around CdTe with 1 cm thick walls

  // CdTe is rotated, so it's oriented as a disc in the x-z plane (axis along y)

  // The shield needs to be a cylinder with axis along y, matching the CdTe
  // orientation

  G4double leadShieldThickness =0.4 * cm;

  // Inner radius: enough to contain the CdTe disc radius plus small clearance

  G4double shieldInnerRadius = CdTeRadius + 0.2 * mm;

  // Outer radius: inner radius + 1 cm thickness

  G4double shieldOuterRadius = shieldInnerRadius + leadShieldThickness;

  // Half-height along y-axis: enough to contain CdTe thickness plus clearance

  G4double shieldInnerHalfHeight = CdTeHalfHeight + 0.2 * mm;

  // Outer half-height: inner + 1 cm thickness on each end

  G4double shieldOuterHalfHeight = shieldInnerHalfHeight + 0.4 * cm;

  // Create outer cylinder

  G4Tubs *solidLeadOuter =

      new G4Tubs("LeadShieldOuter", 0., shieldOuterRadius,
                 shieldOuterHalfHeight, 0 * deg, 360 * deg);

  // Create inner cavity (to be subtracted)

  G4Tubs *solidLeadInner =

      new G4Tubs("LeadShieldInner", 0., shieldInnerRadius,
                 shieldInnerHalfHeight, 0 * deg, 360 * deg);

  // Create the hollow cylinder by subtraction

  G4SubtractionSolid *leadShield =

      new G4SubtractionSolid("LeadShield", solidLeadOuter, solidLeadInner);

  G4LogicalVolume *logicLeadShield =

      new G4LogicalVolume(leadShield, leadMaterial, "LeadShield");

  // Set visual attributes for lead shield

  G4VisAttributes *leadShieldVisAttr =

      new G4VisAttributes(G4Colour(0.4, 0.4, 0.4)); // Dark gray

  logicLeadShield->SetVisAttributes(leadShieldVisAttr);

  // Place lead shield at same position and orientation as CdTe

  // Need to rotate the shield the same way as CdTe

  G4RotationMatrix *rotLeadShield = new G4RotationMatrix();

  rotLeadShield->rotateX(90 * deg);

  G4VPhysicalVolume *physLeadShield = new G4PVPlacement(

      rotLeadShield, G4ThreeVector(0, CdTePosY, CdTePosZ), logicLeadShield,
      "LeadShield",

      logicWorld, false, 0, true);

  // Define aluminum material for the layer between Ge and NaI

  G4Material *Al = nist->FindOrBuildMaterial("G4_Al");

  // Create 5mm thick aluminum disc with same diameter as Ge (1 inch)

  G4double AlHalfThickness = 2.5 * mm; // 5mm thick, so half is 2.5mm

  G4Tubs *solidAl =

      new G4Tubs("Al", 0, 6 * GeRadius, AlHalfThickness, 0, 360 * deg);

  G4LogicalVolume *logicAl = new G4LogicalVolume(solidAl, Al, "Al");

  // Set visual attributes for aluminum

  G4VisAttributes *AlVis =

      new G4VisAttributes(G4Colour(0.7, 0.7, 0.7)); // Light gray

  logicAl->SetVisAttributes(AlVis);

  G4double AlPosY = GeRadius + 0.1 * mm + AlHalfThickness;

  // Rotate aluminum to be horizontal (same as Ge)

  G4RotationMatrix *rotAl = new G4RotationMatrix();

  rotAl->rotateX(90 * deg);

  G4VPhysicalVolume *physAl =

      new G4PVPlacement(rotAl, G4ThreeVector(0, AlPosY, GePosZ), logicAl, "Al",

                        logicWorld, false, 0, true);

  G4Material *NaI = nist->FindOrBuildMaterial("G4_SODIUM_IODIDE");

  G4double NaIRadius = 3.0 * inch / 2;

  G4double NaIHalfHeight = 3.0 * inch / 2;

  G4Tubs *solidNaI =

      new G4Tubs("NaI", 0, NaIRadius, NaIHalfHeight, 0, 360 * deg);

  fScoringVolumeNaI = new G4LogicalVolume(solidNaI, NaI, "NaI");

  G4double NaIPosY = AlPosY + AlHalfThickness + NaIHalfHeight;

  G4double NaIPosZ = GePosZ; // Centered at same Z as Ge

  // Rotate NaI 90 degrees around X axis for vertical orientation

  G4RotationMatrix *rotNaI = new G4RotationMatrix();

  rotNaI->rotateX(90 * deg);

  G4VPhysicalVolume *physNaI =

      new G4PVPlacement(rotNaI, G4ThreeVector(0, NaIPosY, NaIPosZ),

                        fScoringVolumeNaI, "NaI", logicWorld, false, 0, true);

  // Visualization attributes

  G4VisAttributes *GeVis =

      new G4VisAttributes(G4Colour(0.7, 0.7, 0.9)); // Light blue

  fScoringVolumeGe->SetVisAttributes(GeVis);

  G4VisAttributes *CdTeVis =

      new G4VisAttributes(G4Colour(0.9, 0.5, 0.2)); // Orange

  fScoringVolumeCdTe->SetVisAttributes(CdTeVis);

  G4VisAttributes *NaIVis =

      new G4VisAttributes(G4Colour(0.2, 0.8, 0.2)); // Green

  fScoringVolumeNaI->SetVisAttributes(NaIVis);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {

  // Germanium sensitive detector

  SensitiveDetector *GeSD = new SensitiveDetector("Ge");

  fScoringVolumeGe->SetSensitiveDetector(GeSD);

  // CdTe sensitive detector

  SensitiveDetector *CdTeSD = new SensitiveDetector("CdTe");

  fScoringVolumeCdTe->SetSensitiveDetector(CdTeSD);

  // NaI sensitive detector

  SensitiveDetector *NaISD = new SensitiveDetector("NaI");

  fScoringVolumeNaI->SetSensitiveDetector(NaISD);
}
