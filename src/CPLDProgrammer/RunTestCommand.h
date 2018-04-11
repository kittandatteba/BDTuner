#pragma once
#include "BaseCommand.h"
class RunTestCommand : public BaseCommand {

public:

  RunTestCommand(const tstring& runState, const tstring& clock, const tstring& endState);

  virtual ~RunTestCommand() override = default;

  virtual void execute(JtagBase& jtagBase) override;

private:

  uint32_t clock;

  tstring runState;
  tstring endState;

  RunTestCommand(const RunTestCommand&) = delete;
  RunTestCommand(RunTestCommand&&) = delete;
  RunTestCommand& operator =(const RunTestCommand&) = delete;
  RunTestCommand& operator =(RunTestCommand&&) = delete;
};
