#pragma once
#include "JtagBase.h"

class BaseCommand {

public:

  BaseCommand() = default;

  virtual ~BaseCommand() = default;

  virtual void execute(JtagBase& jtagBase) {}

private:
  BaseCommand(const BaseCommand&) = delete;
  BaseCommand(BaseCommand&&) = delete;
  BaseCommand& operator =(const BaseCommand&) = delete;
  BaseCommand& operator =(BaseCommand&&) = delete;
};
