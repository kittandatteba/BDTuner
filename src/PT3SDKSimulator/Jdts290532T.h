#pragma once

#include <EARTH_PT3.h>
#include "IJdts290532.h"

namespace EARTH {
namespace PT {

/**
* JDTS290532(ISDB-T側)の制御を行うためのクラス。
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
  * チューナー制御アドレス・コマンド
  */
  enum {
    ADDR_DEMOD = 0x10U,		// 復調IC(ISDB-T)のアドレス
    ADDR_TUNER = 0x60U		// チューナ(ISDB-T)のアドレス
  };

  /**
  * チャネル番号の最大値。
  */
  static constexpr uint32 CHANNEL_MAX = 112;

  /**
  * チャンネル周波数を取得する。
  */
  static uint32 getChannelFrequency(uint32 channel, sint32 offset);

  /**
  * チャンネル周波数でチューニングする。
  */
  void tuneRf(uint32 freq) const;

  /**
  * チューナーのロック状態を取得する。
  */
  void getTunerLockStatus(bool* rf_synth_lock, bool* ref_synth_lock) const;

  /**
  * 自動AGCのオン／オフを切り替える。
  */
  void setAgcAuto(bool agc_auto) const;

  Jdts290532T(Jdts290532T const&) = delete;
  Jdts290532T(Jdts290532T&&) = delete;
  Jdts290532T& operator =(Jdts290532T const&) = delete;
  Jdts290532T& operator =(Jdts290532T&&) = delete;

};

}
}