#pragma once
#include <memory>
#include <CyAPI.h>
#include "JtagBase.h"

class JtagImpl : public JtagBase {


public:
  JtagImpl();

  virtual ~JtagImpl() override = default;

  virtual void open() override;

  virtual int32_t write(uint8_t * buffer, int32_t length) override;

  virtual int32_t read(uint8_t * buffer, int32_t length) override;

private:

  static constexpr wchar_t FX2FW_NAME[] = L"FX2_BTBN";

  static constexpr uint8_t FX2FW_DATA[] = {
    #include "FX2FarmwareBitBang.inc"
  };

  static constexpr int32_t RENUM_RETRY_COUNT = 40;    // ReNumerationの最大リトライ回数
  static constexpr int32_t RENUM_RETRY_INTRVAL = 100; // ReNumeration のリトライ間隔(msec)

  enum {
    EPA_CTL_IN = 0x81U,		// 制御データ受信用（EP1 IN）
    EPA_CTL_OUT = 0x01U,	// 制御データ送信用（EP1 OUT）
  };

  std::unique_ptr<CCyUSBDevice> m_UsbDevice;

  CCyControlEndPoint*	m_EpCtrlIn;
  CCyControlEndPoint*	m_EpCtrlOut;

  void loadFW(uint8_t id);

  JtagImpl(const JtagImpl&) = delete;
  JtagImpl(JtagImpl&&) = delete;
  JtagImpl& operator =(const JtagImpl&) = delete;
  JtagImpl& operator =(JtagImpl&&) = delete;
};

