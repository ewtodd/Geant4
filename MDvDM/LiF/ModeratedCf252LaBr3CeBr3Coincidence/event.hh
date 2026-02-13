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

  void AddEdepLaBr3(G4double edep, G4double time);
  void AddEdepCeBr3(G4double edep, G4double time);

private:
  G4double fEdepLaBr3, fEdepCeBr3;
  G4double fTimeLaBr3, fTimeCeBr3;
};

#endif
