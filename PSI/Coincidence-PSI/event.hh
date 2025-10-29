#ifndef EVENT_HH
#define EVENT_HH

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4Gamma.hh"
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

  void AddEdepGe(G4double edep, G4double time);
  void AddEdepCdTe(G4double edep, G4double time);
  void AddEdepNaI(G4double edep, G4double time);

private:
  G4double fEdepGe, fEdepCdTe, fEdepNaI;
  G4double fTimeGe, fTimeCdTe, fTimeNaI;
};

#endif
