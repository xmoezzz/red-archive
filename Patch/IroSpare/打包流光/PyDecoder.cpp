#include "StreamDecoder.h"
#include "py.h"
#include "ShinkuDef.h"

Void NTAPI PyDecoder(PBYTE Buffer, ULONG_PTR Length, DWORD Key)
{
	PyECRYPT_ctx S[1];
	PBYTE        iKeyiV;
	DWORD        Reg;

	iKeyiV = (PBYTE)AllocStack(32);
	Reg = Key;
	for (ULONG i = 0; i < 32 / sizeof(DWORD); i++)
	{
		iKeyiV[4 * i + 0] = Reg;
		iKeyiV[4 * i + 1] = Reg >> 8;
		iKeyiV[4 * i + 2] = Reg >> 16;
		iKeyiV[4 * i + 3] = Reg >> 24;

		Reg *= 0x5417;
		Nt_DecodePointer(Reg, i);
	}

	PyECRYPT_keysetup(S, iKeyiV, 128, 128);
	PyECRYPT_ivsetup(S, iKeyiV + 16);
	PyECRYPT_process_bytes(0, S, Buffer, Buffer, Length);

	RtlZeroMemory(S, sizeof(S[0]));
	RtlZeroMemory(iKeyiV, 32);
}

