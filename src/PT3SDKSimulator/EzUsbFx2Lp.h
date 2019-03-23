#pragma once

#include <memory>
#include <EARTH_PT3.h>
#include <CyAPI.h>
#include "II2C.h"

namespace EARTH {
namespace PT {

class IReceiver;

/**
* EzUsbFx2Lpの制御を行うためのクラス。
*/
class EzUsbFx2Lp : public II2C {

public:

  /**
  * コンストラクタ。
  */
  EzUsbFx2Lp();

  /**
  * デストラクタ。
  */
  virtual ~EzUsbFx2Lp() override;

  /**
  * I2Cメッセージの送受信を行う。
  */
  virtual uint32 xfer(Message* msg, uint32 len) override;

  /**
  * ファームウェアをロードし、各種初期化を行う。
  */
  void init(uint8 devNum);

  /**
  * TSデータ転送スレッドを開始する。
  */
  void startXferThread(Device::ISDB isdb, uint32 tunerIndex, IReceiver* receiver);


  /**
  * TSデータ転送スレッドを終了する。
  */
  void stopXferThread(Device::ISDB isdb);


  /**
  * TSデータ転送の有効／無効を切り替える。
  */
  void setXferEnable(Device::ISDB isdb, bool enabled);

private:

  /**
  * TSデータ転送バッファのサイズ
  */
  static constexpr uint32 TS_QUEUE_SIZE = 65536UL;

  /**
  * TSデータ転送バッファの本数
  */
  //static constexpr uint32 TS_QUEUE_NUM = 16UL;
  static constexpr uint32 TS_QUEUE_NUM = 8UL;

  /**
  * ReNumerationの最大リトライ回数
  */
  static constexpr sint32 RENUM_RETRY_COUNT = 40;

  /**
  * ReNumeration のリトライ間隔
  */
  static constexpr sint32 RENUM_RETRY_INTRVAL = 100;

  /**
  * エンドポイントアドレス
  */
  enum {
    EPA_CTL_IN = 0x81U,		// 制御データ受信用（EP1 IN）
    EPA_CTL_OUT = 0x01U,	// 制御データ送信用（EP1 OUT）
    EPA_DATA_S = 0x82U,		// ISDB_SのTSデータ受信用（EP2 IN）
    EPA_DATA_T = 0x86U		// ISDB_TのTSデータ受信用（EP6 IN）
  };

  /**
  * コマンド。
  */
  enum {
    CMD_EP6IN_START = 0x50U,
    CMD_EP6IN_STOP = 0x51U,
    CMD_EP2IN_START = 0x52U,
    CMD_EP2IN_STOP = 0x53U,
    CMD_PORT_CFG = 0x54U,		//addr_mask, out_pins
    CMD_PORT_WRITE = 0x55U,	//value
    CMD_IFCONFIG = 0x56U,		//value
    CMD_MODE_IDLE = 0x57U,
    CMD_I2C_READ = 0x58U,		//adrs,len (return length bytes)(max 64bytes)
    CMD_I2C_WRITE = 0x59U		//adrs,len,data... (max 64bytes)
  };

  /**
  * データ受信スレッド構造体
  */
  struct ThreadInfo {
    Device::ISDB      isdb = Device::ISDB::ISDB_S;
    uint32            tunerIndex = 0;
    HANDLE            threadHandle = nullptr;
    CCyBulkEndPoint*	endPoint = nullptr;
    bool              running = false;
    IReceiver*         receiver = nullptr;
  };

  /**
  * データ受信スレッド情報
  */
  ThreadInfo mThreadInfo[Device::ISDB::ISDB_COUNT];

  /**
  * クリティカルセクション
  */
  CRITICAL_SECTION mCriticalSection;

  /**
  * CyUSBDeviceインスタンス
  */
  std::unique_ptr<CCyUSBDevice> mUsbDevice;

  /**
  * 制御データ受信用エンドポイント
  */
  CCyControlEndPoint*	mEpCtrlIn;

  /**
  * 制御データ送信用エンドポイント
  */
  CCyControlEndPoint*	mEpCtrlOut;

  /**
  * ISDB_SのTSデータ受信用エンドポイント
  */
  CCyBulkEndPoint*	mEpDataS;

  /**
  * ISDB_TのTSデータ受信用エンドポイント
  */
  CCyBulkEndPoint*	mEpDataT;

  /**
  * データ受信スレッドルーチン
  */
  static unsigned CALLBACK processXferThread(void *arg);

  /**
  * ファームウェアのロードを行う。
  */
  void loadFW(uint8 devNum);

  EzUsbFx2Lp(EzUsbFx2Lp const&) = delete;
  EzUsbFx2Lp(EzUsbFx2Lp&&) = delete;
  EzUsbFx2Lp& operator =(EzUsbFx2Lp const&) = delete;
  EzUsbFx2Lp& operator =(EzUsbFx2Lp&&) = delete;
};

}
}