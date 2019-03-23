#include "stdafx.h"
#include <memory>
#include <CyAPI.h>
#include "BDUtil.h"
#include "BDDevice.h"
#include "BDBus.h"

namespace EARTH {
namespace PT {


status BDBus::GetInstance(Bus **bus) {

  if (bus == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" bus=") << std::hex << bus);
    return STATUS_INVALID_PARAM_ERROR;
  }

  BDBus *bdBus;

  // BD_Busクラスのインスタンス生成
  try {
    bdBus = new BDBus();
    *bus = bdBus;

  } catch (std::bad_alloc&) {
    TRACE_F(_T("Out of memory error."));
    return STATUS_OUT_OF_MEMORY_ERROR;

  } catch (...) {
    TRACE_F(_T("General error."));
    return STATUS_GENERAL_ERROR;
  }

  return STATUS_OK;
}


status BDBus::Delete() {

  delete this;
  return STATUS_OK;
}


status BDBus::GetVersion(uint32 *version) const {

  if (version == nullptr) {
    TRACE_F(_T("Invalid param error. version=") << std::hex << version);
    return STATUS_INVALID_PARAM_ERROR;
  }

  // 実際のSDKと同じバージョンを返却
  *version = 0x200;

  return STATUS_OK;
}


status BDBus::Scan(DeviceInfo *deviceInfoPtr, uint32 *deviceInfoCount) {

  if (deviceInfoPtr == nullptr || deviceInfoCount == nullptr) {
    TRACE_F(_T("Invalid param error. deviceInfoPtr=") << std::hex << deviceInfoPtr
      << _T(" deviceInfoCount=") << std::hex << deviceInfoCount);
    return STATUS_INVALID_PARAM_ERROR;
  }

  BDUtil::MLock lock;

  try {
    std::unique_ptr<CCyUSBDevice> usbDev(new CCyUSBDevice(NULL, CYUSBDRV_GUID, false));
    uint8 devices = usbDev->DeviceCount();
    uint8 cnt = 0;
    uint8 findCnt = 0;

    for (uint32 i = 0; i < *deviceInfoCount; i++) {

      // 初期化
      deviceInfoPtr[i].Bus = 0;       // Bus
      deviceInfoPtr[i].Slot = 0;      // Slot
      deviceInfoPtr[i].Function = 0;  // PCI Function番号 (正常:0)
      deviceInfoPtr[i].PTn = 3;       // 品番 (PT3:3)

      if (i < BDUtil::mDeviceMinIndex) {
        continue;
      }

      while (cnt < devices) {
        usbDev->Open(cnt);
        if (usbDev->IsOpen()) {

          // BcdDevice: 0xa0** (Before ReNumeration)
          // BcdDevice: 0xff** (After  ReNumeration)
          if ((usbDev->BcdDevice & 0xff00U) == 0xa000U
            || (usbDev->BcdDevice & 0xff00U) == 0xff00U) {

            // VendorID:  0x04b4
            // ProductID: 0x8613 (Before ReNumeration)
            // ProductID: 0x1004 (After  ReNumeration)
            if (usbDev->VendorID == 0x04b4U
              && (usbDev->ProductID == 0x8613U || usbDev->ProductID == 0x1004U)) {

              // PT（4TS）の仕様に合わせるため、2個1組で扱う
              if (deviceInfoPtr[i].Bus == 0) {
                // Bus:1個目のデバイスのデバイス番号
                deviceInfoPtr[i].Bus = ++findCnt;
              } else {
                // Slot:2個目のデバイスのデバイス番号
                deviceInfoPtr[i].Slot = ++findCnt;
              }
            }
          }
          usbDev->Close();
        }
        cnt++;

        if (deviceInfoPtr[i].Slot != 0) {
          break;
        }
      }
    }

    // 見つかったデバイス数をセット
    if (findCnt > 0) {
      *deviceInfoCount = (findCnt + 1) / 2 + BDUtil::mDeviceMinIndex;
    } else {
      *deviceInfoCount = 0;
    }

  } catch (std::bad_alloc&) {
    TRACE_F(_T("Out of memory error."));
    return STATUS_OUT_OF_MEMORY_ERROR;

  } catch (...) {
    TRACE_F(_T("General error."));
    return STATUS_GENERAL_ERROR;
  }

  return STATUS_OK;
}


status BDBus::NewDevice(const DeviceInfo *deviceInfoPtr, Device **device, Device_ **device_) {

  // BD_Deviceクラスのインスタンス生成
  return BDDevice::GetInstance(deviceInfoPtr, device);
}


BDBus::BDBus() {

  ::setlocale(LC_ALL, "japanese");
}


}
}


// DLLの外部公開関数（インスタンス生成）
extern "C" __declspec(dllexport) EARTH::status _(EARTH::PT::Bus **bus);

EARTH::status _(EARTH::PT::Bus **bus) {

  return EARTH::PT::BDBus::GetInstance(bus);
}
