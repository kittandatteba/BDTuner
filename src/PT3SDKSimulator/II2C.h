#pragma once

#include <Prefix_Integer.h>

namespace EARTH {
namespace PT {

/**
* I2C通信インターフェースクラス。
*/
class II2C {

public:

  /**
  * I2C通信の最大メッセージ長。
  */
  static constexpr uint32 I2C_MSG_MAX = 64;

  /**
  * I2C操作種別。
  */
  enum Operation {
    READ,		// 読み取り
    WRITE		// 書き込み
  };

  /**
  * I2C通信メッセージ構造体。
  */
  struct Message {
    uint8     addr;	// I2Cアドレス
    Operation	ope;	// 読み取りor書き込み
    uint8     len;	// データ長
    uint8*	  buf;	// データ（バイト列）
  };

  /**
  * I2Cメッセージの送受信を行う。
  */
  virtual uint32 xfer(Message* msg, uint32 len) = 0;

  /**
  * コンストラクタ。
  */
  II2C() = default;

  /**
  * デストラクタ。
  */
  virtual ~II2C() = default;

private:

  II2C(const II2C&) = delete;
  II2C(II2C&&) = delete;
  II2C& operator =(const II2C&) = delete;
  II2C& operator =(II2C&&) = delete;
};

}
}