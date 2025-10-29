#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4Box.hh"
#include "G4Exception.hh"
#include "G4LatticeLogical.hh"
#include "G4LatticeManager.hh"
#include "G4LatticePhysical.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Tubs.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VisAttributes.hh"
#include "action.hh"
#include "cmath"
#include "detector.hh"

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
  DetectorConstruction(G4double thick, G4double len_wid);
  ~DetectorConstruction();

  virtual G4VPhysicalVolume *Construct();

private:
  G4double fThick;
  G4double fLen_wid;
  G4LogicalVolume *logicGe;
  G4LogicalVolume *logicAirLayer;
  virtual void ConstructSDandField();
};

#endif
