#include "stdafx.h"
#include "RunTestCommand.h"


RunTestCommand::RunTestCommand(const tstring& runState, const tstring& clock, const tstring& endState)
  : BaseCommand()
  , runState(runState)
  , clock(std::stoi(clock))
  , endState(endState) {}


void RunTestCommand::execute(JtagBase& jtagBase) {

  // end_state��run_state���w�肳�ꂽ�ꍇ�A�V����run_state���f�t�H���g
  jtagBase.putDefaultValue(JtagBase::endTestState, this->runState);

  // end_state���w�肳��Ȃ��ꍇ�A�f�t�H���g�̒l���g�p
  // end_state���w�肳�ꂽ�ꍇ�A���̒l���f�t�H���g
  this->endState = jtagBase.putDefaultValue(JtagBase::endTestState, this->endState);

  // run_state���w�肵�Ȃ��ꍇ�A�ȑO�Ɏw�肳�ꂽ�l�Ɏw��
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
