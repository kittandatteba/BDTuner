#pragma once

#include <EARTH_PT3.h>

namespace EARTH {
namespace PT {

class II2C;

/**
* JDTS290532�̐�����s�����߂̃C���^�[�t�F�[�X�N���X�B
*/
class IJdts290532 {

public:

  /**
  * �R���X�g���N�^�B
  */
  IJdts290532(II2C *i2CInterface);

  /**
  * �f�X�g���N�^�B
  */
  virtual ~IJdts290532() = default;

  /**
  * �`���[�i�[������������B
  */
  virtual void initTuner() const = 0;

  /**
  * ����IC������������B
  */
  virtual void initDemod() const = 0;

  /**
  * �`�����l���ԍ�+�I�t�Z�b�g�l�Ń`���[�j���O����B
  */
  virtual void setFrequency(uint32 channel, sint32 offset) = 0;

  /**
  * �`�����l���ԍ�+�I�t�Z�b�g�l���擾����B
  */
  virtual void getFrequency(uint32 *channel, sint32 *offset) const noexcept = 0;

  /**
  * �X���[�v��Ԃ̃I���^�I�t��؂�ւ���B
  */
  virtual void setTunerSleep(bool sleep) = 0;

  /**
  * �X���[�v��Ԃ��擾����B
  */
  virtual void getTunerSleep(bool *sleep) const noexcept = 0;

  /**
  * TS-ID��ݒ肷��B(ISDB-S��p)
  */
  virtual void setId(uint32 id) const = 0;

  /**
  * TS-ID���擾����B(ISDB-S��p)
  */
  virtual void getId(uint32 *id) const = 0;

  /**
  * C/N�l���擾����B
  */
  virtual void getCn(uint32 *cn100) const = 0;

  /**
  * AGC����ʂ��擾����B
  */
  virtual void getAgc(uint8 *agc, uint8 *max_agc) const = 0;

  /**
  * TMCC�����擾����B(ISDB-T��p)
  */
  virtual void getTmcc(Device::TmccT *tmcc) const = 0;

  /**
  * TMCC�����擾����B(ISDB-S��p)
  */
  virtual void getTmcc(Device::TmccS *tmcc) const = 0;

protected:

  /**
  * �`���[�i�[����A�h���X�E�R�}���h
  */
  enum {
    PASS_THROUGH = 0xfeU		// ����IC�̃p�X�X���[���[�h
  };

  /**
  * ���W�X�^�m�F�̍ő僊�g���C�񐔁B
  */
  static constexpr uint32 RETRY_CNT = 10;

  /**
  * ���W�X�^�m�F�̃��g���C�Ԋu�B�ims�P�ʁj
  */
  static constexpr uint32 RETRY_WAIT = 10;

  /**
  * I2C�ʐM�C���^�[�t�F�C�X�B
  */
  II2C* i2CInterface;

  /**
  * �`�����l���ԍ��B
  */
  uint32 mChannel;

  /**
  * �I�t�Z�b�g�l�B
  */
  sint32 mOffset;

  /**
  * �`���[�i�[�̃X���[�v��ԁB
  */
  bool mSleep;

  /**
  * ���W�X�^�ւ̏������݂��s���B
  */
  void writeReg(uint8 addr_reg, uint8* data,
    uint32 len, uint8 addr_demod, uint8 addr_tuner = 0xff) const;

  /**
  * ���W�X�^�̓ǂݍ��݂��s���B
  */
  void readReg(uint8 addr_reg, uint8* data,
    uint32 len, uint8 addr_demod, uint8 addr_tuner = 0xff) const;

  /**
  * �o�C�g�f�[�^�̎w��r�b�g�t�B�[���h�ɒl��ݒ肷��B
  */
  inline static void setField(uint8& data, uint8 offset, uint8 width, uint8 value) {
    data = (data & (~(((1 << width) - 1) << offset))) | (value << offset);
  }

  /**
  * �o�C�g�f�[�^�̎w��r�b�g�t�B�[���h�̒l��Ԃ��B
  */
  inline static uint8 getField(uint8 data, uint8 offset, uint8 width) {
    return (data >> offset) & ((1 << width) - 1);
  }

  /**
  * �������Βl�ŕԂ��B
  */
  inline static sint32 subAbsolute(sint32 a, sint32 b) {
    if (a > b) {
      return a - b;
    } else {
      return b - a;
    }
  }

private:

  IJdts290532(IJdts290532 const&) = delete;
  IJdts290532(IJdts290532&&) = delete;
  IJdts290532& operator =(IJdts290532 const&) = delete;
  IJdts290532& operator =(IJdts290532&&) = delete;

};

}
}