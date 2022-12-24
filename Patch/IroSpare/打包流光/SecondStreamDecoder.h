#pragma once

#include "my.h"

Void NTAPI DragonDecoder(PBYTE Buffer, ULONG Length, DWORD Key, BYTE Selector);
Void NTAPI FinalDecoder(PBYTE Buffer, ULONG Length, BYTE Key);
