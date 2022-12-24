#ifndef _Compressor_
#define _Compressor_

#include <Windows.h>
#define FASTCALL __fastcall

BOOL
FASTCALL
UCL_NRV2E_Compress(
PVOID   Input,
ULONG   InputSize,
PVOID   Output,
PULONG  OutputSize,
LONG    Level
);


BOOL
FASTCALL
UCL_NRV2E_Decompress(
PVOID   Input,
ULONG   InputSize,
PVOID   Output,
PULONG  OutputSize
);

ULONG_PTR FASTCALL UCL_NRV2E_DecompressASMFast32(PVOID /* pvInput */, PVOID /* pvOutput */);



#endif
