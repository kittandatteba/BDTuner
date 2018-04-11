#include "stdafx.h"
#include "ShiftCommand.h"


ShiftCommand::ShiftCommand(const tstring& type, const tstring& length, const tstring& tdi, const tstring& smask, const tstring& tdo, const tstring& mask)
  : BaseCommand()
  , isIr(::_totupper(type[0]) == _T('I'))
  , length(std::stoi(length))
  , tdi(tdi)
  , smask(smask)
  , tdo(tdo)
  , mask(mask) {}


void ShiftCommand::execute(JtagBase& jtagBase) {

  tstring startState, pathState, endState;

  // SMASKパラメータが存在しない場合は、以前に割り当てられたSMASK値を使用
  this->smask = jtagBase.putDefaultValue(JtagBase::smask, this->smask);

  // MASKパラメータが存在しない場合は、以前に割り当てられたMASK値を使用
  this->mask = jtagBase.putDefaultValue(JtagBase::mask, this->mask);

  if (this->isIr) {
    startState = _T("IRSHIFT");
    pathState = _T("IREXIT1");
    endState = jtagBase.getDefaultValue(JtagBase::endIrState);
  } else {
    startState = _T("DRSHIFT");
    pathState = _T("DREXIT1");
    endState = jtagBase.getDefaultValue(JtagBase::endDrState);
  }

  jtagBase.transitState(startState);

  auto index = tdi.length();
  uint8_t tdiHex, smaskHex, tdoHex, maskHex;
  for (uint32_t i = 0; i < this->length; i++) {

    if (i % 4 == 0) {
      index--;

      tdiHex = toHexValue(this->tdi[index]);
      smaskHex = 0x0F;
      if (!this->smask.empty()) {
        smaskHex = toHexValue(this->smask[index]);
      }

      tdoHex = 0x00;
      maskHex = 0x00;
      if (!this->tdo.empty()) {
        tdoHex = toHexValue(this->tdo[index]);
        maskHex = 0x0F;
        if (!this->mask.empty()) {
          maskHex = toHexValue(this->mask[index]);
        }
      }
    }

    if (i < this->length - 1) {
      jtagBase.xferBit(tdiHex & 0x01, smaskHex & 0x01, tdoHex & 0x01, maskHex & 0x01);
    } else {
      jtagBase.transitState(pathState, tdiHex & 0x01, smaskHex & 0x01, tdoHex & 0x01, maskHex & 0x01);
    }

    tdiHex >>= 1;
    smaskHex >>= 1;
    tdoHex >>= 1;
    maskHex >>= 1;
  }

  jtagBase.transitState(endState);
}