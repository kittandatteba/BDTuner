#include "stdafx.h"
#include "InternalException.h"
#include "Jdts290532T.h"

namespace EARTH {
namespace PT {


Jdts290532T::Jdts290532T(II2C* i2CInterface)
  :IJdts290532(i2CInterface) {}


void Jdts290532T::initTuner() const {

  // �\�t�g���Z�b�g
  writeReg(0xff, NULL, 0, ADDR_DEMOD, ADDR_TUNER);

  uint8 reg_val[] = {
    // �A�h���X, �l
    0x02, 0x00,	// [3:0](I/F���g��)=0(4MHz)�A[4](I/F���])=0(�Ȃ�)
    0x03, 0x00,	// [7:4](�N���X�^�����g��)=0(16MHz)�A[3:0](�N���b�N�o��)=0(����)
    0x05, 0x00,	// [3:0](���t�@�����X���g��)=0(16MHz)
    0x06, 0x10,	// [4:0](���[�h)=2(ISDB-T)
    0x2e, 0x15,
    0x30, 0x10,
    0x45, 0x58,
    0x48, 0x19,
    0x52, 0x03,
    0x53, 0x44,
    0x6a, 0x4b,
    0x76, 0x00,
    0x78, 0x18,
    0x7a, 0x17,
    0x85, 0x06,
    0x01, 0x01 	// TOP_MASTER_ENABLE=1
  };

  // ���W�X�^�ݒ�
  writeReg(reg_val[0], &reg_val[1], sizeof(reg_val) - 1, ADDR_DEMOD, ADDR_TUNER);
}


void Jdts290532T::initDemod()const {

  uint8 reg_val[] = {
    // �A�h���X, �l
    0x13, 0x33,	// OFDM�N���b�N���g���I�t�Z�b�g�␳(4MHz IF(XT=4MHz)���[�h)
    0x14, 0x20,	// OFDM�N���b�N���g���I�t�Z�b�g�␳(4MHz IF(XT=4MHz)���[�h)
    0x30, 0x20,	// �L�����AAFC���[�v����2�ݒ�
    0x31, 0x0d,	// OFDM�L�����A���g���I�t�Z�b�g�␳(4MHz IF(XT=4MHz)���[�h)
    0x32, 0x56,	// OFDM�L�����A���g���I�t�Z�b�g�␳(4MHz IF(XT=4MHz)���[�h)
    0x47, 0x10,	// ??
    0x58, 0x08,	// ??
    0x7d, 0xd2,	// ??
    0xf4, 0x80,	// ISIC���E���ɓ���p��
    0x73, 0x01,	// ??
    0x74, 0x02,	// ??
    0x71, 0x20,	// [5]palonff = 1(�p���e�B���ԃN���b�N��~,�f�[�^��~)
    0x76, 0x0e	// [3]nuval = 1(Null�p�P�b�g��Valid�M���𗧂ĂȂ�)
                // [2]anuval = 1(Null�����ꂽ�p�P�b�g��Valid�M���𗧂ĂȂ�)
  };

  for (int i = 0; i < sizeof(reg_val); i += 2) {
    writeReg(reg_val[i], &reg_val[i + 1], 1, ADDR_DEMOD);
  }
}


void Jdts290532T::setFrequency(uint32 channel, sint32 offset) {

  IJdts290532::setFrequency(channel, offset);

  // ����IC��AGC�o�̓I�t�ݒ�
  setAgcAuto(false);

  // �`�����l���ԍ������g���ɕϊ�
  uint32 freq_hz = getChannelFrequency(channel, offset);

  // �`���[�j���O
  tuneRf(freq_hz);

  // ���b�N��Ԃ̎擾
  bool rf_synth_lock, ref_synth_lock;
  int retry = 0;
  while (true) {
    getTunerLockStatus(&rf_synth_lock, &ref_synth_lock);

    if (rf_synth_lock && ref_synth_lock) break;

    if (retry++ < RETRY_CNT) {
      Sleep(RETRY_WAIT);
    } else {
      TRACE_F(_T("Retry over get tuner lock status.")
        << _T(" rf_synth_lock=") << std::dec << rf_synth_lock
        << _T(" ref_synth_lock=") << std::dec << ref_synth_lock);

      break;
    }
  }

  // ����IC��AGC�o�̓I���ݒ�
  setAgcAuto(true);
}


void Jdts290532T::getFrequency(uint32 * channel, sint32 * offset) const noexcept {

  IJdts290532::getFrequency(channel, offset);
}


void Jdts290532T::setTunerSleep(bool sleep) {

  IJdts290532::setTunerSleep(sleep);

  if (sleep) {
    // ����IC��AGC�o�̓I�t�ݒ�
    setAgcAuto(false);

    // �`���[�i�̃X���[�v�ݒ�
    uint8 ary_reg_val[] = {
      // �A�h���X, �l
      0x01, 0x00,
      0x0f, 0x00
    };
    writeReg(ary_reg_val[0], &ary_reg_val[1], sizeof(ary_reg_val) - 1,
      ADDR_DEMOD, ADDR_TUNER);

    // ����IC�̃X���[�v�ݒ�
    uint8 reg_val = 0x90;
    writeReg(0x03, &reg_val, 1, ADDR_DEMOD);

  } else {
    // ����IC�̃E�F�C�N�A�b�v�ݒ�
    uint8 reg_val = 0x80;
    writeReg(0x03, &reg_val, 1, ADDR_DEMOD);

    // �`���[�i�̃E�F�C�N�A�b�v�ݒ�
    uint8 ary_reg_val[] = {
      // �A�h���X, �l
      0x01, 0x01
    };
    writeReg(ary_reg_val[0], &ary_reg_val[1], sizeof(ary_reg_val) - 1,
      ADDR_DEMOD, ADDR_TUNER);
  }
}


void Jdts290532T::getTunerSleep(bool * sleep) const noexcept {

  IJdts290532::getTunerSleep(sleep);
}


void Jdts290532T::setId(uint32 id) const {

  TRACE_F(_T("Method unmatch."));
  throw InternalException(STATUS_INTERNAL_ERROR, "Method unmatch.");
}


void Jdts290532T::getId(uint32 * id) const {

  TRACE_F(_T("Method unmatch."));
  throw InternalException(STATUS_INTERNAL_ERROR, "Method unmatch.");
}


void Jdts290532T::getCn(uint32* cn100) const {

  // ���W�X�^cndat�̓ǂݏo��
  uint8 buf[3];
  readReg(0x8b, buf, 3, ADDR_DEMOD);

  // ��ʁE���ʃo�C�g�̌���
  uint32 cndat = (buf[0] << 16) | (buf[1] << 8) | buf[2];
  if (cndat == 0) {
    *cn100 = 0;
    return;
  }

  // C/N�̌v�Z
  // cn100 = 0.0024 * P^4 - 0.16 * P^3 + 3.98 * P^2 + 54.91 * P + 309.65
  // (P = 10 * log(5505024 / cndat))
  double p = 10 * std::log10(5505024.0 / cndat);

  double p2 = std::pow(p, 2);
  double p3 = std::pow(p, 3);
  double p4 = std::pow(p, 4);

  p = 0.0024*p4 - 0.16*p3 + 3.98*p2 + 54.91*p + 309.65;

  *cn100 = static_cast<uint32>(p + 0.5);
}

void Jdts290532T::getAgc(uint8 *agc, uint8 *max_agc) const {

  // ���W�X�^ifagc_dt�̓ǂݏo��
  uint8 data;
  readReg(0x82, &data, 1, ADDR_DEMOD);

  *agc = data;
  *max_agc = 255;
}

void Jdts290532T::getTmcc(Device::TmccT * tmcc) const {

  bool ret = false;
  do {
    // ���W�X�^retryov,tmunvld,fulock�̓ǂݏo��
    uint8 data;
    readReg(0x80, &data, 1, ADDR_DEMOD);

    uint8 retryov = getField(data, 7, 1);
    uint8 tmunvld = getField(data, 5, 1);
    uint8 fulock = getField(data, 3, 1);

    if (fulock == 0) {
      ret = true;
      break;
    } else if (retryov != 0) {
      TRACE_F(_T("Invalid retryov value.")
        << _T(" retryov=") << std::dec << retryov);
      break;
    }

    ::Sleep(1);

  } while (!ret);

  if (!ret) {
    TRACE_F(_T("Can not read TMCC value."));
    throw InternalException(STATUS_GENERAL_ERROR, "Can not read TMCC value.");
  }

  static const uint8 BASE = 0xb2;
  static const uint8 SIZE = 0xb9 - BASE + 1;
  uint8	buf[SIZE];

  // TMCC��񃂃j�^���W�X�^�̓ǂݏo��
  readReg(BASE, buf, SIZE, ADDR_DEMOD);

  tmcc->System = getField(buf[0xb2 - BASE], 6, 2);
  tmcc->Indicator = getField(buf[0xb2 - BASE], 2, 4);
  tmcc->Emergency = getField(buf[0xb2 - BASE], 1, 1);
  tmcc->Partial = getField(buf[0xb2 - BASE], 0, 1);

  tmcc->Mode[0] = getField(buf[0xb3 - BASE], 5, 3);
  tmcc->Mode[1] = getField(buf[0xb4 - BASE], 0, 3);
  tmcc->Mode[2] = getField(buf[0xb6 - BASE], 3, 3);

  tmcc->Rate[0] = getField(buf[0xb3 - BASE], 2, 3);
  tmcc->Rate[1] = getField(buf[0xb5 - BASE], 5, 3);
  tmcc->Rate[2] = getField(buf[0xb6 - BASE], 0, 3);

  tmcc->Interleave[0] = getField(buf[0xb3 - BASE], 0, 2) << 1 | getField(buf[0xb4 - BASE], 7, 1);
  tmcc->Interleave[1] = getField(buf[0xb5 - BASE], 2, 3);
  tmcc->Interleave[2] = getField(buf[0xb7 - BASE], 5, 3);

  tmcc->Segment[0] = getField(buf[0xb4 - BASE], 3, 4);
  tmcc->Segment[1] = getField(buf[0xb5 - BASE], 0, 2) << 2 | getField(buf[0xb6 - BASE], 6, 2);
  tmcc->Segment[2] = getField(buf[0xb7 - BASE], 1, 4);
}


void Jdts290532T::getTmcc(Device::TmccS * tmcc) const {

  TRACE_F(_T("Method unmatch."));
  throw InternalException(STATUS_GENERAL_ERROR, "Method unmatch.");
}


uint32 Jdts290532T::getChannelFrequency(uint32 channel, sint32 offset) {

  // �`�����l���ԍ��̗L���͈̓`�F�b�N
  if (channel > CHANNEL_MAX) {
    TRACE_F(_T("Invalid channnel.")
      << _T(" channel=") << std::dec << channel);
    throw InternalException(STATUS_INVALID_PARAM_ERROR, "Invalid channnel.");
  }

  // �`�����l���ԍ������g���ɕϊ��i�`�����l���ԍ���EARTH_PT3�ɏ����j
  uint32 freq_hz = 93000000 + 1000000 / 7 + 6000000 * channel;
  if (channel > 11 && channel < 17)
    freq_hz += 2000000;
  else if (channel > 62)
    freq_hz += 2000000;

  // offset���w�肳��Ă���ΐݒ�i1/7MHz�P�ʁj
  if (offset != 0) freq_hz += offset * 1000000 / 7;

  return freq_hz;
}


void Jdts290532T::tuneRf(uint32 freq) const {

  uint8 reg_val[] = {
    // �A�h���X, �l
    0x0f, 0x00,	// �`���[�j���O�𒆎~
    0x0c, 0x15,	// [5:0](�ш敝)=0x15(6MHz)
    0x0d, 0x40,
    0x0e, 0x0e,
    0x1f, 0x87,
    0x20, 0x1f,
    0x21, 0x87,
    0x22, 0x1f,
    0x80, 0x01,
    0x0f, 0x01 	// �`���[�j���O���J�n
  };

  // Convert RF frequency into 16 bits => 10 bit integer (MHz) + 6 bit fraction
  uint32 dig_rf_freq = 0;
  uint32 temp;
  uint32 frac_divider = 1000000;

  dig_rf_freq = freq / 1000000;
  temp = freq % 1000000;
  for (int i = 0; i < 6; i++) {
    dig_rf_freq <<= 1;
    frac_divider /= 2;
    if (temp > frac_divider) {
      temp -= frac_divider;
      dig_rf_freq++;
    }
  }

  // add to have shift center point by 7.8124 kHz
  if (temp > 7812)
    dig_rf_freq++;

  reg_val[5] = dig_rf_freq & 0xff;
  reg_val[7] = (dig_rf_freq >> 8) & 0xff;

  // Freq Dependent Setting
  if (freq >= 333000000) setField(reg_val[17], 2, 1, 1);

  // ���W�X�^�ݒ�
  writeReg(reg_val[0], &reg_val[1], sizeof(reg_val) - 1, ADDR_DEMOD, ADDR_TUNER);
}


void Jdts290532T::getTunerLockStatus(bool* rf_synth_lock, bool* ref_synth_lock) const {

  // ���W�X�^�ǂݏo��
  uint8 status;
  readReg(0xD8, &status, 1, ADDR_DEMOD, ADDR_TUNER);

  // [3:2](RF synthesizer���b�N���)�̎擾
  uint8 rf_synth = getField(status, 2, 2);
  *rf_synth_lock = (rf_synth == 3) ? true : false;

  // [1:0](RF synthesizer���b�N���)�̎擾
  uint8 ref_synth = getField(status, 0, 2);
  *ref_synth_lock = (ref_synth == 3) ? true : false;
}


void Jdts290532T::setAgcAuto(bool agc_auto) const {

  uint8 reg_val;

  reg_val = (agc_auto) ? 0x40 : 0x00;
  writeReg(0x25, &reg_val, 1, ADDR_DEMOD);

  reg_val = (agc_auto) ? 0x4c : 0x4d;
  writeReg(0x23, &reg_val, 1, ADDR_DEMOD);

  // �������Z�b�g(imsrst=1)
  reg_val = 0x40;
  writeReg(0x01, &reg_val, 1, ADDR_DEMOD);
}


}
}