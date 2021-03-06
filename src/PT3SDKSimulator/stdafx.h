// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
// Windows ヘッダー ファイル:
#include <windows.h>



// TODO: プログラムに必要な追加ヘッダーをここで参照してください
#include <tchar.h>
#include <string>
#include <sstream>

// ワイド文字・マルチバイト文字共用型 tstring
typedef std::basic_string<TCHAR> tstring;
typedef std::basic_stringstream<TCHAR> tstringstream;

#ifdef _DEBUG
// メモリリーク検出用設定（Debug向け）
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#ifdef  UNICODE
// トレース出力マクロ（Debug向け：ワイド文字版）
#define TRACE_F(expr) _CrtDbgReportW(_CRT_WARN, __FILEW__, __LINE__, nullptr, \
	L"[%ls](%u) %ls", __FUNCTIONW__, GetCurrentThreadId(), \
	static_cast<std::wstringstream&>(std::wstringstream() << expr << std::endl).str().c_str())
#else
// トレース出力マクロ（Debug向け：マルチバイト文字版）
#define TRACE_F(expr) _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, nullptr, \
	"[%s](%u) %s", __FUNCTION__, GetCurrentThreadId(), \
	static_cast<std::stringstream&>(std::stringstream() << expr << std::endl).str().c_str())
#endif
#else
// トレース出力マクロ（Release向け：何もしない）
#define TRACE_F(expr)
#endif

#if defined(_DLL) || defined(_DEBUG)
#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#endif
