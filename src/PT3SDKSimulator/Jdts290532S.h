#pragma once

#include <EARTH_PT3.h>
#include "IJdts290532.h"

namespace EARTH {
namespace PT {

/**
* JDTS290532(ISDB-S側)の制御を行うためのクラス。
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
  * チューナー制御アドレス・コマンド
  */
  enum {
    ADDR_DEMOD = 0x11U,		// 復調IC(ISDB-S)のアドレス
    ADDR_TUNER = 0x63U		// チューナ(ISDB-S)のアドレス
  };

  /**
  * チャネル番号の最大値。
  */
  static constexpr uint32 CHANNEL_MAX = 35;

  /**
  * チャンネル周波数を取得する。
  */
  static uint32 getChannelFrequency(uint32 channel, sint32 offset);

  /**
  * レジスタ設定値を初期化する。
  */
  void initReg(uint8* reg_val, uint8 offset, size_t size) const;

  /**
  * チャンネル周波数でチューニングする。
  */
  void tuneRf(uint32 freq) const;

  /**
  * チューナーのロック状態を取得する。
  */
  void getTunerLockStatus(bool* lock) const;

  /**
  * 自動AGCのオン／オフを切り替える。
  */
  void setAgcAuto(bool agc_auto) const;

  Jdts290532S(Jdts290532S const&) = delete;
  Jdts290532S(Jdts290532S&&) = delete;
  Jdts290532S& operator =(Jdts290532S const&) = delete;
  Jdts290532S& operator =(Jdts290532S&&) = delete;

};

}
}