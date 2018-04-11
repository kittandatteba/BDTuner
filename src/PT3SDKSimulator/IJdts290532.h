#pragma once

#include <EARTH_PT3.h>

namespace EARTH {
namespace PT {

class II2C;

/**
* JDTS290532の制御を行うためのインターフェースクラス。
*/
class IJdts290532 {

public:

  /**
  * コンストラクタ。
  */
  IJdts290532(II2C *i2CInterface);

  /**
  * デストラクタ。
  */
  virtual ~IJdts290532() = default;

  /**
  * チューナーを初期化する。
  */
  virtual void initTuner() const = 0;

  /**
  * 復調ICを初期化する。
  */
  virtual void initDemod() const = 0;

  /**
  * チャンネル番号+オフセット値でチューニングする。
  */
  virtual void setFrequency(uint32 channel, sint32 offset) = 0;

  /**
  * チャンネル番号+オフセット値を取得する。
  */
  virtual void getFrequency(uint32 *channel, sint32 *offset) const noexcept = 0;

  /**
  * スリープ状態のオン／オフを切り替える。
  */
  virtual void setTunerSleep(bool sleep) = 0;

  /**
  * スリープ状態を取得する。
  */
  virtual void getTunerSleep(bool *sleep) const noexcept = 0;

  /**
  * TS-IDを設定する。(ISDB-S専用)
  */
  virtual void setId(uint32 id) const = 0;

  /**
  * TS-IDを取得する。(ISDB-S専用)
  */
  virtual void getId(uint32 *id) const = 0;

  /**
  * C/N値を取得する。
  */
  virtual void getCn(uint32 *cn100) const = 0;

  /**
  * AGC制御量を取得する。
  */
  virtual void getAgc(uint8 *agc, uint8 *max_agc) const = 0;

  /**
  * TMCC情報を取得する。(ISDB-T専用)
  */
  virtual void getTmcc(Device::TmccT *tmcc) const = 0;

  /**
  * TMCC情報を取得する。(ISDB-S専用)
  */
  virtual void getTmcc(Device::TmccS *tmcc) const = 0;

protected:

  /**
  * チューナー制御アドレス・コマンド
  */
  enum {
    PASS_THROUGH = 0xfeU		// 復調ICのパススルーモード
  };

  /**
  * レジスタ確認の最大リトライ回数。
  */
  static constexpr uint32 RETRY_CNT = 10;

  /**
  * レジスタ確認のリトライ間隔。（ms単位）
  */
  static constexpr uint32 RETRY_WAIT = 10;

  /**
  * I2C通信インターフェイス。
  */
  II2C* i2CInterface;

  /**
  * チャンネル番号。
  */
  uint32 mChannel;

  /**
  * オフセット値。
  */
  sint32 mOffset;

  /**
  * チューナーのスリープ状態。
  */
  bool mSleep;

  /**
  * レジスタへの書き込みを行う。
  */
  void writeReg(uint8 addr_reg, uint8* data,
    uint32 len, uint8 addr_demod, uint8 addr_tuner = 0xff) const;

  /**
  * レジスタの読み込みを行う。
  */
  void readReg(uint8 addr_reg, uint8* data,
    uint32 len, uint8 addr_demod, uint8 addr_tuner = 0xff) const;

  /**
  * バイトデータの指定ビットフィールドに値を設定する。
  */
  inline static void setField(uint8& data, uint8 offset, uint8 width, uint8 value) {
    data = (data & (~(((1 << width) - 1) << offset))) | (value << offset);
  }

  /**
  * バイトデータの指定ビットフィールドの値を返す。
  */
  inline static uint8 getField(uint8 data, uint8 offset, uint8 width) {
    return (data >> offset) & ((1 << width) - 1);
  }

  /**
  * 差分を絶対値で返す。
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