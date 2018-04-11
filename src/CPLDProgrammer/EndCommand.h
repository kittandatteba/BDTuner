#pragma once
#include "BaseCommand.h"

class EndCommand : public BaseCommand {

public:

  EndCommand(const tstring& type, const tstring& state);

  virtual ~EndCommand() override = default;

  virtual void execute(JtagBase& jtagBase) override;

private:
  bool isIr;
  tstring state;

  EndCommand(const EndCommand&) = delete;
  EndCommand(EndCommand&&) = delete;
  EndCommand& operator =(const EndCommand&) = delete;
  EndCommand& operator =(EndCommand&&) = delete;
};

