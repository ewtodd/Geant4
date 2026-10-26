#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4Box.hh"
#include "G4GenericMessenger.hh"
#include "G4GeometryManager.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RotationMatrix.hh"
#include "G4RunManager.hh"
#include "G4SolidStore.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VisAttributes.hh"
#include "cmath"
#include "detector.hh"

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
  DetectorConstruction();
  ~DetectorConstruction();

  virtual G4VPhysicalVolume *Construct();

  // /moderator/thickness: rebuilds the geometry before the next run.
  void SetModeratorThickness(G4double thickness);
  G4double GetModeratorThickness() const { return fModeratorThickness; }

private:
  G4LogicalVolume *fScoringVolumeWrapPre;
  G4LogicalVolume *fScoringVolumeWrapPost;
  G4double fModeratorThickness;
  G4GenericMessenger *fMessenger;
  virtual void ConstructSDandField();
};

#endif
