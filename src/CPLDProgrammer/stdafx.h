// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: プログラムに必要な追加ヘッダーをここで参照してください
#include <windows.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <regex>

// TCHAR用
typedef std::basic_string<TCHAR> tstring;
typedef std::basic_stringstream<TCHAR> tstringstream;
typedef std::basic_ifstream<TCHAR> tifstream;
typedef std::basic_regex<TCHAR> tregex;
typedef std::match_results<const TCHAR *> tmatch;
typedef std::match_results<tstring::const_iterator> tsmatch;

#ifdef _UNICODE
#define tcout std::wcout
#define tcerr std::wcerr
#else
#define tcout std::cout
#define tcerr std::cerr
#endif

#ifdef _DEBUG
// メモリリーク検出用設定（Debug向け）
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#if defined(_DLL) || defined(_DEBUG)
#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#endif
