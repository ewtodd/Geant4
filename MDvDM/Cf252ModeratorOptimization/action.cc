#include "action.hh"

ActionInitialization::ActionInitialization() {}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::Build() const {
  PrimaryGenerator *generator = new PrimaryGenerator();
  SetUserAction(generator);
  RunAction *runAction = new RunAction();
  SetUserAction(runAction);
}

void ActionInitialization::BuildForMaster() const {
  RunAction *runAction = new RunAction();
  SetUserAction(runAction);
}
