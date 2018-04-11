#pragma once

#include <EARTH_PT3.h>
#include "IJdts290532.h"

namespace EARTH {
namespace PT {

/**
* JDTS290532(ISDB-T��)�̐�����s�����߂̃N���X�B
*/
class Jdts290532T : public IJdts290532 {

public:

  Jdts290532T(II2C *i2CInterface);

  virtual ~Jdts290532T() override = default;

  virtual void initTuner() const override;

  virtual void initDemod() const override;

  virtual void setFrequency(uint32 channel, sint32 offset) override;

  virtual void getFrequency(uint32 *channel, sint32 *offset) const noexcept override;

  virtual void setTunerSleep(bool sleep) override;

  virtual void getTunerSleep(bool *sleep) const noexcept override;

  virtual void setId(uint32 id) const override;

  virtual void getId(uint32 *id) const override;

  virtual void getCn(uint32 * cn100) const override;

  virtual void getAgc(uint8 *agc, uint8 *max_agc) const override;

  virtual void getTmcc(Device::TmccT *tmcc) const override;

  virtual void getTmcc(Device::TmccS *tmcc) const override;

private:

  /**
  * �`���[�i�[����A�h���X�E�R�}���h
  */
  enum {
    ADDR_DEMOD = 0x10U,		// ����IC(ISDB-T)�̃A�h���X
    ADDR_TUNER = 0x60U		// �`���[�i(ISDB-T)�̃A�h���X
  };

  /**
  * �`���l���ԍ��̍ő�l�B
  */
  static constexpr uint32 CHANNEL_MAX = 112;

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
  void getTunerLockStatus(bool* rf_synth_lock, bool* ref_synth_lock) const;

  /**
  * ����AGC�̃I���^�I�t��؂�ւ���B
  */
  void setAgcAuto(bool agc_auto) const;

  Jdts290532T(Jdts290532T const&) = delete;
  Jdts290532T(Jdts290532T&&) = delete;
  Jdts290532T& operator =(Jdts290532T const&) = delete;
  Jdts290532T& operator =(Jdts290532T&&) = delete;

};

}
}