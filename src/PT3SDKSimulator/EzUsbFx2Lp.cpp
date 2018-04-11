#include "stdafx.h"
#include <process.h>
#include <algorithm>
#include "InternalException.h"
#include "IReceiver.h"
#include "EzUsbFx2Lp.h"

namespace EARTH {
namespace PT {


EzUsbFx2Lp::EzUsbFx2Lp()
  : mUsbDevice(new CCyUSBDevice(NULL, CYUSBDRV_GUID, false))
  , mEpCtrlIn(nullptr)
  , mEpCtrlOut(nullptr)
  , mEpDataS(nullptr)
  , mEpDataT(nullptr) {

  // クリティカルセクションの初期化
  ::InitializeCriticalSection(&mCriticalSection);
}


EzUsbFx2Lp::~EzUsbFx2Lp() {

  // クリティカルセクションの削除
  ::DeleteCriticalSection(&mCriticalSection);
}


void EzUsbFx2Lp::init(uint8 usbadr) {
  // ファームウェアのロード
  loadFW(usbadr);

  // Controlエンドポイントの取得
  mEpCtrlIn = static_cast<CCyControlEndPoint*>(mUsbDevice->EndPointOf(EPA_CTL_IN));
  mEpCtrlOut = static_cast<CCyControlEndPoint*>(mUsbDevice->EndPointOf(EPA_CTL_OUT));
  if (mEpCtrlIn == nullptr || mEpCtrlOut == nullptr) {
    TRACE_F(_T("Can not get Control EndPoint."
      << " mEpCtrlIn=") << std::hex << mEpCtrlIn
      << _T(" mEpCtrlOut=") << std::hex << mEpCtrlOut);
    throw InternalException(STATUS_GENERAL_ERROR, "Can not get Control EndPoint.");
  }

  // Controlエンドポイントにリセットをかけておく
  mEpCtrlIn->Reset();
  mEpCtrlOut->Reset();

  // Bulkエンドポイントの取得
  mEpDataS = static_cast<CCyBulkEndPoint*>(mUsbDevice->EndPointOf(EPA_DATA_S));
  mEpDataT = static_cast<CCyBulkEndPoint*>(mUsbDevice->EndPointOf(EPA_DATA_T));

  if (mEpDataS == nullptr || mEpDataT == nullptr) {
    TRACE_F(_T("Can not get Bulk EndPoint."
      << " mEpDataS=") << std::hex << mEpDataS
      << _T(" mEpDataT=") << std::hex << mEpDataT);
    throw InternalException(STATUS_GENERAL_ERROR, "Can not get Bulk EndPoint.");
  }

  // Bulkエンドポイントにリセットをかけておく
  mEpDataS->Reset();
  mEpDataT->Reset();
}


uint32 EzUsbFx2Lp::xfer(Message* msg, uint32 len) {
  UCHAR	cmd[I2C_MSG_MAX];
  LONG	cmd_len = 0;
  LONG	total_len = 0;

  GetLock lock(&mCriticalSection);

  for (uint32 i = 0; i < len; i++) {
    cmd_len = 0;
    if (msg[i].ope == Operation::READ) {
      // I2Cメッセージを受信
      cmd[cmd_len++] = CMD_I2C_READ;
      cmd[cmd_len++] = msg[i].addr;
      cmd[cmd_len++] = msg[i].len;
      if (!mEpCtrlOut->XferData(cmd, cmd_len)) {
        TRACE_F(_T("CCyControlEndPoint::XferData() fail. cmd=") << std::hex << cmd
          << _T(" cmd_len=") << std::dec << cmd_len);
        throw InternalException(STATUS_GENERAL_ERROR, "CCyControlEndPoint::XferData() fail.");
      }

      cmd_len = msg[i].len;
      if (!mEpCtrlIn->XferData(msg[i].buf, cmd_len)) {
        TRACE_F(_T("CCyControlEndPoint::XferData() fail. msg[") << std::dec << i
          << _T("].buf=") << std::hex << msg[i].buf
          << _T(" cmd_len=") << std::dec << cmd_len);
        throw InternalException(STATUS_GENERAL_ERROR, "CCyControlEndPoint::XferData() fail.");
      }

      total_len = cmd_len;

    } else {
      // I2Cメッセージを送信
      cmd[cmd_len++] = CMD_I2C_WRITE;
      cmd[cmd_len++] = msg[i].addr;
      cmd[cmd_len++] = msg[i].len;

      for (unsigned int j = 0; j < msg[i].len; j++) {
        cmd[cmd_len++] = msg[i].buf[j];
      }

      if (!mEpCtrlOut->XferData(cmd, cmd_len)) {
        TRACE_F(_T("CCyControlEndPoint::XferData() fail. cmd=") << std::hex << cmd
          << _T(" cmd_len=") << std::dec << cmd_len);
        throw InternalException(STATUS_GENERAL_ERROR, "CCyControlEndPoint::XferData() fail.");
      }

      total_len += cmd_len - 3;
    }
  }

  return total_len;
}


void EzUsbFx2Lp::startXferThread(Device::ISDB isdb, uint32 tunerIndex, IReceiver * receiver) {

  mThreadInfo[isdb].isdb = isdb;
  mThreadInfo[isdb].tunerIndex = tunerIndex;
  mThreadInfo[isdb].running = true;
  mThreadInfo[isdb].endPoint = (isdb == Device::ISDB::ISDB_S) ? mEpDataS : mEpDataT;
  mThreadInfo[isdb].receiver = receiver;
  mThreadInfo[isdb].threadHandle = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, processXferThread, &(mThreadInfo[isdb]), 0, nullptr));
}


void EzUsbFx2Lp::stopXferThread(Device::ISDB isdb) {

  mThreadInfo[isdb].running = false;
  WaitForSingleObject(mThreadInfo[isdb].threadHandle, INFINITE);
  CloseHandle(mThreadInfo[isdb].threadHandle);
}


void EzUsbFx2Lp::setXferEnable(Device::ISDB isdb, bool enabled) {

  UCHAR cmd[3];
  LONG cmd_len = 0;

  GetLock lock(&mCriticalSection);

  if (enabled) {
    cmd[cmd_len++] = (isdb == Device::ISDB_S) ? CMD_EP2IN_START : CMD_EP6IN_START;
  } else {
    cmd[cmd_len++] = (isdb == Device::ISDB_S) ? CMD_EP2IN_STOP : CMD_EP6IN_STOP;
  }

  // 制御データを送信
  bool ret = mEpCtrlOut->XferData(cmd, cmd_len);

  if (!ret) {
    TRACE_F(_T("CCyControlEndPoint::XferData() fail. cmd=") << std::hex << cmd
      << _T(" cmd_len=") << std::dec << cmd_len);
    throw InternalException(STATUS_GENERAL_ERROR, "CCyControlEndPoint::XferData() fail.");
  }
}


unsigned EzUsbFx2Lp::processXferThread(void *arg) {

  ThreadInfo *threadInfo = static_cast<ThreadInfo*>(arg);

  std::unique_ptr<OVERLAPPED[]> overlapped(new OVERLAPPED[TS_QUEUE_NUM]);
  std::unique_ptr<UCHAR[][TS_QUEUE_SIZE]> data(new UCHAR[TS_QUEUE_NUM][TS_QUEUE_SIZE]);
  std::unique_ptr<PUCHAR[]> context(new PUCHAR[TS_QUEUE_NUM]);

  BOOL ret = SetThreadPriority(threadInfo->threadHandle, THREAD_PRIORITY_HIGHEST);

  uint32 count = 0;
  LONG size = 0;

  for (count = 0; count < TS_QUEUE_NUM; count++) {
    overlapped[count].hEvent = CreateEvent(NULL, false, false, NULL);
    context[count] = threadInfo->endPoint->BeginDataXfer(data[count], TS_QUEUE_SIZE, &(overlapped[count]));
  }

  for (count = 0;;) {
    if (!threadInfo->running) {
      break;
    }

    if (!threadInfo->endPoint->WaitForXfer(&(overlapped[count]), INFINITE)) {
    }

    size = TS_QUEUE_SIZE;
    if (threadInfo->endPoint->FinishDataXfer(data[count], size, &(overlapped[count]), context[count])) {
      threadInfo->receiver->receive(threadInfo->isdb, threadInfo->tunerIndex, data[count], size);
    }

    context[count] = threadInfo->endPoint->BeginDataXfer(data[count], TS_QUEUE_SIZE, &(overlapped[count]));

    if (++count >= TS_QUEUE_NUM) {
      count = 0;
    }
  }

  uint32 currentNum = count;
  do {
    threadInfo->endPoint->Abort();
    threadInfo->endPoint->WaitForXfer(&(overlapped[count]), INFINITE);
    size = TS_QUEUE_SIZE;
    threadInfo->endPoint->FinishDataXfer(data[count], size, &(overlapped[count]), context[count]);
    CloseHandle(overlapped[count].hEvent);

    if (++count >= TS_QUEUE_NUM) {
      count = 0;
    }
  } while (count != currentNum);

  return 0;
}

void EzUsbFx2Lp::loadFW(uint8 usbadr) {

  // デバイスの数
  uint8 devices = mUsbDevice->DeviceCount();

  // USBアドレスがスキャン時と一致するデバイスをオープン
  for (uint8 i = 0; i < devices; i++) {
    mUsbDevice->Open(i);
    if (mUsbDevice->IsOpen()) {
      if (mUsbDevice->USBAddress == usbadr) {
        break;
      }
      mUsbDevice->Close();
    }
  }

  // デバイスのオープン失敗の場合はエラー
  if (!mUsbDevice->IsOpen()) {
    TRACE_F(_T("CCyUSBDevice::Open() fail.")
      << _T(" usbadr=") << std::dec << usbadr);
    throw InternalException(STATUS_GENERAL_ERROR, "CCyUSBDevice::Open() fail.");
  }

  // 必要なファームがロード済みの場合スキップ
  if (::wcscmp(mUsbDevice->Manufacturer, FX2FW_NAME) == 0) {
    return;
  }

  constexpr uint32_t FX2FW_SIZE = sizeof(FX2FW_DATA) / sizeof(UCHAR);
  UCHAR fwData[FX2FW_SIZE];
  ::CopyMemory(fwData, FX2FW_DATA, sizeof(fwData));

  // ファームウェアダウンロードのためCPU（8051）をリセット状態にする
  mUsbDevice->ControlEndPt->ReqCode = 0xa0U;	// Firmware Load
  mUsbDevice->ControlEndPt->Value = 0xe600U;	// CPUCS register
  mUsbDevice->ControlEndPt->Index = 0;
  UCHAR writeData[] = { 0x01U };
  LONG writeLength = sizeof(writeData);
  mUsbDevice->ControlEndPt->Write(writeData, writeLength);

  UCHAR* fwIndex = fwData + 8;
  uint16_t length, address;
  constexpr UCHAR descriptor[] = { 0xb4U, 0x04U, 0x04U, 0x10U, 0x00U, 0x00U };

  for (;;) {
    length = (fwIndex[0] << 8) | fwIndex[1];
    address = (fwIndex[2] << 8) | fwIndex[3];
    fwIndex += 4;

    // FWのデバイスデスクリプタ VendorID:0x04b4 ProductID:0x1004 BcdDevice:0x0000を見つけ出し、
    // devnumに適合するBcdDevice(0xff**)に書き換える
    UCHAR* last = fwIndex + (length & 0x7fffU);
    UCHAR* find = std::search(fwIndex, last, descriptor, descriptor + sizeof(descriptor));
    if (find < last) {
      find[4] = usbadr;
      find[5] = 0xffU;
    }

    // ベンダリクエスト'A0'を使用して8051に書き込む
    mUsbDevice->ControlEndPt->Value = address;
    LONG fwLength = length & 0x7fffU;
    mUsbDevice->ControlEndPt->Write(fwIndex, fwLength);

    //最終（リセット）
    if (length & 0x8000U) {
      break;
    }
    fwIndex += length;
  }

  mUsbDevice->Close();
  ::Sleep(RENUM_RETRY_INTRVAL);

  //再起動後のファームに接続
  bool find = false;
  for (uint32_t cnt = 0; cnt < RENUM_RETRY_COUNT; cnt++) {
    for (uint8 i = 0; i < devices; i++) {
      mUsbDevice->Open(i);
      if (mUsbDevice->IsOpen()) {
        // BcdDevice: 0xff** (After ReNumeration)
        // VendorID:  0x04b4
        // ProductID: 0x1004 (After ReNumeration)
        if (mUsbDevice->BcdDevice == (0xff00U + usbadr)
          && mUsbDevice->VendorID == 0x04b4U
          && mUsbDevice->ProductID == 0x1004U) {
          find = true;
          break;
        }
        mUsbDevice->Close();
      }
    }
    if (find) {
      break;
    }
    ::Sleep(RENUM_RETRY_INTRVAL);
  }

  // デバイスがオープンされていない場合はエラー
  if (!mUsbDevice->IsOpen()) {
    TRACE_F(_T("CCyUSBDevice::Open() fail.")
      << _T(" usbadr=") << std::dec << usbadr);
    throw InternalException(STATUS_GENERAL_ERROR, "CCyUSBDevice::Open() fail.");
  }
}

}
}
