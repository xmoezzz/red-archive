#ifndef _BinaryCode_
#define _BinaryCode_

#include <Windows.h>
#include "detours.h"
#include <cstdio>
#include <string>

using std::string;

VOID WINAPI InstallBinaryCode();

VOID WINAPI InstallSaveData();

VOID WINAPI InstallSaveCode();

VOID WINAPI InstallSaveBin();

#endif
