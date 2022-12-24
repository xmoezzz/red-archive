#include "StreamDecoder.h"
#include "MyHash.h"
#include "ShinkuDef.h"
extern "C"
{
#include "phelix.h"
}


Void NTAPI PhelixDecoder(PBYTE Buffer, ULONG_PTR Length, DWORD Key)
{
	PhelixContext S[1];
	PBYTE         iKeyiV;
	PDWORD        pDwKey;
	BYTE          InitInfo[16];
	DWORD         Reg;

	iKeyiV = (PBYTE)AllocStack(64);
	pDwKey = (PDWORD)iKeyiV;
	Reg = Key;
	for (ULONG i = 0; i < sizeof(InitInfo); i++)
	{
		Reg = Reg * 0x1542;
		Nt_DecodePointer(Reg, i);
		InitInfo[i] = Reg * Key;
	}
	Reg = 0;
	for (ULONG i = 0; i < 64 / sizeof(DWORD); i++)
	{
		Reg      += MyHash(InitInfo, sizeof(InitInfo));
		pDwKey[i] = Reg * 0x84512;
		Nt_DecodePointer(Reg, i);
	}
	Reg = 0;
	RtlZeroMemory(InitInfo, sizeof(InitInfo));

	PBYTE OutputMem = (PBYTE)AllocateMemoryP(Length);

	PhelixSetupKey(S, iKeyiV, 256, 128, 128);
	PhelixDecryptBytes(S, Buffer, OutputMem, Length);
	RtlZeroMemory(S, sizeof(S[0]));
	RtlZeroMemory(iKeyiV, 64);

	RtlCopyMemory(Buffer, OutputMem, Length);
	RtlFillMemory(OutputMem, Length, 0xCC);
	FreeMemoryP(OutputMem);
}

