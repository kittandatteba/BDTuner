#pragma once

#include <vector>
#include <EARTH_PT3.h>

namespace EARTH {
namespace PT {

/**
* EARTH_PT3.h��Bus�N���X�i�C���^�[�t�F�[�X�j�̎����N���X�B
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
  * �R���X�g���N�^�i����J�j
  */
  BD_Bus();

  /**
  * �f�X�g���N�^�i����J�j
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