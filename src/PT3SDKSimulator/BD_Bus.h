#pragma once

#include <vector>
#include <EARTH_PT3.h>

namespace EARTH {
namespace PT {

/**
* EARTH_PT3.hのBusクラス（インターフェース）の実装クラス。
*/
class BD_Bus : public Bus {

public:

  static status GetInstance(Bus **bus);

  virtual status Delete() override;

  virtual status GetVersion(uint32 *version) const override;

  virtual status Scan(DeviceInfo *deviceInfoPtr, uint32 *deviceInfoCount) override;

  virtual status NewDevice(const DeviceInfo *deviceInfoPtr, Device **device, Device_ ** device_ = nullptr) override;

protected:

  /**
  * コンストラクタ（非公開）
  */
  BD_Bus();

  /**
  * デストラクタ（非公開）
  */
  virtual ~BD_Bus() override = default;

private:

  BD_Bus(BD_Bus const&) = delete;
  BD_Bus(BD_Bus&&) = delete;
  BD_Bus& operator =(BD_Bus const&) = delete;
  BD_Bus& operator =(BD_Bus&&) = delete;
};

}
}