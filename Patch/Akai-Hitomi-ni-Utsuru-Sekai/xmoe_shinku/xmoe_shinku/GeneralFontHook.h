#ifndef _GeneralFontHook_
#define _GeneralFontHook_

#include <Windows.h>
#include <Psapi.h>
#include "detours.h"

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "detours.lib")

BOOL WINAPI InstallFont();


#endif
