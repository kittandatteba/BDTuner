#pragma once
#include "BaseCommand.h"

class ShiftCommand : public BaseCommand {

public:

  ShiftCommand(const tstring& type, const tstring& length, const tstring& tdi, const tstring& smask, const tstring& tdo, const tstring& mask);

  virtual ~ShiftCommand() override = default;

  virtual void execute(JtagBase& jtagBase) override;

private:

  bool isIr;
  uint32_t length;
  tstring tdi;
  tstring tdo;
  tstring smask;
  tstring mask;

  inline static uint8_t toHexValue(TCHAR value) {
    const tstring hexValue = _T("0123456789ABCDEF");
    return static_cast<uint8_t>(hexValue.find(::_totupper(value)));
  }

  ShiftCommand(const ShiftCommand&) = delete;
  ShiftCommand(ShiftCommand&&) = delete;
  ShiftCommand& operator =(const ShiftCommand&) = delete;
  ShiftCommand& operator =(ShiftCommand&&) = delete;
};
