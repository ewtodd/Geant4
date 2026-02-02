#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4Tubs.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VisAttributes.hh"
#include "cmath"
#include "detector.hh"

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
  DetectorConstruction();
  ~DetectorConstruction();

  G4LogicalVolume *GetScoringVolumeNaI() const { return fScoringVolumeNaI; }
  G4LogicalVolume *GetScoringVolumeEJ309() const { return fScoringVolumeEJ309; }

  virtual G4VPhysicalVolume *Construct();

private:
  G4LogicalVolume *fScoringVolumeNaI;
  G4LogicalVolume *fScoringVolumeEJ309;

  virtual void ConstructSDandField();

  // Material creation methods
  G4Material *CreateNaITl();
  G4Material *CreateEJ309();
  G4Material *CreateBoratedPolyethylene();
};

#endif
