#include "stdafx.h"
#include "InternalException.h"
#include "Jdts290532S.h"

namespace EARTH {
namespace PT {


Jdts290532S::Jdts290532S(II2C* i2CInterface)
  :IJdts290532(i2CInterface) {}


void Jdts290532S::initTuner() const {

  uint8 reg_val[8];
  initReg(reg_val, 0, sizeof(reg_val));

  // ���W�X�^�ݒ�
  writeReg(0x00, reg_val, sizeof(reg_val), ADDR_DEMOD, ADDR_TUNER);
}


void Jdts290532S::initDemod() const {

  uint8 reg_val[] = {
    // �A�h���X, �l
    0x01, 0x90,	// [7]psksyrst = 1(PSK�V�X�e�����Z�b�g)
    0x06, 0x40,	// [6]iqch = 1(IQ����)
    0x0c, 0x59,	// PSK�N���b�N���g���I�t�Z�b�g�␳(4MHz IF(XT=4MHz)���[�h)
    0x0d, 0xf2,	// PSK�N���b�N���g���I�t�Z�b�g�␳(4MHz IF(XT=4MHz)���[�h)
    0x0e, 0x50,	// PSK�L�����AAFC ���[�v�����␳(4MHz IF(XT=4MHz)���[�h)
    0x85, 0x7a,	// ??
    0xa3, 0xf7,	// [7]anuval = 1(Null�����ꂽ�p�P�b�g��Valid�M���𗧂ĂȂ�)
    0x8e, 0x26	// [5]dvaloff = 1(�p���e�B���Ԃ̃f�[�^��"0"�ɂ���)
                // [2]pkstop = 1(�p���e�B���Ԃ̓N���b�N���~�߂�(188 �o�C�g��))
                // [1]nuval = 1(Null�p�P�b�g��Valid�M���𗧂ĂȂ�)
  };

  for (int i = 0; i < sizeof(reg_val); i += 2) {
    writeReg(reg_val[i], &reg_val[i + 1], 1, ADDR_DEMOD);
  }

}


void Jdts290532S::setFrequency(uint32 channel, sint32 offset) {

  IJdts290532::setFrequency(channel, offset);

  // ����IC��AGC�o�̓I�t�ݒ�
  setAgcAuto(false);

  // �`�����l���ԍ������g���ɕϊ�
  uint32 freq_khz = getChannelFrequency(channel, offset);

  // �`���[�j���O
  tuneRf(freq_khz);

  // ���b�N��Ԃ̎擾
  bool lock;
  int retry = 0;
  while (true) {
    getTunerLockStatus(&lock);

    if (lock) break;

    if (retry++ < RETRY_CNT) {
      Sleep(RETRY_WAIT);
    } else {
      TRACE_F(_T("Retry over get tuner lock status.") << _T(" lock=") << std::dec << lock);

      break;
    }
  }

  // ����IC��AGC�o�̓I���ݒ�
  setAgcAuto(true);
}


void Jdts290532S::getFrequency(uint32 * channel, sint32 * offset) const noexcept {

  IJdts290532::getFrequency(channel, offset);
}


void Jdts290532S::setTunerSleep(bool sleep) {


  IJdts290532::setTunerSleep(sleep);

  uint8 reg_val[1];

  if (sleep) {
    // ����IC��AGC�o�̓I�t�ݒ�
    setAgcAuto(false);

    // �`���[�i�̃X���[�v�ݒ�
    initReg(reg_val, 0, sizeof(reg_val));
    setField(reg_val[0], 0, 3, 0x00);
    writeReg(0x00, reg_val, sizeof(reg_val), ADDR_DEMOD, ADDR_TUNER);

    // ����IC�̃X���[�v�ݒ�
    reg_val[0] = 0x01;
    writeReg(0x17, reg_val, sizeof(reg_val), ADDR_DEMOD);

  } else {
    // ����IC�̃E�F�C�N�A�b�v�ݒ�
    reg_val[0] = 0x00;
    writeReg(0x17, reg_val, sizeof(reg_val), ADDR_DEMOD);

    // �`���[�i�̃E�F�C�N�A�b�v�ݒ�
    initReg(reg_val, 0, sizeof(reg_val));
    setField(reg_val[0], 0, 3, 0x07);
    writeReg(0x00, reg_val, sizeof(reg_val), ADDR_DEMOD, ADDR_TUNER);
  }
}


void Jdts290532S::getTunerSleep(bool * sleep) const noexcept {

  IJdts290532::getTunerSleep(sleep);
}


void Jdts290532S::setId(uint32 id) const {

  // ��ʁE���ʃo�C�g�̕���
  uint8 c_id[2] = { (id >> 8) & 0xff, id & 0xff };

  // ���W�X�^iits�ւ̏�������
  writeReg(0x8f, c_id, 2, ADDR_DEMOD);

}


void Jdts290532S::getId(uint32* id) const {

  // ���W�X�^tsido�̓ǂݏo��
  uint8 c_id[2];
  readReg(0xe6, c_id, 2, ADDR_DEMOD);

  // ��ʁE���ʃo�C�g�̌���
  *id = (c_id[0] << 8) | c_id[1];
}


void Jdts290532S::getCn(uint32* cn100) const {

  // ���W�X�^cnmc�̓ǂݏo��
  uint8 buf[2];
  readReg(0xbc, buf, 2, ADDR_DEMOD);

  // ��ʁE���ʃo�C�g�̌���
  uint32 cnmc = (buf[0] << 8) | buf[1];

  if (cnmc < 3000) {
    *cn100 = 0;
    return;
  }

  // C/N�̌v�Z
  // cn100 = -163.46 * P ^ 5 + 1434.1 * P ^ 4 - 5025.9 * P ^ 3
  //			+ 8897.7 * P ^ 2 - 8956.5 * P + 5885.7
  // (P = sqrt(cnmc - 3000) / 64)
  double p = std::sqrt(cnmc - 3000.0) / 64.0;

  double p2 = std::pow(p, 2);
  double p3 = std::pow(p, 3);
  double p4 = std::pow(p, 4);
  double p5 = std::pow(p, 5);

  p = -163.46*p5 + 1434.1*p4 - 5025.9*p3 + 8897.7*p2 - 8956.5*p + 5885.7;
  if (p < 0) {
    p = 0;
  }

  *cn100 = static_cast<uint32>(p + 0.5);
}


void Jdts290532S::getAgc(uint8 *agc, uint8 *max_agc) const {

  // ���W�X�^agcmc�̓ǂݏo��
  uint8 data;
  readReg(0xba, &data, 1, ADDR_DEMOD);

  *agc = getField(data, 0, 7);
  *max_agc = 127;
}


void Jdts290532S::getTmcc(Device::TmccS* tmcc) const {

  // ���W�X�^tmcerr,emgcy,tmcrev�̓ǂݏo��
  uint8 data;
  readReg(0xc3, &data, 1, ADDR_DEMOD);

  // TMCC�����G���[����̏ꍇ�ُ͈�I��
  if (getField(data, 4, 1) != 0x00U) {
    TRACE_F(_T("Invalid tmcerr value.")
      << _T(" tmcerr=") << std::dec << getField(data, 4, 1));
    throw InternalException(STATUS_GENERAL_ERROR, "Invalid tmcerr value.");
  }

  tmcc->Emergency = getField(data, 2, 1);
  tmcc->ExtFlag = getField(data, 1, 1);

  static const uint8 BASE = 0xc5;
  static const uint8 SIZE = 0xe5 - BASE + 1;
  uint8	buf[SIZE];
  uint8 index;

  // TMCC��񃂃j�^���W�X�^�̓ǂݏo��
  readReg(BASE, buf, SIZE, ADDR_DEMOD);

  tmcc->Indicator = getField(buf[0xc5 - BASE], 3, 5);
  tmcc->UpLink = getField(buf[0xc7 - BASE], 0, 4);

  for (uint8 i = 0; i < 4; i++) {
    uint8	byteOffset = i / 2;
    uint8	bitOffset = (i % 2) ? 0 : 4;
    tmcc->Mode[i] = getField(buf[0xc8 + byteOffset - BASE], bitOffset, 4);
    tmcc->Slot[i] = getField(buf[0xca + i - BASE], 0, 6);
  }

  for (uint8 i = 0; i < 8; i++) {
    index = 0xce + i * 2 - BASE;
    tmcc->Id[i] = (buf[index] << 8) | buf[index + 1];
  }

  // ����TS�ԍ�0�ɑ΂���TS_ID���j�^��0�̏ꍇ�ُ͈�I��
  if (tmcc->Id[0] == 0) {
    TRACE_F(_T("Invalid tsid0 value.")
      << _T(" tsid0=") << std::dec << tmcc->Id[0]);
    throw InternalException(STATUS_GENERAL_ERROR, "Invalid tsid0 value.");
  }
}


void Jdts290532S::getTmcc(Device::TmccT * tmcc) const {

  TRACE_F(_T("Method unmatch."));
  throw InternalException(STATUS_INTERNAL_ERROR, "Method unmatch.");
}


void Jdts290532S::initReg(uint8 * reg_val, uint8 offset, size_t size) {

  constexpr uint8 init_val[] =
  {
    0x07,	// [7:3](���t�@�����X���g��)=0(16MHz)�A[2](LPT)=1(WAKE UP)�A[1](RX)=1(WAKE UP)�A[0](SYN)=1(WAKE UP)
    0x11,
    0xdc,
    0x85,
    0x0c,	// [6](RCCLKOFF)=0(Activate the calibration clock)�A[4:0](�ш敝)=12(34.5MHz / 2 - 5)
    0x01,
    0xe6,
    0x1e
  };

  ::CopyMemory(reg_val, &init_val[offset], size);
}


uint32 Jdts290532S::getChannelFrequency(uint32 channel, sint32 offset) {

  // �`�����l���ԍ��̗L���͈̓`�F�b�N
  if (channel > CHANNEL_MAX) {
    TRACE_F(_T("Invalid channnel.")
      << _T(" channel=") << std::dec << channel);
    throw InternalException(STATUS_INVALID_PARAM_ERROR, "Invalid channnel.");
  }

  uint32 freq_khz;

  // �`�����l���ԍ������g���ɕϊ��i�`�����l���ԍ���EARTH_PT3�ɏ����j
  if (channel < 12)
    freq_khz = 1049480 + 38360 * channel;
  else if (channel < 24)
    freq_khz = 1613000 + 40000 * (channel - 12);
  else
    freq_khz = 1593000 + 40000 * (channel - 24);

  // offset���w�肳��Ă���ΐݒ�i1kHz�P�ʁj
  if (offset != 0) freq_khz += offset;

  return freq_khz;
}



void Jdts290532S::tuneRf(uint32 freq) const {

  // ���g���ݒ�
  uint32 divider, ref, p, presc;
  uint32 mclk = 16000000;
  sint32 p_calc, p_calc_opt = 1000, r_div, r_div_opt = 0, p_val;

  if (freq <= 1023000) {
    p = 1;
    presc = 0;
  } else if (freq <= 1300000) {
    p = 1;
    presc = 1;
  } else if (freq <= 2046000) {
    p = 0;
    presc = 0;
  } else {
    p = 0;
    presc = 1;
  }

  p_val = (int)(1 << (p + 1)) * 10;
  for (r_div = 0; r_div <= 3; r_div++) {
    p_calc = (mclk / 100000);
    p_calc /= (1 << (r_div + 1));
    if ((subAbsolute(p_calc, p_val)) < (subAbsolute(p_calc_opt, p_val)))
      r_div_opt = r_div;

    p_calc_opt = (mclk / 100000);
    p_calc_opt /= (1 << (r_div_opt + 1));
  }

  ref = mclk / ((1 << (r_div_opt + 1))  * (1 << (p + 1)));
  divider = (((freq * 1000) + (ref >> 1)) / ref);

  uint8 reg_val[4];

  reg_val[0] = divider & 0xff;
  reg_val[1] = static_cast<uint8>
    ((p << 4) | (presc << 5) | (r_div_opt << 6) | ((divider >> 8) & 0x0f));
  initReg(&reg_val[2], 4, 2);

  writeReg(0x02, reg_val, sizeof(reg_val), ADDR_DEMOD, ADDR_TUNER);

  // VCO auto calibration �J�n
  setField(reg_val[3], 2, 1, 1);

  writeReg(0x05, &reg_val[3], 1, ADDR_DEMOD, ADDR_TUNER);

  int retry = 0;
  uint8 buf;
  while (true) {
    readReg(0x05, &buf, 1, ADDR_DEMOD, ADDR_TUNER);

    if (getField(buf, 2, 1) == 1)
      break;

    if (retry++ < RETRY_CNT) {
      Sleep(RETRY_WAIT);
    } else {
      TRACE_F(_T("Rretry over VCO auto calibration.")
        << _T(" CALVCOSTRT=") << std::dec << getField(buf, 2, 1));

      break;
    }
  }

  // LPF auto calibration �J�n
  setField(reg_val[3], 1, 1, 1);

  writeReg(0x05, &reg_val[3], 1, ADDR_DEMOD, ADDR_TUNER);

  while (true) {
    readReg(0x05, &buf, 1, ADDR_DEMOD, ADDR_TUNER);

    if (getField(buf, 1, 1) == 1)
      break;

    if (retry++ < RETRY_CNT) {
      Sleep(RETRY_WAIT);
    } else {
      TRACE_F(_T("Retry over LPF auto calibration.")
        << _T(" CALRCSTRT=") << std::dec << getField(buf, 1, 1));

      break;
    }
  }

  // calibration done, desactivate the calibration Clock
  setField(reg_val[2], 6, 1, 1);

  writeReg(0x04, &reg_val[2], 1, ADDR_DEMOD, ADDR_TUNER);
}


void Jdts290532S::getTunerLockStatus(bool* lock) const {

  // ���W�X�^�ǂݏo��
  uint8 status;
  readReg(0x05, &status, 1, ADDR_DEMOD, ADDR_TUNER);

  // ���b�N��Ԃ̎擾
  *lock = (getField(status, 0, 1) == 1) ? true : false;
}


void Jdts290532S::setAgcAuto(bool agc_auto) const {

  uint8 reg_val;

  reg_val = (agc_auto) ? 0xff : 0x00;
  writeReg(0x0a, &reg_val, 1, ADDR_DEMOD);

  reg_val = (agc_auto) ? 0xb1 : 0xb0;
  writeReg(0x10, &reg_val, 1, ADDR_DEMOD);

  reg_val = (agc_auto) ? 0x40 : 0x00;
  writeReg(0x11, &reg_val, 1, ADDR_DEMOD);

  // �������Z�b�g(pskmsrst=1)
  reg_val = 0x01;
  writeReg(0x03, &reg_val, 1, ADDR_DEMOD);
}


}
}