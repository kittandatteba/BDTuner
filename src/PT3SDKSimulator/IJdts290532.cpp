#include "stdafx.h"
#include <cassert>
#include "InternalException.h"
#include "II2C.h"
#include "IJdts290532.h"

namespace EARTH {
namespace PT {


IJdts290532::IJdts290532(II2C *i2CInterface)
  :i2CInterface(i2CInterface) {}


void IJdts290532::setFrequency(uint32 channel, sint32 offset) {
  mChannel = channel;
  mOffset = offset;
}

void IJdts290532::getFrequency(uint32 *channel, sint32 *offset) const noexcept {
  *channel = mChannel;
  *offset = mOffset;
}


void IJdts290532::setTunerSleep(bool sleep) {
  mSleep = sleep;
}


void IJdts290532::getTunerSleep(bool *sleep) const noexcept {
  *sleep = mSleep;
}


void IJdts290532::writeReg(uint8 addr_reg, uint8 *data,
  uint32 len, uint8 addr_demod, uint8 addr_tuner) const {

  uint32 tlen;
  uint8 wbuf[II2C::I2C_MSG_MAX];
  uint8 wlen = 0;

  if (addr_tuner != 0xff) {
    // チューナーのレジスタアクセス
    wbuf[wlen++] = PASS_THROUGH;
    wbuf[wlen++] = addr_tuner << 1;
  }

  wbuf[wlen++] = addr_reg;

  assert(wlen + len <= II2C::I2C_MSG_MAX);

  for (uint32 i = 0; i < len; i++) {
    wbuf[wlen++] = data[i];
  }

  II2C::Message msg = { addr_demod, II2C::Operation::WRITE, wlen, wbuf };

  try {
    tlen = i2CInterface->xfer(&msg, 1);
  } catch (...) {
    throw InternalException(STATUS_I2C_ERROR, "I2C transfer fail.");
  }

  if (tlen != wlen) {
    TRACE_F(_T("I2C transfer fail.")
      << _T(" tlen=") << std::dec << tlen
      << _T(" wlen=") << std::dec << wlen);
    throw InternalException(STATUS_I2C_ERROR, "I2C transfer fail.");
  }
}


void IJdts290532::readReg(uint8 addr_reg, uint8 *data,
  uint32 len, uint8 addr_demod, uint8 addr_tuner) const {

  uint32 tlen;
  uint8 rlen = 0;

  assert(len <= II2C::I2C_MSG_MAX);
  rlen = static_cast<uint8>(len);


  try {
    if (addr_tuner != 0xff) {
      // チューナーのレジスタアクセス
      uint8 wbuf1[3] = { PASS_THROUGH, static_cast<uint8>(addr_tuner << 1), addr_reg };
      uint8 wbuf2[2] = { PASS_THROUGH, static_cast<uint8>((addr_tuner << 1) | 0x01) };

      II2C::Message	msg[3] = {
        { addr_demod, II2C::Operation::WRITE, 3, wbuf1 },
        { addr_demod, II2C::Operation::WRITE, 2, wbuf2 },
        { addr_demod, II2C::Operation::READ, rlen, data }
      };
      tlen = i2CInterface->xfer(msg, 3);
    } else {
      // 復調ICのレジスタアクセス
      uint8 wbuf[1] = { addr_reg };

      II2C::Message	msg[2] = {
        { addr_demod, II2C::Operation::WRITE, 1, wbuf },
        { addr_demod, II2C::Operation::READ, rlen, data }
      };
      tlen = i2CInterface->xfer(msg, 2);
    }
  } catch (...) {
    throw InternalException(STATUS_I2C_ERROR, "I2C transfer fail.");
  }

  if (tlen != rlen) {
    TRACE_F(_T("I2C transfer fail.")
      << _T(" tlen=") << std::dec << tlen
      << _T(" rlen=") << std::dec << rlen);
    throw InternalException(STATUS_I2C_ERROR, "I2C transfer fail.");
  }
}


}
}
