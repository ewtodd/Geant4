#ifndef EVENT_HH
#define EVENT_HH

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4Gamma.hh"
#include "G4Neutron.hh"
#include "G4SystemOfUnits.hh"
#include "G4UserEventAction.hh"
#include "Randomize.hh"
#include "run.hh"

class EventAction : public G4UserEventAction {
public:
  EventAction(RunAction *);
  ~EventAction();

  virtual void BeginOfEventAction(const G4Event *);
  virtual void EndOfEventAction(const G4Event *);

  // NaI(Tl) detector
  void AddEdepNaI(G4double edep, G4double time, G4bool isNeutron);

  // EJ-309 detector
  void AddEdepEJ309(G4double edep, G4double time, G4bool isNeutron);

private:
  // NaI(Tl) - Gamma deposits
  G4double fEdepNaI_Gamma;
  G4double fTimeNaI_Gamma;

  // NaI(Tl) - Neutron deposits
  G4double fEdepNaI_Neutron;
  G4double fTimeNaI_Neutron;

  // EJ-309 - Gamma deposits
  G4double fEdepEJ309_Gamma;
  G4double fTimeEJ309_Gamma;

  // EJ-309 - Neutron deposits
  G4double fEdepEJ309_Neutron;
  G4double fTimeEJ309_Neutron;
};

#endif
