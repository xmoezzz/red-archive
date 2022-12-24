#include "Sosemanuk.h"
#include "my.h"
#include "CubeHashWarpper.h"

Void NTAPI SosemanukDecoder(PBYTE Buffer, ULONG Length, DWORD Key)
{
	SoECRYPT_ctx S[1];
	PBYTE        iKeyiV;
	BYTE         KeyInfo[16];
	DWORD        CurKey;

	iKeyiV = (PBYTE)AllocStack(64);
	CurKey = Key;

	for (ULONG_PTR i = 0; i < 4; i++)
	{
		Key *= 0x5478;
		KeyInfo[4*i + 0] = (BYTE)Key;
		KeyInfo[4*i + 1] = (BYTE)Key >> 8;
		KeyInfo[4*i + 2] = (BYTE)Key >> 16;
		KeyInfo[4*i + 3] = (BYTE)Key >> 24;
	}
	CurKey = 0;

	Cubehash512(KeyInfo, sizeof(KeyInfo), iKeyiV);
	RtlZeroMemory(KeyInfo, sizeof(KeyInfo));

	SoECRYPT_keysetup(S, iKeyiV,    256, 256);
	SoECRYPT_ivsetup (S, iKeyiV + 32);
	RtlZeroMemory(iKeyiV, 64);

	SoECRYPT_encrypt_bytes(S, Buffer, Buffer, Length);

	RtlZeroMemory(S, sizeof(S[0]));
}
