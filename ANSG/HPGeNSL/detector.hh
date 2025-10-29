#ifndef DETECTOR_HH
#define DETECTOR_HH

#include "G4AnalysisManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4VSensitiveDetector.hh"
#include "G4ios.hh"
#include "event.hh"

class SensitiveDetector : public G4VSensitiveDetector {
public:
  SensitiveDetector(G4String name);
  ~SensitiveDetector();

  virtual G4bool ProcessHits(G4Step *aStep, G4TouchableHistory *ROhist);
};

#endif
