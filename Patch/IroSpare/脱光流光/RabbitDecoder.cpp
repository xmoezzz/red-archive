#include "Rabbit.h"
#include "my.h"
#include "JHHash.h"
#include "mt19937ar.h"
#include "ShinkuDef.h"
#include "StreamDecoder.h"

Void NTAPI RabbitDecoder(PBYTE Buffer, ULONG Length, DWORD Key)
{
	RaECRYPT_ctx S[1];
	PBYTE iKeyiV;
	BYTE  KeyInfo[16];
	DWORD EaxVal;

	iKeyiV = (PBYTE)AllocStack(32);

	auto GenInitKey = [](PBYTE InitKey, ULONG Size, DWORD Key)
	{
		DWORD Reg = Key * 0x4185D;
		DWORD key1 = Key ^ 0x51846325;
		DWORD key0 = key1 * 0x51748;
		for (ULONG_PTR i = 0; i < Size; i++)
		{
			Reg = ((key1 ^ ((key0 ^ key1) + 0x5D588B65)) - 0x359D3E2A) * Reg;
			DWORD v = (key1 ^ ((key1 ^ Reg) - 0x70E44324)) + 0x6C078965;
			InitKey[i] = v << (Reg >> 27) | v >> (32 - (Reg >> 27));
		}
	};

	GenInitKey(KeyInfo, sizeof(KeyInfo), Key);
	JHHash(256, KeyInfo, sizeof(KeyInfo), iKeyiV);

	__asm
	{
		mov EaxVal, eax;
	}
	init_genrand(Nt_DecodePointer((Key * Length), EaxVal ? EaxVal : 13));
	RaECRYPT_keysetup(S, iKeyiV, genrand_int32(), 128 * genrand_int32()); //16
	RaECRYPT_ivsetup(S, iKeyiV + 16); //8
	RtlZeroMemory(iKeyiV, 32);
	RaECRYPT_decrypt_bytes(S, Buffer, Buffer, Length);
	RtlZeroMemory(S, sizeof(S[0]));
}
