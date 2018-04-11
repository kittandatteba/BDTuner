#include "stdafx.h"
#include "JtagImpl.h"


JtagImpl::JtagImpl()
  : m_UsbDevice(new CCyUSBDevice)
  , m_EpCtrlIn(nullptr)
  , m_EpCtrlOut(nullptr) {}


void JtagImpl::open() {
  // ファームウェアのロード
  loadFW(0);

  // Controlエンドポイントの取得
  m_EpCtrlIn = static_cast<CCyControlEndPoint*>(m_UsbDevice->EndPointOf(EPA_CTL_IN));
  m_EpCtrlOut = static_cast<CCyControlEndPoint*>(m_UsbDevice->EndPointOf(EPA_CTL_OUT));
  if (m_EpCtrlIn == nullptr || m_EpCtrlOut == nullptr) {
    throw std::runtime_error("Can not get Control EndPoint.");
  }

  // Controlエンドポイントにリセットをかけておく
  m_EpCtrlIn->Reset();
  m_EpCtrlOut->Reset();
}


int32_t JtagImpl::write(uint8_t * buffer, int32_t length) {

  UCHAR	cmd[64];
  LONG	cmd_len = 0;

  for (int32_t i = 0; i < length; i++) {
    cmd[cmd_len++] = buffer[i];
  }
  if (!m_EpCtrlOut->XferData(cmd, cmd_len)) {
    throw std::runtime_error("CCyControlEndPoint::XferData() fail.");
  }
  return cmd_len;

}


int32_t JtagImpl::read(uint8_t * buffer, int32_t length) {

  UCHAR	cmd[64];
  LONG	cmd_len = length;

  if (!m_EpCtrlIn->XferData(cmd, cmd_len)) {
    throw std::runtime_error("CCyControlEndPoint::XferData() fail.");
  }

  length = 0;
  for (int32_t i = 0; i < cmd_len; i++) {
    buffer[length++] = cmd[i];
  }
  return length;

}


void JtagImpl::loadFW(uint8_t id) {

  uint8_t devCnt = m_UsbDevice->DeviceCount();
  bool find = false;

  for (uint8_t i = 0; i < devCnt; i++) {

    // デバイスのオープン（失敗の場合はリセット後にリトライ）
    if (!m_UsbDevice->Open(i)) {
      m_UsbDevice->Reset();
      m_UsbDevice->Open(i);
    }

    if (m_UsbDevice->IsOpen()) {
      if ((m_UsbDevice->BcdDevice & 0xFF00U) == 0xA000U || (m_UsbDevice->BcdDevice & 0xFF00U) == 0xFF00U) {
        // BcdDevice: 0xA0** (Before ReNumeration)
        // BcdDevice: 0xFF** (After  ReNumeration)
        if ((m_UsbDevice->VendorID == 0x04B4U && m_UsbDevice->ProductID == 0x8613U) ||
          (m_UsbDevice->VendorID == 0x04B4U && m_UsbDevice->ProductID == 0x1004U)) {
          // VendorID:  0x04B4
          // ProductID: 0x8613 (Before ReNumeration)
          // ProductID: 0x1004 (After  ReNumeration)
          break;
        }
      }
      //前回openしたデバイスの情報が残るので消しておく
      m_UsbDevice->Manufacturer[0] = 0;
      m_UsbDevice->Close();
    }
  }

  // デバイスのオープン失敗の場合はエラー
  if (!m_UsbDevice->IsOpen()) {
    throw std::runtime_error("CCyUSBDevice::Open() fail.");
  }

  // 必要なファームがロード済みの場合スキップ
  if (::wcscmp(m_UsbDevice->Manufacturer, FX2FW_NAME) == 0) {
    return;
  }

  constexpr uint32_t FX2FW_SIZE = sizeof(FX2FW_DATA) / sizeof(UCHAR);
  UCHAR fwData[FX2FW_SIZE];
  ::CopyMemory(fwData, FX2FW_DATA, sizeof(fwData));

  // ファームウェアダウンロードのためCPU（8051）をリセット状態にする
  m_UsbDevice->ControlEndPt->ReqCode = 0xA0U;	// Firmware Load
  m_UsbDevice->ControlEndPt->Value = 0xE600U;	// CPUCS register
  m_UsbDevice->ControlEndPt->Index = 0;
  UCHAR writeData[] = { 0x01U };
  LONG writeLength = sizeof(writeData);
  m_UsbDevice->ControlEndPt->Write(writeData, writeLength);

  UCHAR* fwIndex = fwData + 8;
  uint16_t length, address;
  constexpr UCHAR descriptor[] = { 0xB4U, 0x04U, 0x04U, 0x10U, 0x00U, 0x00U };

  for (;;) {
    length = (fwIndex[0] << 8) | fwIndex[1];
    address = (fwIndex[2] << 8) | fwIndex[3];
    fwIndex += 4;

    // FWのデバイスデスクリプタ VendorID:0x04B4 ProductID:0x1004 BcdDevice:0x0000を見つけ出し、
    // idに適合するBcdDevice(0xFF**)に書き換える
    UCHAR* last = fwIndex + (length & 0x7FFFU);
    UCHAR* find = std::search(fwIndex, last, descriptor, descriptor + sizeof(descriptor));
    if (find < last) {
      find[4] = id;
      find[5] = 0xFFU;
    }

    // ベンダリクエスト'A0'を使用して8051に書き込む
    m_UsbDevice->ControlEndPt->Value = address;
    LONG fwLength = length & 0x7FFFU;
    m_UsbDevice->ControlEndPt->Write(fwIndex, fwLength);

    //最終（リセット）
    if (length & 0x8000U) {
      break;
    }
    fwIndex += length;
  }

  m_UsbDevice->Close();
  ::Sleep(RENUM_RETRY_INTRVAL);

  //再起動後のファームに接続
  for (uint32_t count = 0; count < RENUM_RETRY_COUNT; count++) {

    for (uint8_t i = 0; i < devCnt; i++) {

      // デバイスのオープン（失敗の場合はリセット後にリトライ）
      if (!m_UsbDevice->Open(i)) {
        m_UsbDevice->Reset();
        m_UsbDevice->Open(i);
      }

      if (m_UsbDevice->IsOpen()) {
        if ((m_UsbDevice->BcdDevice == 0xFF00U + id) &&
          (m_UsbDevice->VendorID == 0x04B4U) &&
          (m_UsbDevice->ProductID == 0x1004U)) {
          // BcdDevice: 0xFF** (After ReNumeration)
          // VendorID:  0x04B4
          // ProductID: 0x1004 (After ReNumeration)
          return;
        }
        m_UsbDevice->Close();
      }
    }

    ::Sleep(RENUM_RETRY_INTRVAL);
  }

  // 接続失敗の場合はエラー
  throw std::runtime_error("CCyUSBDevice::Open() fail.");
}
