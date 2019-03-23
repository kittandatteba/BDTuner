#include "stdafx.h"
#include "BDUtil.h"

namespace EARTH {
namespace PT {


uint32 BDUtil::mDeviceMinIndex = 0;

uint32 BDUtil::mSetFrequencyDelay[] = { 300, 100 };

uint32 BDUtil::mTunerBBGain = 8;


void BDUtil::readIniFile(HMODULE hModule) {

  // iniファイルのパスを設定
  TCHAR iniFilePath[MAX_PATH] = {};
  ::GetModuleFileName(hModule, iniFilePath, MAX_PATH);
  size_t baseSize = ::_tcslen(iniFilePath) - 4;
  ::_tcsncpy_s(&iniFilePath[baseSize], MAX_PATH - baseSize, _T(".ini"), 4);

  // iniファイルの設定値を取得
  mDeviceMinIndex = ::GetPrivateProfileInt(_T("SET"), _T("DEVICE_MIN_INDEX"), 0, iniFilePath);
  mSetFrequencyDelay[Device::ISDB_S] = ::GetPrivateProfileInt(_T("SET"), _T("SET_FREQUENCY_DELAY_S"), 300, iniFilePath);
  mSetFrequencyDelay[Device::ISDB_T] = ::GetPrivateProfileInt(_T("SET"), _T("SET_FREQUENCY_DELAY_T"), 100, iniFilePath);
  mTunerBBGain = ::GetPrivateProfileInt(_T("SET"), _T("TUNER_BBGAIN_S"), 8, iniFilePath);
}


}
}

