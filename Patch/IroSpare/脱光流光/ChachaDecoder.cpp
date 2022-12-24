#include "StreamDecoder.h"
#include "Chacha.h"

Void NTAPI ChachaDecoder(PBYTE Buffer, ULONG_PTR Length, DWORD Key)
{
	ChaECRYPT_ctx S[1];
	PBYTE         iKeyiV;
	DWORD         Reg;

	iKeyiV = (PBYTE)AllocStack(32);
	Reg = Key;
	for (ULONG i = 0; i < 32; i++)
	{
		Reg *= 0x541;
		iKeyiV[i] = Reg;
	}
	Reg = 0;

	ChaECRYPT_keysetup(S, iKeyiV, 256, KERNEL32_AddAtomA);
	ChaECRYPT_ivsetup(S, iKeyiV + 16);
	ChaECRYPT_decrypt_bytes(S, Buffer, Buffer, Length); 

	RtlZeroMemory(S, sizeof(S[0]));
	RtlZeroMemory(iKeyiV, 32);
}
