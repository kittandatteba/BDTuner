#include "stdafx.h"
#include "InternalException.h"
#include "EzUsbFx2Lp.h"
#include "Jdts290532S.h"
#include "Jdts290532T.h"
#include "BD_Device.h"

namespace EARTH {
namespace PT {


status BD_Device::GetInstance(const Bus::DeviceInfo *deviceInfo, Device **device) {

  if ((deviceInfo == NULL) || (device == NULL)) {
    TRACE_F(_T("Invalid param error.")
      << _T(" deviceInfo=") << std::hex << deviceInfo
      << _T(" device=") << std::hex << device);
    return STATUS_INVALID_PARAM_ERROR;
  }

  BD_Device *bdDevice;

  // BD_Deviceクラスのインスタンス生成
  try {
    bdDevice = new BD_Device(deviceInfo);
    *device = bdDevice;

  } catch (std::bad_alloc&) {
    TRACE_F(_T("Out of memory error."));
    return STATUS_OUT_OF_MEMORY_ERROR;

  } catch (...) {
    TRACE_F(_T("General error."));
    return STATUS_GENERAL_ERROR;
  }

  return STATUS_OK;
}


status BD_Device::Delete() {

  delete this;
  return STATUS_OK;
}


status BD_Device::Open() {

  if (!mUsbAddress[0]) {
    TRACE_F(_T("General error.")
      << _T(" mUsbAddress[0]=") << std::hex << mUsbAddress[0]);
    return STATUS_GENERAL_ERROR;
  }

  status status = STATUS_OK;

  for (uint32 tunerIndex = 0; tunerIndex < DEVICE_COUNT; tunerIndex++) {
    if (!mUsbAddress[tunerIndex])
      break;

    // 既にインスタンス生成済みの場合はエラー
    if (mUsbInstance[tunerIndex]) {
      TRACE_F(_T("Device is already open error.")
        << _T(" tunerIndex=") << std::dec << tunerIndex);
      status = STATUS_DEVICE_IS_ALREADY_OPEN_ERROR;
      break;
    }

    try {
      // EzUsbFx2Lpクラスのインスタンス生成
      mUsbInstance[tunerIndex].reset(new EzUsbFx2Lp());


      // EzUsbFx2Lpの初期化
      mUsbInstance[tunerIndex]->init(mUsbAddress[tunerIndex]);


      // Jdts290532クラスのインスタンス生成
      mTunerInstance[idx(ISDB_S, tunerIndex)].reset(new Jdts290532S(mUsbInstance[tunerIndex].get()));
      mTunerInstance[idx(ISDB_T, tunerIndex)].reset(new Jdts290532T(mUsbInstance[tunerIndex].get()));

    } catch (std::bad_alloc&) {
      TRACE_F(_T("Out of memory error."));
      status = STATUS_OUT_OF_MEMORY_ERROR;
      break;

    } catch (...) {
      TRACE_F(_T("General error."));
      status = STATUS_GENERAL_ERROR;
      break;
    }
  }

  return status;
}


status BD_Device::Close() {

  for (uint32 tunerIndex = 0; tunerIndex < DEVICE_COUNT; tunerIndex++) {
    for (uint32 i = 0; i < ISDB_COUNT; i++) {
      ISDB isdb = static_cast<ISDB>(i);

      // Jdts290532クラスのインスタンス開放
      mTunerInstance[idx(isdb, tunerIndex)].reset();
    }

    // EzUsbFx2Lpクラスのインスタンス開放
    mUsbInstance[tunerIndex].reset();
  }
  return STATUS_OK;
}


status BD_Device::GetConstantInfo(ConstantInfo *info) const {

  if (info == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" info=") << std::hex << info);
    return STATUS_INVALID_PARAM_ERROR;
  }

  // ダミーの固定情報を設定する
  info->PTn = 0x03U;
  info->Version_RegisterMap = 0x01U;
  info->Version_FPGA = 0x04U;
  info->CanTransportTS = true;

  return STATUS_OK;
}


status BD_Device::SetLnbPower(LnbPower lnbPower) {

  // LNB電源制御はハードウェアに未実装のため
  // 設定値の保持のみで何の制御もしない

  mLnbPower = lnbPower;

  return STATUS_OK;
}


status BD_Device::GetLnbPower(LnbPower *lnbPower) const {

  // LNB電源制御はハードウェアに未実装のため
  // 設定値の保持のみで何の制御もしない

  if (lnbPower == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" lnbPower=") << std::hex << lnbPower);
    return STATUS_INVALID_PARAM_ERROR;
  }

  *lnbPower = mLnbPower;

  return STATUS_OK;
}


status BD_Device::SetLnbPowerWhenClose(LnbPower lnbPower) {

  // LNB電源制御はハードウェアに未実装のため
  // 設定値の保持のみで何の制御もしない

  mLnbPowerWhenClose = lnbPower;

  return STATUS_OK;
}


status BD_Device::GetLnbPowerWhenClose(LnbPower *lnbPower) const {

  // LNB電源制御はハードウェアに未実装のため
  // 設定値の保持のみで何の制御もしない

  if (lnbPower == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" lnbPower=") << std::hex << lnbPower);
    return STATUS_INVALID_PARAM_ERROR;
  }

  *lnbPower = mLnbPowerWhenClose;

  return STATUS_OK;
}


status BD_Device::InitTuner() {

  try {
    for (sint32 tunerIndex = 0; tunerIndex < DEVICE_COUNT; tunerIndex++) {

      for (uint32 i = 0; i < ISDB_COUNT; i++) {
        ISDB isdb = static_cast<ISDB>(i);

        mChannel[idx(isdb, tunerIndex)] = (isdb == ISDB_S) ? 0 : ((tunerIndex == 0) ? 70 : 71);
        mChannelOffset[idx(isdb, tunerIndex)] = 0;
        mSleep[idx(isdb, tunerIndex)] = true;

        if (mUsbAddress[tunerIndex]) {

          // チューナの初期化
          mTunerInstance[idx(isdb, tunerIndex)]->initTuner();

          // 復調ICの初期化
          mTunerInstance[idx(isdb, tunerIndex)]->initDemod();

          // デフォルトのチャンネルでチューニング
          mTunerInstance[idx(isdb, tunerIndex)]->setFrequency(mChannel[idx(isdb, tunerIndex)], mChannelOffset[idx(isdb, tunerIndex)]);

          // スリープモードに移行
          mTunerInstance[idx(isdb, tunerIndex)]->setTunerSleep(mSleep[idx(isdb, tunerIndex)]);
        }
      }
    }

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BD_Device::SetAmpPowerT(bool b) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::SetTunerSleep(ISDB isdb, uint32 tunerIndex, bool sleep) {

  try {
    validate(isdb, tunerIndex, false);

    // チャンネル値、チャンネルオフセット値を取得
    //uint32 channel;
    //sint32 offset;
    //mTunerInstance[idx(isdb, tunerIndex)]->getFrequency(&channel, &offset);

    // スリープ状態の設定
    mTunerInstance[idx(isdb, tunerIndex)]->setTunerSleep(sleep);

    // チューニング
    //mTunerInstance[idx(isdb, tunerIndex)]->setFrequency(channel, offset);

  } catch (SingleDeviceException&) {
    mSleep[idx(isdb, tunerIndex)] = sleep;

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BD_Device::GetTunerSleep(ISDB isdb, uint32 tunerIndex, bool *sleep) const {

  if (sleep == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" sleep=") << std::hex << sleep);
    return STATUS_INVALID_PARAM_ERROR;
  }

  try {
    validate(isdb, tunerIndex, false);

    // スリープ状態を取得
    mTunerInstance[idx(isdb, tunerIndex)]->getTunerSleep(sleep);

  } catch (SingleDeviceException&) {
    *sleep = mSleep[idx(isdb, tunerIndex)];

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BD_Device::SetFrequency(ISDB isdb, uint32 tunerIndex, uint32 channel, sint32 offset) {

  status status = STATUS_OK;

  try {
    validate(isdb, tunerIndex);

    mReady[idx(isdb, tunerIndex)] = false;

    mTunerInstance[idx(isdb, tunerIndex)]->setFrequency(channel, offset);

    // チャネル切り替え完了からTSデータ転送開始まで指定時間スリープ
    Sleep(FREQUENCY_SET_INTERVAL);

    mReSync[idx(isdb, tunerIndex)] = true;
    mReady[idx(isdb, tunerIndex)] = true;

  } catch (SingleDeviceException&) {
    mChannel[idx(isdb, tunerIndex)] = channel;
    mChannelOffset[idx(isdb, tunerIndex)] = offset;

  } catch (InternalException& e) {
    status = e.status();
  }

  return status;
}


status BD_Device::GetFrequency(ISDB isdb, uint32 tunerIndex, uint32 *channel, sint32 *offset) const {

  if ((channel == nullptr) || (offset == nullptr)) {
    TRACE_F(_T("Invalid param error.")
      << _T(" channel=") << std::hex << channel
      << _T(" offset=") << std::hex << offset);
    return STATUS_INVALID_PARAM_ERROR;
  }

  try {
    validate(isdb, tunerIndex);

    // チャンネル値、チャンネルオフセット値を取得
    mTunerInstance[idx(isdb, tunerIndex)]->getFrequency(channel, offset);

  } catch (SingleDeviceException&) {
    *channel = mChannel[idx(isdb, tunerIndex)];
    *offset = mChannelOffset[idx(isdb, tunerIndex)];

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BD_Device::GetFrequencyOffset(ISDB isdb, uint32 tunerIndex, sint32 *clock, sint32 *carrier) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::GetCnAgc(ISDB isdb, uint32 tunerIndex, uint32 *cn100, uint32 *currentAgc, uint32 *maxAgc) {

  if ((cn100 == nullptr) || (currentAgc == nullptr) || (maxAgc == nullptr)) {
    TRACE_F(_T("Invalid param error.")
      << _T(" cn100=") << std::hex << cn100
      << _T(" currentAgc=") << std::hex << currentAgc
      << _T(" maxAgc=") << std::hex << maxAgc);
    return STATUS_INVALID_PARAM_ERROR;
  }

  try {
    validate(isdb, tunerIndex);

    uint8 byteAgc, byteMaxAgc;
    mTunerInstance[idx(isdb, tunerIndex)]->getCn(cn100);
    mTunerInstance[idx(isdb, tunerIndex)]->getAgc(&byteAgc, &byteMaxAgc);
    *currentAgc = byteAgc;
    *maxAgc = byteMaxAgc;

  } catch (SingleDeviceException&) {
    *cn100 = 0;
    *currentAgc = 0;
    *maxAgc = 255;

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BD_Device::GetRFLevel(uint32 tunerIndex, float *level) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::SetIdS(uint32 tunerIndex, uint32 id) {

  try {
    validate(ISDB_S, tunerIndex);

    mTunerInstance[idx(ISDB_S, tunerIndex)]->setId(id);

  } catch (SingleDeviceException&) {
    mTsId[idx(ISDB_S, tunerIndex)] = id;

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BD_Device::GetIdS(uint32 tunerIndex, uint32 *id) {

  try {
    validate(ISDB_S, tunerIndex);

    mTunerInstance[idx(ISDB_S, tunerIndex)]->getId(id);

  } catch (SingleDeviceException&) {
    *id = mTsId[idx(ISDB_S, tunerIndex)];

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BD_Device::SetInnerErrorRateLayer(ISDB isdb, uint32 tunerIndex, LayerIndex layerIndex) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::GetInnerErrorRate(ISDB isdb, uint32 tunerIndex, ErrorRate *errorRate) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::GetCorrectedErrorRate(ISDB isdb, uint32 tunerIndex, LayerIndex layerIndex, ErrorRate *errorRate) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::ResetCorrectedErrorCount(ISDB isdb, uint32 tunerIndex) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::GetErrorCount(ISDB isdb, uint32 tunerIndex, uint32 *count) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::GetTmccS(uint32 tunerIndex, TmccS *tmcc) {

  if (tmcc == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" tmcc=") << std::hex << tmcc);
    return STATUS_INVALID_PARAM_ERROR;
  }

  try {
    validate(ISDB_S, tunerIndex);

    mTunerInstance[idx(ISDB_S, tunerIndex)]->getTmcc(tmcc);

  } catch (SingleDeviceException&) {
    // ここは2個目のチューナーが存在しない場合はエラーにする
    return STATUS_GENERAL_ERROR;

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BD_Device::GetLayerS(uint32 tunerIndex, LayerS *layer) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::GetTmccT(uint32 tunerIndex, TmccT *tmcc) {

  if (tmcc == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" tmcc=") << std::hex << tmcc);
    return STATUS_INVALID_PARAM_ERROR;
  }

  try {
    validate(ISDB_T, tunerIndex);

    mTunerInstance[idx(ISDB_T, tunerIndex)]->getTmcc(tmcc);

  } catch (SingleDeviceException&) {
    // ここは2個目のチューナーが存在しない場合はエラーにする
    return STATUS_GENERAL_ERROR;

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BD_Device::SetLayerEnable(ISDB isdb, uint32 tunerIndex, LayerMask layerMask) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::GetLayerEnable(ISDB isdb, uint32 tunerIndex, LayerMask *layerMask) const {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::SetTsPinsMode(ISDB isdb, uint32 index, const TsPinsMode *mode) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::GetTsPinsLevel(ISDB isdb, uint32 index, TsPinsLevel *level) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::GetTsSyncByte(ISDB isdb, uint32 index, uint8 *syncByte) {

  *syncByte = TS_SYNC_BYTE;
  return STATUS_OK;
}


status BD_Device::SetRamPinsMode(RamPinsMode mode) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::LockBuffer__Obsolated__(void *ptr, uint32 size, void **handle) {

  // 未実装エラーを返却
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BD_Device::LockBuffer(void *ptr, uint32 size, TransferDirection direction, void **handle) {

  uint64 addess = reinterpret_cast<uint64>(ptr);
  if ((addess & 0x07) != 0) {
    TRACE_F(_T("Invalid param error(lower 3bit not 0).")
      << _T(" ptr=") << std::hex << ptr);
    return STATUS_INVALID_PARAM_ERROR;
  }

  try {
    // 転送用バッファ情報を生成
    BufferDesc* bufferDesc = new BufferDesc();
    bufferDesc->infoCount = 1;
    bufferDesc->bufferInfo.Address = addess;
    bufferDesc->bufferInfo.Size = size;
    bufferDesc->isdb = ISDB_COUNT;
    bufferDesc->tunerIndex = DEVICE_COUNT;
    bufferDesc->sync = false;
    bufferDesc->next = nullptr;

    // メンバ変数に詰める
    mBufferDescs[addess] = std::unique_ptr<BufferDesc>(bufferDesc);

    *handle = bufferDesc;

  } catch (std::bad_alloc&) {
    return STATUS_OUT_OF_MEMORY_ERROR;
  }

  return STATUS_OK;
}


status BD_Device::SyncBufferCpu(void *handle) {

  return STATUS_OK;
}


status BD_Device::SyncBufferIo(void *handle) {

  return STATUS_OK;
}


status BD_Device::UnlockBuffer(void *handle) {

  return STATUS_OK;
}


status BD_Device::GetBufferInfo(void *handle, const BufferInfo **infoTable, uint32 *infoCount) {

  BufferDesc* bufferDesc = static_cast<BufferDesc*>(handle);
  *infoTable = &(bufferDesc->bufferInfo);
  *infoCount = bufferDesc->infoCount;

  return STATUS_OK;
}


status BD_Device::SetTransferPageDescriptorAddress(ISDB isdb, uint32 tunerIndex, uint64 pageDescriptorAddress) {

  try {
    validate(isdb, tunerIndex, false);

    uint64 currentDesc = pageDescriptorAddress;
    uint8 *ptr;
    uint64 pageAddr;
    uint32 pageSize;
    uint32 totalSize = 0;
    BufferDesc* first = nullptr;
    BufferDesc* prev = nullptr;

    while (!(currentDesc & 0x01ULL)) {
      ptr = reinterpret_cast<uint8*>(currentDesc);
      pageAddr = (*reinterpret_cast<uint64*>(ptr + 0)) & ~0x07ULL;
      pageSize = (*reinterpret_cast<uint32*>(ptr + 8)) & ~0x07UL;
      currentDesc = (*reinterpret_cast<uint64*>(ptr + 12)) & ~0x02ULL;

      if (mBufferDescs.count(pageAddr) > 0) {
        std::unique_ptr<BufferDesc> &bufferDesc = mBufferDescs[pageAddr];
        bufferDesc->isdb = isdb;
        bufferDesc->tunerIndex = tunerIndex;

        // TS同期バイトのチェック対象の設定
        if (totalSize % TS_PKT_LEN == 0) {
          bufferDesc->sync = true;
          totalSize = 0;
        } else {
          totalSize += pageSize;
        }

        // 循環リストの次要素の設定
        if (!first) {
          first = bufferDesc.get();
        }
        if (prev) {
          prev->next = bufferDesc.get();
        }
        prev = bufferDesc.get();
      }

      if (currentDesc == pageDescriptorAddress) {
        break;
      }
    }

    if (prev) {
      prev->next = first;
    }

    mDescriptor[idx(isdb, tunerIndex)] = first;

  } catch (SingleDeviceException&) {

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BD_Device::SetTransferEnabled(ISDB isdb, uint32 tunerIndex, bool enabled) {

  try {
    validate(isdb, tunerIndex, false);

    if (!mTransferEnabled[idx(isdb, tunerIndex)] && enabled) {
      // TSデータ受信スレッドを開始
      TRACE_F(_T("*** Start TS data xfer thread. ***")
        << _T(" isdb=") << std::dec << isdb
        << _T(" tunerIndex=") << std::dec << tunerIndex
      );
      mUsbInstance[tunerIndex]->startXferThread(isdb, tunerIndex, this);

    } else if (mTransferEnabled[idx(isdb, tunerIndex)] && !enabled) {
      // TSデータ転送スレッドを停止
      TRACE_F(_T("*** Stop TS data xfer thread. ***")
        << _T(" isdb=") << std::dec << isdb
        << _T(" tunerIndex=") << std::dec << tunerIndex
      );
      mUsbInstance[tunerIndex]->stopXferThread(isdb);
    }
    mUsbInstance[tunerIndex]->setXferEnable(isdb, enabled);

  } catch (SingleDeviceException&) {

  } catch (InternalException& e) {
    return e.status();
  }

  mTransferEnabled[idx(isdb, tunerIndex)] = enabled;
  return STATUS_OK;
}


status BD_Device::GetTransferEnabled(ISDB isdb, uint32 tunerIndex, bool *enabled) const {

  try {
    validate(isdb, tunerIndex, false);

  } catch (SingleDeviceException&) {

  } catch (InternalException& e) {
    return e.status();
  }

  *enabled = mTransferEnabled[idx(isdb, tunerIndex)];
  return STATUS_OK;
}


status BD_Device::SetTransferTestMode(ISDB isdb, uint32 tunerIndex, bool testMode, uint16 initial, bool _not) {

  return STATUS_OK;
}


status BD_Device::GetTransferInfo(ISDB isdb, uint32 tunerIndex, TransferInfo *transferInfo) {

  try {
    validate(isdb, tunerIndex, false);

  } catch (SingleDeviceException&) {

  } catch (InternalException& e) {
    return e.status();
  }

  // ダミーの値を返す
  transferInfo->Busy = false;
  transferInfo->Status = 0;
  transferInfo->InternalFIFO_A_Overflow = false;
  transferInfo->InternalFIFO_A_Underflow = false;
  transferInfo->ExternalFIFO_Overflow = false;
  transferInfo->ExternalFIFO_MaxUsedBytes = 4096;
  transferInfo->InternalFIFO_B_Overflow = false;
  transferInfo->InternalFIFO_B_Underflow = false;

  return STATUS_OK;
}


bool BD_Device::receive(Device::ISDB isdb, uint32 tunerIndex, uint8 * data, uint32 size) {

  if (!mReady[idx(isdb, tunerIndex)]) {
    return true;
  }

  BufferDesc* currentDesc = mDescriptor[idx(isdb, tunerIndex)];
  uint32 currentOffset = mOffset[idx(isdb, tunerIndex)];
  uint64 pageAddr;
  uint32 pageSize, writeSize;
  uint8* newData;

  if (mReSync[idx(isdb, tunerIndex)] && currentDesc->sync && currentOffset > 0) {

    // TS同期バイトの位置を検出
    uint8* newData = findSyncByte(data, size);
    if (newData == nullptr) {
      return true;
    }

    // コピーの開始位置を調整
    uint32 gap = currentOffset % TS_PKT_LEN;
    uint32 correct = TS_PKT_LEN - gap;

    if (correct != newData - data) {
      size = size - static_cast<uint32>(newData - data);
      data = newData;
      if (currentOffset - gap >= 0) {
        currentOffset -= gap;
      } else {
        currentOffset += correct;
      }
    }

    mReSync[idx(isdb, tunerIndex)] = false;
  }

  // データのコピー
  while (true) {
    pageAddr = currentDesc->bufferInfo.Address;
    pageSize = currentDesc->bufferInfo.Size;
    writeSize = 0;

    if (currentOffset > 0) {
      pageAddr += currentOffset;
      pageSize -= currentOffset;

    } else if (currentDesc->sync) {
      newData = findSyncByte(data, size);
      if (newData != nullptr) {
        size = size - static_cast<uint32>(newData - data);
        data = newData;
      }
    }

    writeSize = (pageSize < size) ? pageSize : size;
    ::CopyMemory(reinterpret_cast<uint8*>(pageAddr), data, writeSize);

    if (writeSize == pageSize) {
      currentOffset = 0;
      currentDesc = currentDesc->next;
    } else {
      currentOffset += writeSize;
    }

    data += writeSize;
    size -= writeSize;
    if (size <= 0) {
      break;
    }
  }

  mDescriptor[idx(isdb, tunerIndex)] = currentDesc;
  mOffset[idx(isdb, tunerIndex)] = currentOffset;

  return true;
}


void BD_Device::validate(ISDB isdb, uint32 tunerIndex, bool checkSleep) const {

  // パラメータのチェック
  if ((DEVICE_COUNT <= tunerIndex) || (ISDB_COUNT <= isdb)) {
    TRACE_F(_T("Invalid parameter.")
      << _T(" tunerIndex=") << std::dec << tunerIndex
      << _T(" isdb=") << std::dec << isdb);
    throw InternalException(STATUS_INVALID_PARAM_ERROR, "Invalid parameter.");
  }

  // 2個目のチューナーが存在するかのチェック
  if (!mUsbAddress[tunerIndex]) {
    //TRACE_F(_T("No USB address.")
    //  << _T(" tunerIndex=") << std::dec << tunerIndex);
    throw SingleDeviceException("No USB address.");
  }

  // デバイスのオープン状態のチェック
  if (!mUsbInstance[tunerIndex]) {
    TRACE_F(_T("Device is not open.")
      << _T(" tunerIndex=") << std::dec << tunerIndex);
    throw InternalException(STATUS_DEVICE_IS_NOT_OPEN_ERROR, "Device is not open.");
  }

  // スリープ状態のチェック
  if (checkSleep) {
    bool sleep;
    mTunerInstance[idx(isdb, tunerIndex)]->getTunerSleep(&sleep);
    if (sleep) {
      TRACE_F(_T("Tuner is sleep.")
        << _T(" tunerIndex=") << std::dec << tunerIndex
        << _T(" isdb=") << std::dec << isdb);
      throw InternalException(STATUS_TUNER_IS_SLEEP_ERROR, "Tuner is sleep.");
    }
  }
}


uint8 * BD_Device::findSyncByte(uint8 * data, uint32 size) {

  bool find = false;
  uint8* ptr;
  uint32 count;

  for (ptr = data; ptr < data + size; ptr++) {

    // TS同期バイト以外はスキップ
    if (*ptr != TS_SYNC_BYTE) {
      continue;
    }

    // 残りサイズ中に指定数のTS同期バイトが入らない場合はNG
    if (ptr + TS_PKT_LEN * (TS_SYNC_COUNT - 1) >= data + size) {
      break;
    }

    // TS同期バイトが指定数連続して存在するかチェック
    count = 0;
    do {
      if (++count >= TS_SYNC_COUNT) {
        return ptr;
      }
    } while (*(ptr + TS_PKT_LEN * count) == TS_SYNC_BYTE);
  }

  return nullptr;
}


BD_Device::BD_Device(const Bus::DeviceInfo *deviceInfo) {

  // Bus:1個目のデバイスのUSBバスアドレス
  mUsbAddress[0] = static_cast<uint8>(deviceInfo->Bus);


  // Slot:2個目のデバイスのUSBバスアドレス
  mUsbAddress[1] = static_cast<uint8>(deviceInfo->Slot);

  // チューナー単位の設定値の初期化
  for (uint32 tunerIndex = 0; tunerIndex < DEVICE_COUNT; tunerIndex++) {
    for (uint32 i = 0; i < ISDB_COUNT; i++) {
      ISDB isdb = static_cast<ISDB>(i);

      mDescriptor[idx(isdb, tunerIndex)] = 0;
      mOffset[idx(isdb, tunerIndex)] = 0;
      mTransferEnabled[idx(isdb, tunerIndex)] = false;
      mReady[idx(isdb, tunerIndex)] = false;
      mReSync[idx(isdb, tunerIndex)] = false;
    }
  }

  // LNB電源制御設定（ダミー）の初期化
  mLnbPower = LNB_POWER_OFF;
  mLnbPowerWhenClose = LNB_POWER_OFF;
}


}
}