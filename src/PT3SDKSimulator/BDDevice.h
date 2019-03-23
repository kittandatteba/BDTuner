#pragma once

#include <memory>
#include <map>
#include <EARTH_PT3.h>
#include "IReceiver.h"

namespace EARTH {
namespace PT {

class EzUsbFx2Lp;
class IJdts290532;

/**
* EARTH_PT3.h��Device�N���X�i�C���^�[�t�F�[�X�j�̎����N���X�B
*/
class BDDevice : public Device, public IReceiver {

public:

  static status GetInstance(const Bus::DeviceInfo *deviceInfo, Device **device);

  virtual status Delete() override;

  virtual status Open() override;

  virtual status Close() override;

  virtual status GetConstantInfo(ConstantInfo *info) const override;

  virtual status SetLnbPower(LnbPower lnbPower) override;

  virtual status GetLnbPower(LnbPower *lnbPower) const override;

  virtual status SetLnbPowerWhenClose(LnbPower lnbPower) override;

  virtual status GetLnbPowerWhenClose(LnbPower *lnbPower) const override;

  virtual status InitTuner() override;

  virtual status SetAmpPowerT(bool b) override;

  virtual status SetTunerSleep(ISDB isdb, uint32 tunerIndex, bool sleep) override;

  virtual status GetTunerSleep(ISDB isdb, uint32 tunerIndex, bool *sleep) const override;

  virtual status SetFrequency(ISDB isdb, uint32 tunerIndex, uint32 channel,
    sint32 offset = 0) override;

  virtual status GetFrequency(ISDB isdb, uint32 tunerIndex, uint32 *channel,
    sint32 *offset) const override;

  virtual status GetFrequencyOffset(ISDB isdb, uint32 tunerIndex,
    sint32 *clock, sint32 *carrier) override;

  virtual status GetCnAgc(ISDB isdb, uint32 tunerIndex, uint32 *cn100,
    uint32 *currentAgc, uint32 *maxAgc) override;

  virtual status GetRFLevel(uint32 tunerIndex, float *level) override;

  virtual status SetIdS(uint32 tunerIndex, uint32 id) override;

  virtual status GetIdS(uint32 tunerIndex, uint32 *id) override;

  virtual status SetInnerErrorRateLayer(ISDB isdb, uint32 tunerIndex,
    LayerIndex layerIndex) override;

  virtual status GetInnerErrorRate(ISDB isdb, uint32 tunerIndex, ErrorRate *errorRate) override;

  virtual status GetCorrectedErrorRate(ISDB isdb, uint32 tunerIndex,
    LayerIndex layerIndex, ErrorRate *errorRate) override;

  virtual status ResetCorrectedErrorCount(ISDB isdb, uint32 tunerIndex) override;

  virtual status GetErrorCount(ISDB isdb, uint32 tunerIndex, uint32 *count) override;

  virtual status GetTmccS(uint32 tunerIndex, TmccS *tmcc) override;

  virtual status GetLayerS(uint32 tunerIndex, LayerS *layer) override;

  virtual status GetTmccT(uint32 tunerIndex, TmccT *tmcc) override;

  virtual status SetLayerEnable(ISDB isdb, uint32 tunerIndex, LayerMask layerMask) override;

  virtual status GetLayerEnable(ISDB isdb, uint32 tunerIndex, LayerMask *layerMask) const override;

  // TS �s��
  virtual status SetTsPinsMode(ISDB isdb, uint32 index, const TsPinsMode *mode) override;

  virtual status GetTsPinsLevel(ISDB isdb, uint32 index, TsPinsLevel *level) override;

  virtual status GetTsSyncByte(ISDB isdb, uint32 index, uint8 *syncByte) override;

  virtual status SetRamPinsMode(RamPinsMode mode) override;

  // DMA �o�b�t�@
  virtual status LockBuffer__Obsolated__(void *ptr, uint32 size, void **handle) override;

  virtual status LockBuffer(void *ptr, uint32 size, TransferDirection direction,
    void **handle) override;

  virtual status SyncBufferCpu(void *handle) override;

  virtual status SyncBufferIo(void *handle) override;

  virtual status UnlockBuffer(void *handle) override;

  virtual status GetBufferInfo(void *handle, const BufferInfo **infoTable,
    uint32 *infoCount) override;

  // DMA
  virtual status SetTransferPageDescriptorAddress(ISDB isdb, uint32 tunerIndex,
    uint64 pageDescriptorAddress) override;

  virtual status SetTransferEnabled(ISDB isdb, uint32 tunerIndex, bool  enabled) override;

  virtual status GetTransferEnabled(ISDB isdb, uint32 tunerIndex, bool *enabled) const override;

  virtual status SetTransferTestMode(ISDB isdb, uint32 tunerIndex,
    bool testMode = false, uint16 initial = 0, bool _not = false) override;

  virtual status GetTransferInfo(ISDB isdb, uint32 tunerIndex,
    TransferInfo *transferInfo) override;

  virtual bool receive(Device::ISDB isdb, uint32 tunerIndex, uint8* data, uint32 size) override;

protected:

  /**
  * �R���X�g���N�^�i����J�j
  */
  BDDevice(const Bus::DeviceInfo *deviceInfo);

  /**
  * �f�X�g���N�^�i����J�j
  */
  virtual ~BDDevice() override = default;

private:

  /**
  * �]���p�o�b�t�@���
  */
  struct BufferDesc {
    ISDB        isdb;
    uint32      tunerIndex;
    uint32      infoCount;
    BufferInfo  bufferInfo;
    bool        sync;
    BufferDesc* next;
  };

  /**
  * �`���[�i�[��
  */
  static constexpr sint32 DEVICE_COUNT = 2;

  /**
  * GetBufferInfo()�ŕԋp����BufferInfo1������̃u���b�N�T�C�Y�i4096�̔{���j
  */
  static constexpr uint32 BLOCK_SIZE = 4096 * 47;

  /**
  * TS�����o�C�g
  */
  static constexpr uint8 TS_SYNC_BYTE = 0x47U;

  /**
  * TS�p�P�b�g��
  */
  static constexpr uint32 TS_PKT_LEN = 188;

  /**
  * TS�����o�C�g�̏o�����i1�ȏ�j
  */
  static constexpr uint32 TS_SYNC_COUNT = 8;

  /**
  * �f�o�C�X�ԍ��iBD_Bus����A�g�j
  */
  uint8	mDevNum[DEVICE_COUNT];

  /**
  * EzUsbFx2Lp�C���X�^���X
  */
  std::unique_ptr<EzUsbFx2Lp> mUsbInstance[DEVICE_COUNT];

  /**
  * Jdts290532�C���X�^���X
  */
  std::unique_ptr<IJdts290532> mTunerInstance[ISDB_COUNT * DEVICE_COUNT];

  /**
  * �]���p�o�b�t�@���
  */
  std::map<uint64, std::unique_ptr<BufferDesc>> mBufferDescs;

  BufferDesc* mDescriptor[ISDB_COUNT * DEVICE_COUNT];
  uint32 mOffset[ISDB_COUNT * DEVICE_COUNT];
  bool mTransferEnabled[ISDB_COUNT * DEVICE_COUNT];
  bool mReady[ISDB_COUNT * DEVICE_COUNT];
  bool mReSync[ISDB_COUNT * DEVICE_COUNT];

  /**
  * LNB�d������ݒ�i�_�~�[�j
  */
  LnbPower mLnbPower;

  /**
  * LNB�d������ݒ�i�_�~�[�j
  */
  LnbPower mLnbPowerWhenClose;

  /**
  * �`�����l���ԍ�
  */
  uint32 mChannel[ISDB_COUNT * DEVICE_COUNT];

  /**
  * �`�����l���I�t�Z�b�g�l
  */
  sint32 mChannelOffset[ISDB_COUNT * DEVICE_COUNT];

  /**
  * �`���[�i�[�̃X���[�v���
  */
  bool mSleep[ISDB_COUNT * DEVICE_COUNT];

  /**
  * �`���[�i�[��TSID
  */
  uint32 mTsId[ISDB_COUNT * DEVICE_COUNT];

  /**
  * TS�����o�C�g�̈ʒu���擾����B
  */
  static uint8* findSyncByte(uint8* data, uint32 size);

  /**
  * �X�e�[�^�X����p���ʏ����B
  */
  void validate(ISDB isdb, uint32 tunerIndex, bool checkSleep = true) const;

  /**
  * ISDB�ƃ`���[�i�[�ԍ�����z�񒆂̈ʒu���擾����B
  */
  inline static uint32 idx(ISDB isdb, uint32 tunerIndex) {
    return isdb * 2 + tunerIndex;
  }

  BDDevice(BDDevice const&) = delete;
  BDDevice(BDDevice&&) = delete;
  BDDevice& operator =(BDDevice const&) = delete;
  BDDevice& operator =(BDDevice&&) = delete;
};

}
}
