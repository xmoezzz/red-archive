#pragma once

#include <Windows.h>
#include "NtDefine.h"

NTSTATUS WINAPI
LoadDllFromMemory(
PVOID           DllBuffer,
ULONG           DllBufferSize,
PUNICODE_STRING ModuleFileName,
PVOID*          ModuleHandle,
ULONG           Flags);
