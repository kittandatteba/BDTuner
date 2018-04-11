#pragma once

#include <memory>
#include <EARTH_PT3.h>
#include <CyAPI.h>
#include "II2C.h"

namespace EARTH {
namespace PT {

class IReceiver;

/**
* EzUsbFx2Lp�̐�����s�����߂̃N���X�B
*/
class EzUsbFx2Lp : public II2C {

public:

  /**
  * �R���X�g���N�^�B
  */
  EzUsbFx2Lp();

  /**
  * �f�X�g���N�^�B
  */
  virtual ~EzUsbFx2Lp() override;

  /**
  * I2C���b�Z�[�W�̑���M���s���B
  */
  virtual uint32 xfer(Message* msg, uint32 len) override;

  /**
  * �t�@�[���E�F�A�����[�h���A�e�평�������s���B
  */
  void init(uint8 usbadr);

  /**
  * TS�f�[�^�]���X���b�h���J�n����B
  */
  void startXferThread(Device::ISDB isdb, uint32 tunerIndex, IReceiver* receiver);


  /**
  * TS�f�[�^�]���X���b�h���I������B
  */
  void stopXferThread(Device::ISDB isdb);


  /**
  * TS�f�[�^�]���̗L���^������؂�ւ���B
  */
  void setXferEnable(Device::ISDB isdb, bool enabled);

private:

  /**
  * TS�f�[�^�]���o�b�t�@�̃T�C�Y
  */
  static constexpr uint32 TS_QUEUE_SIZE = 65536UL;

  /**
  * TS�f�[�^�]���o�b�t�@�̖{��
  */
  //static const uint32 TS_QUEUE_NUM = 16UL;
  static constexpr uint32 TS_QUEUE_NUM = 8UL;

  /**
  * �t�@�[���E�F�A��
  */
  static constexpr wchar_t FX2FW_NAME[] = L"FX2_TUNR";

  /**
  * �t�@�[���E�F�A�f�[�^
  */
  static constexpr uint8 FX2FW_DATA[] =
  {
    #include "FX2FarmwareTuner.inc"
  };

  /**
  * ReNumeration�̍ő僊�g���C��
  */
  static constexpr sint32 RENUM_RETRY_COUNT = 40;

  /**
  * ReNumeration �̃��g���C�Ԋu
  */
  static constexpr sint32 RENUM_RETRY_INTRVAL = 100;

  /**
  * �G���h�|�C���g�A�h���X
  */
  enum {
    EPA_CTL_IN = 0x81U,		// ����f�[�^��M�p�iEP1 IN�j
    EPA_CTL_OUT = 0x01U,	// ����f�[�^���M�p�iEP1 OUT�j
    EPA_DATA_S = 0x82U,		// ISDB_S��TS�f�[�^��M�p�iEP2 IN�j
    EPA_DATA_T = 0x86U		// ISDB_T��TS�f�[�^��M�p�iEP6 IN�j
  };

  /**
  * �R�}���h�B
  */
  enum {
    CMD_EP6IN_START = 0x50U,
    CMD_EP6IN_STOP = 0x51U,
    CMD_EP2IN_START = 0x52U,
    CMD_EP2IN_STOP = 0x53U,
    CMD_I2C_READ = 0x54U,		//adrs,len (return length bytes)(max 64bytes)
    CMD_I2C_WRITE = 0x55U		//adrs,len,data... (max 64bytes)
  };

  /**
  * �f�[�^��M�X���b�h�\����
  */
  struct ThreadInfo {
    Device::ISDB      isdb = Device::ISDB::ISDB_S;
    uint32            tunerIndex = 0;
    HANDLE            threadHandle = nullptr;
    CCyBulkEndPoint*	endPoint = nullptr;
    bool              running = false;
    IReceiver*         receiver = nullptr;
  };

  /**
  * �r������p�N���X
  */
  class GetLock {
  public:
    GetLock(LPCRITICAL_SECTION cSPointer) :mCSPointer(cSPointer) {
      ::EnterCriticalSection(mCSPointer);
    }

    ~GetLock() {
      ::LeaveCriticalSection(mCSPointer);
    }
  private:
    LPCRITICAL_SECTION  mCSPointer;
  };

  /**
  * �f�[�^��M�X���b�h���
  */
  ThreadInfo mThreadInfo[Device::ISDB::ISDB_COUNT];

  /**
  * �N���e�B�J���Z�N�V����
  */
  CRITICAL_SECTION mCriticalSection;

  /**
  * CyUSBDevice�C���X�^���X
  */
  std::unique_ptr<CCyUSBDevice> mUsbDevice;

  /**
  * ����f�[�^��M�p�G���h�|�C���g
  */
  CCyControlEndPoint*	mEpCtrlIn;

  /**
  * ����f�[�^���M�p�G���h�|�C���g
  */
  CCyControlEndPoint*	mEpCtrlOut;

  /**
  * ISDB_S��TS�f�[�^��M�p�G���h�|�C���g
  */
  CCyBulkEndPoint*	mEpDataS;

  /**
  * ISDB_T��TS�f�[�^��M�p�G���h�|�C���g
  */
  CCyBulkEndPoint*	mEpDataT;

  /**
  * �f�[�^��M�X���b�h���[�`��
  */
  static unsigned CALLBACK processXferThread(void *arg);

  /**
  * �t�@�[���E�F�A�̃��[�h���s���B
  */
  void loadFW(uint8 usbadr);

  EzUsbFx2Lp(EzUsbFx2Lp const&) = delete;
  EzUsbFx2Lp(EzUsbFx2Lp&&) = delete;
  EzUsbFx2Lp& operator =(EzUsbFx2Lp const&) = delete;
  EzUsbFx2Lp& operator =(EzUsbFx2Lp&&) = delete;
};

}
}