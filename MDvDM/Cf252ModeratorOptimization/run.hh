#ifndef RUN_HH
#define RUN_HH

#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4Threading.hh"
#include "G4UserRunAction.hh"
#include "OutputMerger.hh"
#include "construction.hh"

class RunAction : public G4UserRunAction {
public:
  RunAction();
  ~RunAction();

  virtual void BeginOfRunAction(const G4Run *);
  virtual void EndOfRunAction(const G4Run *);

private:
  G4String OutputStem() const;
};

#endif
