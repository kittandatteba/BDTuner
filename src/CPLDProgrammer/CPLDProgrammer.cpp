// CPLDProgrammer.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <stdexcept>
#include "SvfParser.h"
#include "JtagImpl.h"


int _tmain(int argc, _TCHAR* argv[]) {

#ifdef _DEBUG
  // メモリリーク情報ダンプ先を標準出力に設定
  ::_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
  ::_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);

  ::_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
  ::_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);

  ::_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
  ::_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

  ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF |
    _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  auto result = 0;

  try {

    if (argc < 2) {
      tcerr << argv[0] << _T(" file") << std::endl;
      throw std::runtime_error("parameter error");
    }

    tstring fileName(argv[1]);
    if (fileName.empty()) {
      tcerr << _T("specify SVF file") << std::endl;
      throw std::runtime_error("file not found");
    }

    tifstream ifs(fileName);
    if (ifs.fail()) {
      tcerr << _T("can't open ") << fileName << std::endl;
      throw std::runtime_error("file open error");
    }

    JtagImpl jtagDevice;
    jtagDevice.open();
    jtagDevice.reset();

    tstring line;
    while (std::getline(ifs, line)) {

      if (line.empty() || line[0] == _T('!')) {
        continue;
      }

      tcout << "[" << line << "]" << std::endl;

      auto command = SvfParser::parse(line);
      command->execute(jtagDevice);
    }

    jtagDevice.flush();
    if (jtagDevice.getUnmatch() != 0) {
      tcerr << _T("TDO unmatch count=") << jtagDevice.getUnmatch() << std::endl;
      throw std::runtime_error("TDO unmatch");
    }

    tcout << _T("finished normal") << std::endl;

  } catch (std::exception &e) {
    std::cerr << "[error] " << e.what() << std::endl;
    result = -1;
  }

#ifdef _DEBUG
  // メモリリーク情報ダンプ
  _CrtDumpMemoryLeaks();
#endif

  return result;
}
