#ifndef _NtProtect_
#define _NtProtect_

#include <Windows.h>
#include "detours.h"

HRESULT WINAPI SetupNtProtect(HMODULE hModule);
HRESULT WINAPI ThreadWorker(HMODULE hModule);

#endif
