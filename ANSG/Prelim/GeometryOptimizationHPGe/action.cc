#include "action.hh"

ActionInitialization::ActionInitialization() : G4VUserActionInitialization() {}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::BuildForMaster() const {
  SetUserAction(new RunAction());
}

void ActionInitialization::Build() const {
  SetUserAction(new PrimaryGenerator());
  SetUserAction(new RunAction());
}
