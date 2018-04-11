#pragma once
#include "BaseCommand.h"

class StateCommand : public BaseCommand {

public:

  StateCommand(const tstring& state);

  virtual ~StateCommand() override = default;

  virtual void execute(JtagBase& jtagBase) override;

private:

  tstring state;

  StateCommand(const StateCommand&) = delete;
  StateCommand(StateCommand&&) = delete;
  StateCommand& operator =(const StateCommand&) = delete;
  StateCommand& operator =(StateCommand&&) = delete;
};
