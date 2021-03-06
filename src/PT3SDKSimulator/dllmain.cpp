// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"
#include "BDUtil.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {

  switch (ul_reason_for_call) {

  case DLL_PROCESS_ATTACH:

#ifdef _DEBUG
    ::_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    ::_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    ::_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);

    ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF |
    //  _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    EARTH::PT::BDUtil::readIniFile(hModule);
    break;

  case DLL_THREAD_ATTACH:
    break;

  case DLL_THREAD_DETACH:
    break;

  case DLL_PROCESS_DETACH:

#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif

    break;
  }

  return TRUE;
}
