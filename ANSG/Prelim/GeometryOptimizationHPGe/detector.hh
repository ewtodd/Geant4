#ifndef DETECTOR_HH
#define DETECTOR_HH

#include "G4AnalysisManager.hh"
#include "G4Gamma.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VSensitiveDetector.hh"
#include "construction.hh"

class SensitiveDetector : public G4VSensitiveDetector {
public:
  SensitiveDetector(G4String name);
  ~SensitiveDetector();

private:
  virtual G4bool ProcessHits(G4Step *, G4TouchableHistory *);
};

#endif
