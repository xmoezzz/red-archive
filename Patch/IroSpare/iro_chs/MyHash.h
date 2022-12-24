#pragma once

#include "my.h"

UInt32 MyHash(LPVoid lpBuffer, Int32 BufferSize);
void salsa10_hash(DWORD x[16], DWORD in[16]);
