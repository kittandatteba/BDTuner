#include "stdafx.h"
#include "JtagBase.h"


JtagBase::JtagBase()
  : readBuff()
  , writeBuff()
  , index(0)
  , unmatch(0)
  , state(_T("")) {}


void JtagBase::reset() {

  for (uint32_t i = 0; i < 5; i++) {
    this->xferBit(0x00, 0x00, 0x00, 0x00, 0x01);
  }
  this->state = _T("RESET");
}


void JtagBase::transitState(const tstring& nextState, uint8_t tdi, uint8_t smask, uint8_t tdo, uint8_t mask) {

  while (this->state != nextState) {
    if (this->tmsZeroRouteStates.at(this->state).count(nextState) > 0) {
      this->xferBit(tdi, smask, tdo, mask, 0x00);
      this->state = this->nextState.at(this->state)[0];
    } else {
      this->xferBit(tdi, smask, tdo, mask, 0x01);
      this->state = this->nextState.at(this->state)[1];
    }
  }
}


void JtagBase::xferBit(uint8_t tdi, uint8_t smask, uint8_t tdo, uint8_t mask, uint8_t tms) {

  // (b7,b6,b5,b4,b3,b2,b1,b0) = (TCK2,TDO2,TMS2,TDI2,TCK1,TDO1,TMS1,TDI1)
  this->writeBuff[this->index] = 1 << 7 | tms << 5 | ((tdi & smask) << 4) | 0 << 3 | tms << 1 | (tdi & smask);
  this->readBuff[this->index] = tdo << 6 | mask;
  this->index++;

  if (this->index == USB_BUFSIZE) {
    this->flush();
  }
}


void JtagBase::flush() {

  if (this->index == 0) {
    return;
  }

  int32_t written = this->write(writeBuff, this->index);
  if (written != this->index) {
    throw std::runtime_error("write error");
  }

  uint8_t result[USB_BUFSIZE] = {};
  int32_t read = this->read(result, this->index);
  if (read != this->index) {
    throw std::runtime_error("read error");
  }

  for (uint32_t i = 0; i < this->index; i++) {
    if (this->readBuff[i] & 1) {
      // (b7,b6,b5,b4,b3,b2,b1,b0) = (TCK2,TDO2,TMS2,TDI2,TCK1,TDO1,TMS1,TDI1)
      if ((result[i] & 0x40) != (this->readBuff[i] & 0x40)) {
        this->unmatch++;
      }
    }
  }

  ::ZeroMemory(this->readBuff, sizeof(this->readBuff));
  ::ZeroMemory(this->writeBuff, sizeof(this->writeBuff));
  this->index = 0;
}
