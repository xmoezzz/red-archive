#ifndef _Koikake_chs_
#define _Koikake_chs_

#include <Windows.h>
#include <string>
#include <vector>
#include "detours.h"
#include <memory>
#include <malloc.h>

using std::vector;
using std::wstring;
using std::string;

#pragma comment(lib,"detours.lib")

HRESULT WINAPI InitHook();
HRESULT WINAPI UnInitHook();

#endif
