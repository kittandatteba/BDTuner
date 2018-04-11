#include "stdafx.h"
#include "EndCommand.h"


EndCommand::EndCommand(const tstring& type, const tstring& state)
  : BaseCommand()
  , isIr(::_totupper(type[0]) == _T('I'))
  , state(state) {}


void EndCommand::execute(JtagBase& jtagBase) {

  if (this->isIr) {
    jtagBase.putDefaultValue(JtagBase::endIrState, this->state);
  } else {
    jtagBase.putDefaultValue(JtagBase::endDrState, this->state);
  }
}
