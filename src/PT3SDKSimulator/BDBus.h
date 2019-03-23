#pragma once

#include <vector>
#include <EARTH_PT3.h>

namespace EARTH {
namespace PT {

/**
* EARTH_PT3.hのBusクラス（インターフェース）の実装クラス。
*/
class BDBus : public Bus {

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
  BDBus();

  /**
  * デストラクタ（非公開）
  */
  virtual ~BDBus() override = default;

private:

  BDBus(BDBus const&) = delete;
  BDBus(BDBus&&) = delete;
  BDBus& operator =(BDBus const&) = delete;
  BDBus& operator =(BDBus&&) = delete;
};

}
}
