#pragma once

#include "my.h"

Void NTAPI CxdecDecoder(DWORD hash, DWORD offset, PBYTE buffer, DWORD bufferlen);
Void NTAPI RC4Decoder(PBYTE Buffer, ULONG_PTR Length, DWORD Key);
Void NTAPI BaseDecoder1(PBYTE buf, DWORD len, DWORD key);
Void NTAPI BaseDecoder2(PBYTE buf, DWORD len, DWORD offset, DWORD hash);
Void NTAPI BaseDecoder3(PBYTE buf, DWORD len, DWORD key);
Void NTAPI VMPCDecoder(PBYTE Buffer, ULONG Length, DWORD Key);
Void NTAPI HC128Decoder(PBYTE Buffer, ULONG Length, DWORD Key);
Void NTAPI Salsa20Decoder(PBYTE Buffer, ULONG Length, DWORD Key);
Void NTAPI SosemanukDecoder(PBYTE Buffer, ULONG Length, DWORD Key);
Void NTAPI RabbitDecoder(PBYTE Buffer, ULONG Length, DWORD Key);
Void NTAPI NekoXcodeDecoder(PBYTE Buffer, ULONG Length, DWORD Key);
Void NTAPI BaseDecoder3(PBYTE buf, DWORD len, DWORD key);
Void NTAPI PanamaDecoder(PBYTE Buffer, ULONG_PTR Length, DWORD Key);
Void NTAPI PhelixDecoder(PBYTE Buffer, ULONG_PTR Length, DWORD Key);
Void NTAPI ChachaDecoder(PBYTE Buffer, ULONG_PTR Length, DWORD Key);
Void NTAPI PyDecoder(PBYTE Buffer, ULONG_PTR Length, DWORD Key);
Void NTAPI Edon80Decoder(PBYTE Buffer, ULONG_PTR Length, DWORD Key);
