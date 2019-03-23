#pragma once

#include <EARTH_PT3.h>

namespace EARTH {
namespace PT {

/**
* ���ʃ��[�e�B���e�B�[�N���X�B
*/
class BDUtil {

public:

  /**
  * �f�o�C�X�̊J�n�ԍ��i0�`9�j
  */
  static uint32 mDeviceMinIndex;

  /**
  * �`���l���؂�ւ�������TS�f�[�^�]���J�n�܂ł̑҂����ԁimsec�j
  */
  static uint32 mSetFrequencyDelay[Device::ISDB_COUNT];

  /**
  * BS/CS�iISDB-S�j�`���[�i�[�̃Q�C���l�i1�`16�j
  */
  static uint32 mTunerBBGain;

  /**
  * ini�t�@�C������ݒ�l��ǂݍ��ށB
  */
  static void readIniFile(HMODULE hModule);

  /**
  * Mutex�ɂ��r������p�N���X�B
  */
  class MLock {
  public:
    MLock() :mMutex(nullptr) {
      static constexpr LPCTSTR mMutexName = _T("Global\\PT3SDKSimulator");
      mMutex = ::CreateMutex(nullptr, FALSE, mMutexName);
      WaitForSingleObject(mMutex, INFINITE);
    }

    ~MLock() {
      ::ReleaseMutex(mMutex);
    }
  private:
    HANDLE mMutex;
  };

  /**
  * CriticalSection�ɂ��r������p�N���X�B
  */
  class CSLock {
  public:
    CSLock(LPCRITICAL_SECTION cSPointer) :mCSPointer(cSPointer) {
      ::EnterCriticalSection(mCSPointer);
    }

    ~CSLock() {
      ::LeaveCriticalSection(mCSPointer);
    }
  private:
    LPCRITICAL_SECTION  mCSPointer;
  };

private:

  BDUtil() = default;
  virtual ~BDUtil() = default;
  BDUtil(const BDUtil&) = delete;
  BDUtil(BDUtil&&) = delete;
  BDUtil& operator =(const BDUtil&) = delete;
  BDUtil& operator =(BDUtil&&) = delete;

};

}
}
