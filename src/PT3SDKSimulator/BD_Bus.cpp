#include "stdafx.h"
#include <memory>
#include <CyAPI.h>
#include "BD_Device.h"
#include "BD_Bus.h"

namespace EARTH {
namespace PT {


status BD_Bus::GetInstance(Bus **bus) {

  if (bus == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" bus=") << std::hex << bus);
    return STATUS_INVALID_PARAM_ERROR;
  }

  BD_Bus *bdBus;

  // BD_Bus�N���X�̃C���X�^���X����
  try {
    bdBus = new BD_Bus();
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


status BD_Bus::Delete() {

  delete this;
  return STATUS_OK;
}


status BD_Bus::GetVersion(uint32 *version) const {

  if (version == nullptr) {
    TRACE_F(_T("Invalid param error. version=") << std::hex << version);
    return STATUS_INVALID_PARAM_ERROR;
  }

  // ���ۂ�SDK�Ɠ����o�[�W������ԋp
  *version = 0x200;

  return STATUS_OK;
}


status BD_Bus::Scan(DeviceInfo *deviceInfoPtr, uint32 *deviceInfoCount) {

  if (deviceInfoPtr == nullptr || deviceInfoCount == nullptr) {
    TRACE_F(_T("Invalid param error. deviceInfoPtr=") << std::hex << deviceInfoPtr
      << _T(" deviceInfoCount=") << std::hex << deviceInfoCount);
    return STATUS_INVALID_PARAM_ERROR;
  }

  try {
    std::unique_ptr<CCyUSBDevice> usbDev(new CCyUSBDevice(NULL, CYUSBDRV_GUID, false));
    uint8 devices = usbDev->DeviceCount();
    uint8 cnt = 0;

    for (uint8 i = 0; i < devices; i++) {
      // ������ɒB���Ă����猟���I��
      if (static_cast<uint32>(cnt / 2) >= *deviceInfoCount) break;

      usbDev->Open(i);
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

            // PT�i4TS�j�̎d�l�ɍ��킹�邽�߁A2��1�g�ň���
            if (cnt % 2 == 0) {
              // Bus:1�ڂ̃f�o�C�X��USB�o�X�A�h���X
              deviceInfoPtr[cnt / 2].Bus = usbDev->USBAddress;

              // Slot:2�ڂ̃f�o�C�X(0�ŏ�����)
              deviceInfoPtr[cnt / 2].Slot = 0;

              // �ȉ��̓_�~�[�l
              deviceInfoPtr[cnt / 2].Function = 0;	// PCI Function�ԍ� (����:0)
              deviceInfoPtr[cnt / 2].PTn = 3;			// �i�� (PT3:3)
            } else {
              // Slot:2�ڂ̃f�o�C�X��USB�o�X�A�h���X
              deviceInfoPtr[cnt / 2].Slot = usbDev->USBAddress;
            }
            cnt++;
          }
        }
        usbDev->Close();
      }
    }

    // ���������f�o�C�X�����Z�b�g
    *deviceInfoCount = (cnt + 1) / 2;

  } catch (std::bad_alloc&) {
    TRACE_F(_T("Out of memory error."));
    return STATUS_OUT_OF_MEMORY_ERROR;

  } catch (...) {
    TRACE_F(_T("General error."));
    return STATUS_GENERAL_ERROR;
  }

  return STATUS_OK;
}


status BD_Bus::NewDevice(const DeviceInfo *deviceInfoPtr, Device **device, Device_ **device_) {

  // BD_Device�N���X�̃C���X�^���X����
  return BD_Device::GetInstance(deviceInfoPtr, device);
}


BD_Bus::BD_Bus() {

  ::setlocale(LC_ALL, "japanese");
}


}
}


// DLL�̊O�����J�֐��i�C���X�^���X�����j
extern "C" __declspec(dllexport) EARTH::status _(EARTH::PT::Bus **bus);

EARTH::status _(EARTH::PT::Bus **bus) {

  return EARTH::PT::BD_Bus::GetInstance(bus);
}
