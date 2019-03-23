#pragma once

#include <EARTH_PT3.h>

namespace EARTH {
namespace PT {

/**
* 共通ユーティリティークラス。
*/
class BDUtil {

public:

  /**
  * デバイスの開始番号（0～9）
  */
  static uint32 mDeviceMinIndex;

  /**
  * チャネル切り替え時からTSデータ転送開始までの待ち時間（msec）
  */
  static uint32 mSetFrequencyDelay[Device::ISDB_COUNT];

  /**
  * BS/CS（ISDB-S）チューナーのゲイン値（1～16）
  */
  static uint32 mTunerBBGain;

  /**
  * iniファイルから設定値を読み込む。
  */
  static void readIniFile(HMODULE hModule);

  /**
  * Mutexによる排他制御用クラス。
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
  * CriticalSectionによる排他制御用クラス。
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
