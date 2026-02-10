#include "construction.hh"
// Define the static member
DetectorConstruction::DetectorConstruction(G4double thick, G4double len_wid)
    : fThick(thick), fLen_wid(len_wid) {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume *DetectorConstruction::Construct() {
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

  G4Material *Ge = nist->FindOrBuildMaterial("G4_Ge");

  G4Box *solidWorld = new G4Box("solidWorld", 1.0 * m, 1.0 * m, 1.0 * m);
  //  make the germanium!

  G4double radius = 8 * mm;
  G4double halfHeight = 5 * mm;

  G4double len_wid = fLen_wid;
  G4double thick = fThick;

  //  make the germanium sample
  G4Box *solidGe = new G4Box("solidGe", thick, len_wid, len_wid);
  // G4Tubs *solidGe =
  // new G4Tubs("solidGe", 0., radius, halfHeight, 0 * deg, 360 * deg);

  // get the volume of the sample
  G4cout << "Volume of the Germanium target: "
         << solidGe->GetCubicVolume() / cm3 << " cm3." << G4endl;

  G4double airThickness = 2.5 * mm; // Thickness of the air layer around Ge
  //  make the air layer surrounding the germanium sample
  G4Box *solidAirLayer =
      new G4Box("solidAirLayer", (thick + airThickness),
                (len_wid + airThickness), (len_wid + airThickness));
  // G4Tubs *solidAirLayer =
  //   new G4Tubs("solidAir", 0., radius + airThickness,
  //            halfHeight + airThickness, 0 * deg, 360 * deg);

  G4LogicalVolume *logicWorld =
      new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

  G4VPhysicalVolume *physWorld = new G4PVPlacement(
      0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

  logicGe = new G4LogicalVolume(solidGe, Ge, "logicGe");
  logicAirLayer = new G4LogicalVolume(solidAirLayer, worldMat, "logicAirLayer");

  G4VisAttributes *GeVisAttr = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0));
  G4VisAttributes *airVisAttr =
      new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 0.5)); // semi-transparent red

  logicGe->SetVisAttributes(GeVisAttr);
  logicAirLayer->SetVisAttributes(airVisAttr);
  // Create a rotation matrix for the Ge sample
  G4RotationMatrix *rotGe = new G4RotationMatrix();
  rotGe->rotateY(0 * deg); // Example rotation around the Z-axis

  // Place the Air Layer in the world
  G4VPhysicalVolume *physAirLayer = new G4PVPlacement(
      0, G4ThreeVector(0., 0., len_wid + airThickness + 3. * cm), logicAirLayer,
      "physAirLayer", logicWorld, false, 0, true);

  // Place the Ge inside the Air Layer
  G4VPhysicalVolume *physGe =
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicGe, "physGe",
                        logicAirLayer, false, 0, true);
  // Germanium lattice information
  // G4LatticeManager gives physics processes access to lattices by volume
  G4LatticeManager *LM = G4LatticeManager::GetLatticeManager();
  G4LatticeLogical *GeLogical = LM->LoadLattice(Ge, "Ge");

  // G4LatticePhysical assigns G4LatticeLogical a physical orientation
  G4LatticePhysical *GePhysical =
      new G4LatticePhysical(GeLogical, physGe->GetFrameRotation());
  LM->RegisterLattice(physGe, GePhysical);

  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  SensitiveDetector *sensDet = new SensitiveDetector("Germanium");
  logicGe->SetSensitiveDetector(sensDet);

  SensitiveDetector *sensDetAir = new SensitiveDetector("Air");
  logicAirLayer->SetSensitiveDetector(sensDetAir);
}
