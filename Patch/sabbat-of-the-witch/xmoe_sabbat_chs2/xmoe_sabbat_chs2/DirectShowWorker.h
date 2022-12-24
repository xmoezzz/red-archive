#ifndef _DirectShowWorker_
#define _DirectShowWorker_

#include <Windows.h>
#include <DShow.h>
#include <string>

using std::wstring;

HRESULT WINAPI InitDirectShowFilter();
HRESULT WINAPI UnInitDirectShowFilter();

#endif
