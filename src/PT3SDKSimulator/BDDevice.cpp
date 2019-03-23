#include "stdafx.h"
#include "InternalException.h"
#include "EzUsbFx2Lp.h"
#include "Jdts290532S.h"
#include "Jdts290532T.h"
#include "BDUtil.h"
#include "BDDevice.h"

namespace EARTH {
namespace PT {


status BDDevice::GetInstance(const Bus::DeviceInfo *deviceInfo, Device **device) {

  if ((deviceInfo == NULL) || (device == NULL)) {
    TRACE_F(_T("Invalid param error.")
      << _T(" deviceInfo=") << std::hex << deviceInfo
      << _T(" device=") << std::hex << device);
    return STATUS_INVALID_PARAM_ERROR;
  }

  BDDevice *bdDevice;

  // BD_Device�N���X�̃C���X�^���X����
  try {
    bdDevice = new BDDevice(deviceInfo);
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


status BDDevice::Delete() {

  delete this;
  return STATUS_OK;
}


status BDDevice::Open() {

  if (!mDevNum[0]) {
    TRACE_F(_T("General error.")
      << _T(" mUsbAddress[0]=") << std::hex << mDevNum[0]);
    return STATUS_GENERAL_ERROR;
  }

  status status = STATUS_OK;

  for (uint32 tunerIndex = 0; tunerIndex < DEVICE_COUNT; tunerIndex++) {
    if (!mDevNum[tunerIndex])
      break;

    // ���ɃC���X�^���X�����ς݂̏ꍇ�̓G���[
    if (mUsbInstance[tunerIndex]) {
      TRACE_F(_T("Device is already open error.")
        << _T(" tunerIndex=") << std::dec << tunerIndex);
      status = STATUS_DEVICE_IS_ALREADY_OPEN_ERROR;
      break;
    }

    try {
      // EzUsbFx2Lp�N���X�̃C���X�^���X����
      mUsbInstance[tunerIndex].reset(new EzUsbFx2Lp());


      // EzUsbFx2Lp�̏�����
      mUsbInstance[tunerIndex]->init(mDevNum[tunerIndex]);


      // Jdts290532�N���X�̃C���X�^���X����
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


status BDDevice::Close() {

  for (uint32 tunerIndex = 0; tunerIndex < DEVICE_COUNT; tunerIndex++) {
    for (uint32 i = 0; i < ISDB_COUNT; i++) {
      ISDB isdb = static_cast<ISDB>(i);

      // Jdts290532�N���X�̃C���X�^���X�J��
      mTunerInstance[idx(isdb, tunerIndex)].reset();
    }

    // EzUsbFx2Lp�N���X�̃C���X�^���X�J��
    mUsbInstance[tunerIndex].reset();
  }
  return STATUS_OK;
}


status BDDevice::GetConstantInfo(ConstantInfo *info) const {

  if (info == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" info=") << std::hex << info);
    return STATUS_INVALID_PARAM_ERROR;
  }

  // �_�~�[�̌Œ����ݒ肷��
  info->PTn = 0x03U;
  info->Version_RegisterMap = 0x01U;
  info->Version_FPGA = 0x04U;
  info->CanTransportTS = true;
  info->BitLength_PageDescriptorSize = 0x14U;

  return STATUS_OK;
}


status BDDevice::SetLnbPower(LnbPower lnbPower) {

  // LNB�d������̓n�[�h�E�F�A�ɖ������̂���
  // �ݒ�l�̕ێ��݂̂ŉ��̐�������Ȃ�

  mLnbPower = lnbPower;

  return STATUS_OK;
}


status BDDevice::GetLnbPower(LnbPower *lnbPower) const {

  // LNB�d������̓n�[�h�E�F�A�ɖ������̂���
  // �ݒ�l�̕ێ��݂̂ŉ��̐�������Ȃ�

  if (lnbPower == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" lnbPower=") << std::hex << lnbPower);
    return STATUS_INVALID_PARAM_ERROR;
  }

  *lnbPower = mLnbPower;

  return STATUS_OK;
}


status BDDevice::SetLnbPowerWhenClose(LnbPower lnbPower) {

  // LNB�d������̓n�[�h�E�F�A�ɖ������̂���
  // �ݒ�l�̕ێ��݂̂ŉ��̐�������Ȃ�

  mLnbPowerWhenClose = lnbPower;

  return STATUS_OK;
}


status BDDevice::GetLnbPowerWhenClose(LnbPower *lnbPower) const {

  // LNB�d������̓n�[�h�E�F�A�ɖ������̂���
  // �ݒ�l�̕ێ��݂̂ŉ��̐�������Ȃ�

  if (lnbPower == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" lnbPower=") << std::hex << lnbPower);
    return STATUS_INVALID_PARAM_ERROR;
  }

  *lnbPower = mLnbPowerWhenClose;

  return STATUS_OK;
}


status BDDevice::InitTuner() {

  try {
    for (sint32 tunerIndex = 0; tunerIndex < DEVICE_COUNT; tunerIndex++) {

      for (uint32 i = 0; i < ISDB_COUNT; i++) {
        ISDB isdb = static_cast<ISDB>(i);

        mChannel[idx(isdb, tunerIndex)] = (isdb == ISDB_S) ? 0 : ((tunerIndex == 0) ? 70 : 71);
        mChannelOffset[idx(isdb, tunerIndex)] = 0;
        mSleep[idx(isdb, tunerIndex)] = true;

        if (mDevNum[tunerIndex]) {

          // �`���[�i�̏�����
          mTunerInstance[idx(isdb, tunerIndex)]->initTuner();

          // ����IC�̏�����
          mTunerInstance[idx(isdb, tunerIndex)]->initDemod();

          // �f�t�H���g�̃`�����l���Ń`���[�j���O
          mTunerInstance[idx(isdb, tunerIndex)]->setFrequency(mChannel[idx(isdb, tunerIndex)], mChannelOffset[idx(isdb, tunerIndex)]);

          // �X���[�v���[�h�Ɉڍs
          mTunerInstance[idx(isdb, tunerIndex)]->setTunerSleep(mSleep[idx(isdb, tunerIndex)]);
        }
      }
    }

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BDDevice::SetAmpPowerT(bool b) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::SetTunerSleep(ISDB isdb, uint32 tunerIndex, bool sleep) {

  try {
    validate(isdb, tunerIndex, false);

    // �`�����l���l�A�`�����l���I�t�Z�b�g�l���擾
    //uint32 channel;
    //sint32 offset;
    //mTunerInstance[idx(isdb, tunerIndex)]->getFrequency(&channel, &offset);

    // �X���[�v��Ԃ̐ݒ�
    mTunerInstance[idx(isdb, tunerIndex)]->setTunerSleep(sleep);

    // �`���[�j���O
    //mTunerInstance[idx(isdb, tunerIndex)]->setFrequency(channel, offset);

  } catch (SingleDeviceException&) {
    mSleep[idx(isdb, tunerIndex)] = sleep;

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BDDevice::GetTunerSleep(ISDB isdb, uint32 tunerIndex, bool *sleep) const {

  if (sleep == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" sleep=") << std::hex << sleep);
    return STATUS_INVALID_PARAM_ERROR;
  }

  try {
    validate(isdb, tunerIndex, false);

    // �X���[�v��Ԃ��擾
    mTunerInstance[idx(isdb, tunerIndex)]->getTunerSleep(sleep);

  } catch (SingleDeviceException&) {
    *sleep = mSleep[idx(isdb, tunerIndex)];

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BDDevice::SetFrequency(ISDB isdb, uint32 tunerIndex, uint32 channel, sint32 offset) {

  status status = STATUS_OK;

  try {
    validate(isdb, tunerIndex);

    mReady[idx(isdb, tunerIndex)] = false;

    mTunerInstance[idx(isdb, tunerIndex)]->setFrequency(channel, offset);

    // �`���l���؂�ւ���������TS�f�[�^�]���J�n�܂Ŏw�莞�ԃX���[�v
    Sleep(BDUtil::mSetFrequencyDelay[isdb]);

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


status BDDevice::GetFrequency(ISDB isdb, uint32 tunerIndex, uint32 *channel, sint32 *offset) const {

  if ((channel == nullptr) || (offset == nullptr)) {
    TRACE_F(_T("Invalid param error.")
      << _T(" channel=") << std::hex << channel
      << _T(" offset=") << std::hex << offset);
    return STATUS_INVALID_PARAM_ERROR;
  }

  try {
    validate(isdb, tunerIndex);

    // �`�����l���l�A�`�����l���I�t�Z�b�g�l���擾
    mTunerInstance[idx(isdb, tunerIndex)]->getFrequency(channel, offset);

  } catch (SingleDeviceException&) {
    *channel = mChannel[idx(isdb, tunerIndex)];
    *offset = mChannelOffset[idx(isdb, tunerIndex)];

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BDDevice::GetFrequencyOffset(ISDB isdb, uint32 tunerIndex, sint32 *clock, sint32 *carrier) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::GetCnAgc(ISDB isdb, uint32 tunerIndex, uint32 *cn100, uint32 *currentAgc, uint32 *maxAgc) {

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


status BDDevice::GetRFLevel(uint32 tunerIndex, float *level) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::SetIdS(uint32 tunerIndex, uint32 id) {

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


status BDDevice::GetIdS(uint32 tunerIndex, uint32 *id) {

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


status BDDevice::SetInnerErrorRateLayer(ISDB isdb, uint32 tunerIndex, LayerIndex layerIndex) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::GetInnerErrorRate(ISDB isdb, uint32 tunerIndex, ErrorRate *errorRate) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::GetCorrectedErrorRate(ISDB isdb, uint32 tunerIndex, LayerIndex layerIndex, ErrorRate *errorRate) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::ResetCorrectedErrorCount(ISDB isdb, uint32 tunerIndex) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::GetErrorCount(ISDB isdb, uint32 tunerIndex, uint32 *count) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::GetTmccS(uint32 tunerIndex, TmccS *tmcc) {

  if (tmcc == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" tmcc=") << std::hex << tmcc);
    return STATUS_INVALID_PARAM_ERROR;
  }

  try {
    validate(ISDB_S, tunerIndex);

    mTunerInstance[idx(ISDB_S, tunerIndex)]->getTmcc(tmcc);

  } catch (SingleDeviceException&) {
    // ������2�ڂ̃`���[�i�[�����݂��Ȃ��ꍇ�̓G���[�ɂ���
    return STATUS_GENERAL_ERROR;

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BDDevice::GetLayerS(uint32 tunerIndex, LayerS *layer) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::GetTmccT(uint32 tunerIndex, TmccT *tmcc) {

  if (tmcc == nullptr) {
    TRACE_F(_T("Invalid param error.")
      << _T(" tmcc=") << std::hex << tmcc);
    return STATUS_INVALID_PARAM_ERROR;
  }

  try {
    validate(ISDB_T, tunerIndex);

    mTunerInstance[idx(ISDB_T, tunerIndex)]->getTmcc(tmcc);

  } catch (SingleDeviceException&) {
    // ������2�ڂ̃`���[�i�[�����݂��Ȃ��ꍇ�̓G���[�ɂ���
    return STATUS_GENERAL_ERROR;

  } catch (InternalException& e) {
    return e.status();
  }

  return STATUS_OK;
}


status BDDevice::SetLayerEnable(ISDB isdb, uint32 tunerIndex, LayerMask layerMask) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::GetLayerEnable(ISDB isdb, uint32 tunerIndex, LayerMask *layerMask) const {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::SetTsPinsMode(ISDB isdb, uint32 index, const TsPinsMode *mode) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::GetTsPinsLevel(ISDB isdb, uint32 index, TsPinsLevel *level) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::GetTsSyncByte(ISDB isdb, uint32 index, uint8 *syncByte) {

  *syncByte = TS_SYNC_BYTE;
  return STATUS_OK;
}


status BDDevice::SetRamPinsMode(RamPinsMode mode) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::LockBuffer__Obsolated__(void *ptr, uint32 size, void **handle) {

  // �������G���[��ԋp
  TRACE_F(_T("Not implimented."));

  return STATUS_NOT_IMPLIMENTED;
}


status BDDevice::LockBuffer(void *ptr, uint32 size, TransferDirection direction, void **handle) {

  uint64 addess = reinterpret_cast<uint64>(ptr);
  if ((addess & 0x07) != 0) {
    TRACE_F(_T("Invalid param error(lower 3bit not 0).")
      << _T(" ptr=") << std::hex << ptr);
    return STATUS_INVALID_PARAM_ERROR;
  }

  try {
    // �]���p�o�b�t�@���𐶐�
    mBufferDescs[addess] = std::unique_ptr<BufferDesc>(new BufferDesc());
    mBufferDescs[addess]->infoCount = 1;
    mBufferDescs[addess]->bufferInfo.Address = addess;
    mBufferDescs[addess]->bufferInfo.Size = size;
    mBufferDescs[addess]->isdb = ISDB_COUNT;
    mBufferDescs[addess]->tunerIndex = DEVICE_COUNT;
    mBufferDescs[addess]->sync = false;
    mBufferDescs[addess]->next = nullptr;

    *handle = mBufferDescs[addess].get();

  } catch (std::bad_alloc&) {
    return STATUS_OUT_OF_MEMORY_ERROR;
  }

  return STATUS_OK;
}


status BDDevice::SyncBufferCpu(void *handle) {

  return STATUS_OK;
}


status BDDevice::SyncBufferIo(void *handle) {

  return STATUS_OK;
}


status BDDevice::UnlockBuffer(void *handle) {

  return STATUS_OK;
}


status BDDevice::GetBufferInfo(void *handle, const BufferInfo **infoTable, uint32 *infoCount) {

  BufferDesc* bufferDesc = static_cast<BufferDesc*>(handle);
  *infoTable = &(bufferDesc->bufferInfo);
  *infoCount = bufferDesc->infoCount;

  return STATUS_OK;
}


status BDDevice::SetTransferPageDescriptorAddress(ISDB isdb, uint32 tunerIndex, uint64 pageDescriptorAddress) {

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

        // TS�����o�C�g�̃`�F�b�N�Ώۂ̐ݒ�
        if (totalSize % TS_PKT_LEN == 0) {
          bufferDesc->sync = true;
          totalSize = 0;
        } else {
          totalSize += pageSize;
        }

        // �z���X�g�̎��v�f�̐ݒ�
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


status BDDevice::SetTransferEnabled(ISDB isdb, uint32 tunerIndex, bool enabled) {

  try {
    validate(isdb, tunerIndex, false);

    if (!mTransferEnabled[idx(isdb, tunerIndex)] && enabled) {
      // TS�f�[�^��M�X���b�h���J�n
      TRACE_F(_T("*** Start TS data xfer thread. ***")
        << _T(" isdb=") << std::dec << isdb
        << _T(" tunerIndex=") << std::dec << tunerIndex
      );
      mUsbInstance[tunerIndex]->startXferThread(isdb, tunerIndex, this);

    } else if (mTransferEnabled[idx(isdb, tunerIndex)] && !enabled) {
      // TS�f�[�^�]���X���b�h���~
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


status BDDevice::GetTransferEnabled(ISDB isdb, uint32 tunerIndex, bool *enabled) const {

  try {
    validate(isdb, tunerIndex, false);

  } catch (SingleDeviceException&) {

  } catch (InternalException& e) {
    return e.status();
  }

  *enabled = mTransferEnabled[idx(isdb, tunerIndex)];
  return STATUS_OK;
}


status BDDevice::SetTransferTestMode(ISDB isdb, uint32 tunerIndex, bool testMode, uint16 initial, bool _not) {

  return STATUS_OK;
}


status BDDevice::GetTransferInfo(ISDB isdb, uint32 tunerIndex, TransferInfo *transferInfo) {

  try {
    validate(isdb, tunerIndex, false);

  } catch (SingleDeviceException&) {

  } catch (InternalException& e) {
    return e.status();
  }

  // �_�~�[�̒l��Ԃ�
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


bool BDDevice::receive(Device::ISDB isdb, uint32 tunerIndex, uint8 * data, uint32 size) {

  if (!mReady[idx(isdb, tunerIndex)]) {
    return true;
  }

  BufferDesc* currentDesc = mDescriptor[idx(isdb, tunerIndex)];
  uint32 currentOffset = mOffset[idx(isdb, tunerIndex)];
  uint64 pageAddr;
  uint32 pageSize, writeSize;
  uint8* newData;

  if (mReSync[idx(isdb, tunerIndex)] && currentDesc->sync && currentOffset > 0) {

    // TS�����o�C�g�̈ʒu�����o
    uint8* newData = findSyncByte(data, size);
    if (newData == nullptr) {
      return true;
    }

    // �R�s�[�̊J�n�ʒu�𒲐�
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

  // �f�[�^�̃R�s�[
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


void BDDevice::validate(ISDB isdb, uint32 tunerIndex, bool checkSleep) const {

  // �p�����[�^�̃`�F�b�N
  if ((DEVICE_COUNT <= tunerIndex) || (ISDB_COUNT <= isdb)) {
    TRACE_F(_T("Invalid parameter.")
      << _T(" tunerIndex=") << std::dec << tunerIndex
      << _T(" isdb=") << std::dec << isdb);
    throw InternalException(STATUS_INVALID_PARAM_ERROR, "Invalid parameter.");
  }

  // 2�ڂ̃`���[�i�[�����݂��邩�̃`�F�b�N
  if (!mDevNum[tunerIndex]) {
    //TRACE_F(_T("No USB address.")
    //  << _T(" tunerIndex=") << std::dec << tunerIndex);
    throw SingleDeviceException("No USB address.");
  }

  // �f�o�C�X�̃I�[�v����Ԃ̃`�F�b�N
  if (!mUsbInstance[tunerIndex]) {
    TRACE_F(_T("Device is not open.")
      << _T(" tunerIndex=") << std::dec << tunerIndex);
    throw InternalException(STATUS_DEVICE_IS_NOT_OPEN_ERROR, "Device is not open.");
  }

  // �X���[�v��Ԃ̃`�F�b�N
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


uint8 * BDDevice::findSyncByte(uint8 * data, uint32 size) {

  bool find = false;
  uint8* ptr;
  uint32 count;

  for (ptr = data; ptr < data + size; ptr++) {

    // TS�����o�C�g�ȊO�̓X�L�b�v
    if (*ptr != TS_SYNC_BYTE) {
      continue;
    }

    // �c��T�C�Y���Ɏw�萔��TS�����o�C�g������Ȃ��ꍇ��NG
    if (ptr + TS_PKT_LEN * (TS_SYNC_COUNT - 1) >= data + size) {
      break;
    }

    // TS�����o�C�g���w�萔�A�����đ��݂��邩�`�F�b�N
    count = 0;
    do {
      if (++count >= TS_SYNC_COUNT) {
        return ptr;
      }
    } while (*(ptr + TS_PKT_LEN * count) == TS_SYNC_BYTE);
  }

  return nullptr;
}


BDDevice::BDDevice(const Bus::DeviceInfo *deviceInfo) {

  // Bus:1�ڂ̃f�o�C�X�ԍ�
  mDevNum[0] = static_cast<uint8>(deviceInfo->Bus);


  // Slot:2�ڂ̃f�o�C�X�̔ԍ�
  mDevNum[1] = static_cast<uint8>(deviceInfo->Slot);

  // �`���[�i�[�P�ʂ̐ݒ�l�̏�����
  for (uint32 tunerIndex = 0; tunerIndex < DEVICE_COUNT; tunerIndex++) {
    for (uint32 i = 0; i < ISDB_COUNT; i++) {
      ISDB isdb = static_cast<ISDB>(i);

      mDescriptor[idx(isdb, tunerIndex)] = nullptr;
      mOffset[idx(isdb, tunerIndex)] = 0;
      mTransferEnabled[idx(isdb, tunerIndex)] = false;
      mReady[idx(isdb, tunerIndex)] = false;
      mReSync[idx(isdb, tunerIndex)] = false;
    }
  }

  // LNB�d������ݒ�i�_�~�[�j�̏�����
  mLnbPower = LNB_POWER_OFF;
  mLnbPowerWhenClose = LNB_POWER_OFF;
}


}
}
