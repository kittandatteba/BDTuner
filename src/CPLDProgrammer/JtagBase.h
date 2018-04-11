#pragma once
#include <cstdint>
#include <array>
#include <unordered_map>
#include <unordered_set>

class JtagBase {

public:

  JtagBase();

  virtual ~JtagBase() = default;

  void reset();

  void transitState(const tstring& nextState, uint8_t tdi = 0x00, uint8_t smask = 0x00, uint8_t tdo = 0x00, uint8_t mask = 0x00);

  void xferBit(uint8_t tdi, uint8_t smask, uint8_t tdo, uint8_t mask, uint8_t tms = 0x00);

  void flush();

  inline uint32_t getUnmatch() {
    return this->unmatch;
  }

  enum KeyName {
    endIrState,
    endDrState,
    runTestState,
    endTestState,
    smask,
    mask
  };

  inline tstring& getDefaultValue(KeyName key) {
    return this->defaultValue[key];
  }

  inline tstring& putDefaultValue(KeyName key, const tstring& value) {
    if (!value.empty()) {
      this->defaultValue[key] = value;
    }
    return this->defaultValue[key];
  }

protected:

  virtual void open() = 0;

  virtual int32_t write(uint8_t *buffer, int32_t length) = 0;

  virtual int32_t read(uint8_t *buffer, int32_t length) = 0;

private:

  const std::unordered_map<tstring, std::array<tstring, 2>> nextState = {
    // key:現ステート   value[0]:TMS=0での遷移先ステート,  value[1]:TMS=1での遷移先ステート
    { _T("RESET"),      std::array<tstring, 2>{ _T("IDLE"),       _T("RESET")     } },
    { _T("IDLE"),       std::array<tstring, 2>{ _T("IDLE"),       _T("DRSELECT")  } },
    { _T("DRSELECT"),   std::array<tstring, 2>{ _T("DRCAPTURE"),  _T("IRSELECT")  } },
    { _T("DRCAPTURE"),  std::array<tstring, 2>{ _T("DRSHIFT"),    _T("DREXIT1")   } },
    { _T("DRSHIFT"),    std::array<tstring, 2>{ _T("DRSHIFT"),    _T("DREXIT1")   } },
    { _T("DREXIT1"),    std::array<tstring, 2>{ _T("DRPAUSE"),    _T("DRUPDATE")  } },
    { _T("DRPAUSE"),    std::array<tstring, 2>{ _T("DRPAUSE"),    _T("DREXIT2")   } },
    { _T("DREXIT2"),    std::array<tstring, 2>{ _T("DRSHIFT"),    _T("DRUPDATE")  } },
    { _T("DRUPDATE"),   std::array<tstring, 2>{ _T("IDLE"),       _T("DRSELECT")  } },
    { _T("IRSELECT"),   std::array<tstring, 2>{ _T("IRCAPTURE"),  _T("RESET")     } },
    { _T("IRCAPTURE"),  std::array<tstring, 2>{ _T("IRSHIFT"),    _T("IREXIT1")   } },
    { _T("IRSHIFT"),    std::array<tstring, 2>{ _T("IRSHIFT"),    _T("IREXIT1")   } },
    { _T("IREXIT1"),    std::array<tstring, 2>{ _T("IRPAUSE"),    _T("IRUPDATE")  } },
    { _T("IRPAUSE"),    std::array<tstring, 2>{ _T("IRPAUSE"),    _T("IREXIT2")   } },
    { _T("IREXIT2"),    std::array<tstring, 2>{ _T("IRSHIFT"),    _T("IRUPDATE")  } },
    { _T("IRUPDATE"),   std::array<tstring, 2>{ _T("IDLE"),       _T("DRSELECT")  } }
  };

  const std::unordered_map<tstring, std::unordered_set<tstring>> tmsZeroRouteStates = {
    // key:現ステート   value:TMS=0の遷移ルートを選択すべき最終ステートの集合
    { _T("RESET"),      { _T("IDLE"), _T("DRSELECT"), _T("DRCAPTURE"), _T("DRSHIFT"), _T("DREXIT1"), _T("DRPAUSE"), _T("DREXIT2"), _T("DRUPDATE"), _T("IRSELECT"), _T("IRCAPTURE"), _T("IRSHIFT"), _T("IREXIT1"), _T("IRPAUSE"), _T("IREXIT2"), _T("IRUPDATE") } },
    { _T("IDLE"),       { _T("IDLE") } },
    { _T("DRSELECT"),   { _T("DRCAPTURE"), _T("DRSHIFT"), _T("DREXIT1"), _T("DRPAUSE"), _T("DREXIT2"), _T("DRUPDATE") } },
    { _T("DRCAPTURE"),  { _T("DRSHIFT") } },
    { _T("DRSHIFT"),    { _T("DRSHIFT") } },
    { _T("DREXIT1"),    { _T("DRPAUSE"), _T("DREXIT2") } },
    { _T("DRPAUSE"),    { _T("DRPAUSE") } },
    { _T("DREXIT2"),    { _T("DRSHIFT"), _T("DREXIT1"), _T("DRPAUSE"), _T("DREXIT2") } },
    { _T("DRUPDATE"),   { _T("IDLE") } },
    { _T("IRSELECT"),   { _T("IRCAPTURE"), _T("IRSHIFT"), _T("IREXIT1"), _T("IRPAUSE"), _T("IREXIT2"), _T("IRUPDATE") } },
    { _T("IRCAPTURE"),  { _T("IRSHIFT") } },
    { _T("IRSHIFT"),    { _T("IRSHIFT") } },
    { _T("IREXIT1"),    { _T("IRPAUSE"), _T("IREXIT2") } },
    { _T("IRPAUSE"),    { _T("IRPAUSE") } },
    { _T("IREXIT2"),    { _T("IRSHIFT"), _T("IREXIT1"), _T("IRPAUSE"), _T("IREXIT2") } },
    { _T("IRUPDATE"),   { _T("IDLE") } }
  };

  std::unordered_map<KeyName, tstring> defaultValue = {
    { endIrState,   _T("IDLE") },
    { endDrState,   _T("IDLE") },
    { runTestState, _T("IDLE") },
    { endTestState, _T("IDLE") },
    { smask,        _T("")     },
    { mask,         _T("")     }
  };

  static constexpr uint32_t USB_BUFSIZE = 64;
  uint8_t readBuff[USB_BUFSIZE];
  uint8_t writeBuff[USB_BUFSIZE];
  uint32_t index;
  uint32_t unmatch;

  tstring state;

  JtagBase(const JtagBase&) = delete;
  JtagBase(JtagBase&&) = delete;
  JtagBase& operator =(const JtagBase&) = delete;
  JtagBase& operator =(JtagBase&&) = delete;
};
