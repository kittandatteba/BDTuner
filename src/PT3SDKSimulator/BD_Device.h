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
* EARTH_PT3.hのDeviceクラス（インターフェース）の実装クラス。
*/
class BD_Device : public Device, public IReceiver {

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

  // TS ピン
  virtual status SetTsPinsMode(ISDB isdb, uint32 index, const TsPinsMode *mode) override;

  virtual status GetTsPinsLevel(ISDB isdb, uint32 index, TsPinsLevel *level) override;

  virtual status GetTsSyncByte(ISDB isdb, uint32 index, uint8 *syncByte) override;

  virtual status SetRamPinsMode(RamPinsMode mode) override;

  // DMA バッファ
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
    TransferInfo *transferInfo);

  virtual bool receive(Device::ISDB isdb, uint32 tunerIndex, uint8* data, uint32 size) override;

protected:

  /**
  * コンストラクタ（非公開）
  */
  BD_Device(const Bus::DeviceInfo *deviceInfo);

  /**
  * デストラクタ（非公開）
  */
  virtual ~BD_Device() override = default;

private:

  /**
  * 転送用バッファ情報
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
  * チューナー数
  */
  static constexpr sint32 DEVICE_COUNT = 2;

  /**
  * GetBufferInfo()で返却するBufferInfo1個あたりのブロックサイズ（4096の倍数）
  */
  static constexpr uint32 BLOCK_SIZE = 4096 * 47;

  /**
  * TS同期バイト
  */
  static constexpr uint8 TS_SYNC_BYTE = 0x47U;

  /**
  * TSパケット長
  */
  static constexpr uint32 TS_PKT_LEN = 188;

  /**
  * TS同期バイトの出現数（1以上）
  */
  static constexpr uint32 TS_SYNC_COUNT = 8;

  /**
  * チャネル切り替え時からTSデータ転送開始までの待ち時間（msec）
  */
  static constexpr uint32 FREQUENCY_SET_INTERVAL = 300;


  /**
  * USBバスアドレス（BD_Busから連携）
  */
  uint8	mUsbAddress[DEVICE_COUNT];

  /**
  * EzUsbFx2Lpインスタンス
  */
  std::unique_ptr<EzUsbFx2Lp> mUsbInstance[DEVICE_COUNT];

  /**
  * Jdts290532インスタンス
  */
  std::unique_ptr<IJdts290532> mTunerInstance[ISDB_COUNT * DEVICE_COUNT];

  /**
  * 転送用バッファ情報
  */
  std::map<uint64, std::unique_ptr<BufferDesc>> mBufferDescs;

  BufferDesc* mDescriptor[ISDB_COUNT * DEVICE_COUNT];
  uint32 mOffset[ISDB_COUNT * DEVICE_COUNT];
  bool mTransferEnabled[ISDB_COUNT * DEVICE_COUNT];
  bool mReady[ISDB_COUNT * DEVICE_COUNT];
  bool mReSync[ISDB_COUNT * DEVICE_COUNT];

  /**
  * LNB電源制御設定（ダミー）
  */
  LnbPower mLnbPower;

  /**
  * LNB電源制御設定（ダミー）
  */
  LnbPower mLnbPowerWhenClose;

  /**
  * チャンネル番号
  */
  uint32 mChannel[ISDB_COUNT * DEVICE_COUNT];

  /**
  * チャンネルオフセット値
  */
  sint32 mChannelOffset[ISDB_COUNT * DEVICE_COUNT];

  /**
  * チューナーのスリープ状態
  */
  bool mSleep[ISDB_COUNT * DEVICE_COUNT];

  /**
  * チューナーのTSID
  */
  uint32 mTsId[ISDB_COUNT * DEVICE_COUNT];

  /**
  * TS同期バイトの位置を取得する。
  */
  static uint8* findSyncByte(uint8* data, uint32 size);

  /**
  * ステータス判定用共通処理。
  */
  void validate(ISDB isdb, uint32 tunerIndex, bool checkSleep = true) const;

  /**
  * ISDBとチューナー番号から配列中の位置を取得する。
  */
  inline static uint32 idx(ISDB isdb, uint32 tunerIndex) {
    return isdb * 2 + tunerIndex;
  }

  BD_Device(BD_Device const&) = delete;
  BD_Device(BD_Device&&) = delete;
  BD_Device& operator =(BD_Device const&) = delete;
  BD_Device& operator =(BD_Device&&) = delete;
};

}
}