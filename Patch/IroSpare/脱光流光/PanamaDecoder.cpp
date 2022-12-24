#include "StreamDecoder.h"
#include "MyHash.h"

extern "C"
{
#include "panama.h"
}


//256Bit
Void NTAPI PanamaDecoder(PBYTE Buffer, ULONG_PTR Length, DWORD Key)
{
	PANAMA_KEY S[1];
	PBYTE      iKeyiV;
	DWORD      InfoKey[16];
	DWORD      Reg;

	iKeyiV = (PBYTE)AllocStack(64);
	Reg = Key;
	for (ULONG i = 0; i < countof(InfoKey); i++)
	{
		InfoKey[i] = Reg * Reg;
		Reg *= 0x128456;
	}

	salsa10_hash((PDWORD)iKeyiV, InfoKey);

	RtlZeroMemory(InfoKey, sizeof(InfoKey));
	_mcrypt_panama_set_key(S, (PCHAR)iKeyiV, 32, (PCHAR)iKeyiV + 32, 32);
	_mcrypt_panama_decrypt(S, Buffer, Length);
	RtlZeroMemory(iKeyiV, 64);
	RtlZeroMemory(S, sizeof(S[0]));
}
