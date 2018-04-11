#pragma once

#include <EARTH_PT3.h>
#include "IJdts290532.h"

namespace EARTH {
namespace PT {

/**
* JDTS290532(ISDB-S��)�̐�����s�����߂̃N���X�B
*/
class Jdts290532S : public IJdts290532 {

public:

  Jdts290532S(II2C *i2CInterface);

  virtual ~Jdts290532S() override = default;

  virtual void initTuner() const override;

  virtual void initDemod() const override;

  virtual void setFrequency(uint32 channel, sint32 offset) override;

  virtual void getFrequency(uint32 *channel, sint32 *offset) const noexcept override;

  virtual void setTunerSleep(bool sleep) override;

  virtual void getTunerSleep(bool *sleep) const noexcept override;

  virtual void setId(uint32 id) const override;

  virtual void getId(uint32 *id) const override;

  virtual void getCn(uint32 *cn100) const override;

  virtual void getAgc(uint8 *agc, uint8 *max_agc) const override;

  virtual void getTmcc(Device::TmccS *tmcc) const override;

  virtual void getTmcc(Device::TmccT *tmcc) const override;

private:

  /**
  * �`���[�i�[����A�h���X�E�R�}���h
  */
  enum {
    ADDR_DEMOD = 0x11U,		// ����IC(ISDB-S)�̃A�h���X
    ADDR_TUNER = 0x63U		// �`���[�i(ISDB-S)�̃A�h���X
  };

  /**
  * �`���l���ԍ��̍ő�l�B
  */
  static constexpr uint32 CHANNEL_MAX = 35;

  /**
  * ���W�X�^�ݒ�l������������B
  */
  static void initReg(uint8* reg_val, uint8 offset, size_t size);

  /**
  * �`�����l�����g�����擾����B
  */
  static uint32 getChannelFrequency(uint32 channel, sint32 offset);

  /**
  * �`�����l�����g���Ń`���[�j���O����B
  */
  void tuneRf(uint32 freq) const;

  /**
  * �`���[�i�[�̃��b�N��Ԃ��擾����B
  */
  void getTunerLockStatus(bool* lock) const;

  /**
  * ����AGC�̃I���^�I�t��؂�ւ���B
  */
  void setAgcAuto(bool agc_auto) const;

  Jdts290532S(Jdts290532S const&) = delete;
  Jdts290532S(Jdts290532S&&) = delete;
  Jdts290532S& operator =(Jdts290532S const&) = delete;
  Jdts290532S& operator =(Jdts290532S&&) = delete;

};

}
}