#pragma once

#include <EARTH_PT3.h>

namespace EARTH {
namespace PT {

/**
* EzUsbFx2Lp�����TS�f�[�^��M�p�C���^�[�t�F�[�X�N���X�B
*/
class IReceiver {
public:

  virtual bool receive(Device::ISDB isdb, uint32 tunerIndex, uint8* data, uint32 size) = 0;

  /**
  * �R���X�g���N�^�B
  */
  IReceiver() = default;

  /**
  * �f�X�g���N�^�B
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
