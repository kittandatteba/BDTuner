#include "stdafx.h"
#include "SvfParser.h"
#include "ShiftCommand.h"
#include "RunTestCommand.h"
#include "StateCommand.h"
#include "EndCommand.h"


std::unique_ptr<BaseCommand> SvfParser::parse(tstring & line) {

  tregex reShift(_T(R"(S([DI])R\s*([0-9]+)\s*TDI\s*\(([0-9A-F]+)\)\s*(?:SMASK\s\(([0-9A-F]+)\))?\s*(?:TDO\s*\(([0-9A-F]+)\))?\s*(?:MASK\s*\(([0-9A-F]+)\))?\s*;)"), std::regex_constants::icase);
  tregex reRunTest(_T(R"(RUNTEST\s*(IRPAUSE|DRPAUSE|RESET|IDLE)?\s*([0-9]+)\s*TCK\s*(?:ENDSTATE\s*(IRPAUSE|DRPAUSE|RESET|IDLE))?\s*;)"), std::regex_constants::icase);
  tregex reState(_T(R"(STATE\s*(IRPAUSE|DRPAUSE|RESET|IDLE)\s*;)"), std::regex_constants::icase);
  tregex reEnd(_T(R"(END([DI])R\s*(IRPAUSE|DRPAUSE|RESET|IDLE)\s*;)"), std::regex_constants::icase);
  tregex reIgnore(_T(R"((?:FREQUENCY|TRST|T[DI]R|H[DI]R).*;)"), std::regex_constants::icase);

  tsmatch match;
  if (std::regex_match(line, match, reShift)) {

    // SDR or SIR
    return std::unique_ptr<BaseCommand>(new ShiftCommand(match[1].str(), match[2].str(), match[3].str(), match[4].str(), match[5].str(), match[6].str()));

  } else if (std::regex_match(line, match, reRunTest)) {

    // RUNTEST
    return std::unique_ptr<BaseCommand>(new RunTestCommand(match[1].str(), match[2].str(), match[3].str()));

  } else if (std::regex_match(line, match, reState)) {

    // STATE
    return std::unique_ptr<BaseCommand>(new StateCommand(match[1].str()));

  } else if (std::regex_match(line, match, reEnd)) {

    // ENDDR or ENDIR
    return std::unique_ptr<BaseCommand>(new EndCommand(match[1].str(), match[2].str()));

  } else if (std::regex_match(line, match, reIgnore)) {

    // 無視する命令
    return std::unique_ptr<BaseCommand>(new BaseCommand());;

  } else {

    // パース不可能
    tcerr << _T("parse error. command = [") << line << _T("]") << std::endl;
    throw std::runtime_error("parse error");
  }

  return nullptr;
}