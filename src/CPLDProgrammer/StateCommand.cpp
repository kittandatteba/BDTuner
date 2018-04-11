#include "stdafx.h"
#include "StateCommand.h"


StateCommand::StateCommand(const tstring& state)
  : BaseCommand()
  , state(state) {}


void StateCommand::execute(JtagBase& jtagBase) {

  jtagBase.transitState(this->state);
}
