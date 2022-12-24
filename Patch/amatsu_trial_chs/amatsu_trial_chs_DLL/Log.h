#pragma once

#include <Windows.h>

DWORD NTAPI OutputInfo(LPCSTR  lpString);
DWORD NTAPI OutputInfo(LPCWSTR lpString);
DWORD NTAPI OutputInfo(LPCSTR lpString, INT CodePage);
