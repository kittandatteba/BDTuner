#include "stdafx.h"
#include "RunTestCommand.h"


RunTestCommand::RunTestCommand(const tstring& runState, const tstring& clock, const tstring& endState)
  : BaseCommand()
  , runState(runState)
  , clock(std::stoi(clock))
  , endState(endState) {}


void RunTestCommand::execute(JtagBase& jtagBase) {

  // end_stateはrun_stateが指定された場合、新しいrun_stateがデフォルト
  jtagBase.putDefaultValue(JtagBase::endTestState, this->runState);

  // end_stateが指定されない場合、デフォルトの値を使用
  // end_stateが指定された場合、その値がデフォルト
  this->endState = jtagBase.putDefaultValue(JtagBase::endTestState, this->endState);

  // run_stateを指定しない場合、以前に指定された値に指定
  this->runState = jtagBase.putDefaultValue(JtagBase::runTestState, this->runState);

  jtagBase.transitState(this->runState);

  uint8_t tms = 0;
  if (this->runState == _T("RESET")) {
    tms = 1;
  }

  for (uint32_t i = 0; i < this->clock; i++) {
    jtagBase.xferBit(0, 0, 0, 0, tms);
  }

  jtagBase.transitState(this->endState);
}
