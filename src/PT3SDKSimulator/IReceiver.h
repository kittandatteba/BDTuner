#pragma once

#include <EARTH_PT3.h>

namespace EARTH {
namespace PT {

/**
* EzUsbFx2LpからのTSデータ受信用インターフェースクラス。
*/
class IReceiver {
public:

  virtual bool receive(Device::ISDB isdb, uint32 tunerIndex, uint8* data, uint32 size) = 0;

  /**
  * コンストラクタ。
  */
  IReceiver() = default;

  /**
  * デストラクタ。
  */
  virtual ~IReceiver() = default;

private:

  IReceiver(const IReceiver&) = delete;
  IReceiver(IReceiver&&) = delete;
  IReceiver& operator =(const IReceiver&) = delete;
  IReceiver& operator =(IReceiver&&) = delete;
};

}
}
