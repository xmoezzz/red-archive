#include "Salsa20.h"
#include "my.h"
#include "mt19937ar.h"
#include "CubeHashWarpper.h"

Void NTAPI Salsa20Decoder(PBYTE Buffer, ULONG Length, DWORD Key)
{
	SaECRYPT_ctx S[1];
	PBYTE iKey, iV;
	BYTE  InitInfo[64];
	DWORD CurKey;

	iKey = (PBYTE)AllocStack(32);
	iV   = (PBYTE)AllocStack(32);
	CurKey = Key;

	for (ULONG_PTR i = 0; i < 64; i++)
	{
		CurKey *= 0x5612;
		InitInfo[i] = (BYTE)CurKey;
		CurKey ^= 0x13151418;
	}

	Cubehash256(InitInfo + 0,  32, iKey);
	Cubehash256(InitInfo + 32, 32, iV);

	RtlZeroMemory(InitInfo, sizeof(InitInfo));

	init_genrand(NTDLL_AlpcFreeCompletionListMessage ^ Key ^ (DWORD)NtGetTickCount() * Length);
	SaECRYPT_keysetup(S, iKey, 256, genrand_int32());
	SaECRYPT_ivsetup(S, iV);
	SaECRYPT_encrypt_bytes(S, Buffer, Buffer, Length);

	RtlZeroMemory(S, sizeof(S[0]));
}
