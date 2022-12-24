#ifndef _GenMD5Code_
#define _GenMD5Code_

#include "md5.h"
#include <string>
#include <Windows.h>

using std::wstring;

VOID WINAPI GenMD5Code(const WCHAR* FileName, wstring& OutHex);

VOID WINAPI GenMD5CodeFile(const unsigned char* Buffer, ULONG Len, wstring& OutHex);

#endif
