#include "StreamDecoder.h"
#include "Edon80.h"

Void NTAPI Edon80Decoder(PBYTE Buffer, ULONG_PTR Length, DWORD Key)
{
	Edon80ECRYPT_ctx S[1];
	PBYTE            iKeyiV;
	DWORD            Reg;

	iKeyiV = (PBYTE)AllocStack(32);
	Reg = Key;
	for (ULONG i = 0; i < 32 / sizeof(DWORD); i++)
	{
		Reg ^= 0x58726141;
		Reg += Key;
		Reg *= 0x514;

		iKeyiV[i + 1] = Reg >> 24;
		iKeyiV[i + 3] = Reg >> 16;
		iKeyiV[i + 0] = Reg >> 8;
		iKeyiV[i + 2] = Reg;
	}
	Reg = 0;

	Edon80ECRYPT_keysetup(S, iKeyiV, 128, 128);
	Edon80ECRYPT_ivsetup(S,  iKeyiV + 16);
	RtlZeroMemory(iKeyiV, 32);

	PBYTE OutputMem = (PBYTE)AllocateMemoryP(Length);

	Edon80ECRYPT_decrypt_bytes(S, Buffer, OutputMem, Length);

	RtlCopyMemory(Buffer, OutputMem, Length);
	RtlFillMemory(OutputMem, Length, 0xCC);
	FreeMemoryP(OutputMem);

	RtlZeroMemory(S, sizeof(S[0]));
}
