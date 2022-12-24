#include "ecrypt-sync.h"
#include "Blake2Wrapper.h"
#include "StreamDecoder.h"
#include "GlobalMap.h"
#include "NodeKeyDef.h"

Void NTAPI HC128Decoder(PBYTE Buffer, ULONG Length, DWORD Key)
{
	PBYTE      iKey, iV, NodeKey;
	DWORD      InitKey[4];
	ECRYPT_ctx S[1];

	NodeKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(HC128_BaseKey_Mask);
	if (!NodeKey)
		Ps::ExitProcess(0);

	iKey = (PBYTE)AllocStack(32);
	iV = (PBYTE)AllocStack(32);

	auto InitBaseKey = [](PDWORD ibKey, ULONG_PTR Length, DWORD HashKey)
	{
		DWORD RegKey = HashKey;
		for (ULONG_PTR i = 0; i < Length; i++)
		{
			RegKey *= 0x56128475;
			ibKey[i] = RegKey;
		}
	};

	InitBaseKey(InitKey, countof(InitKey), Key);
	blake2sp_buffer((PBYTE)InitKey, countof(InitKey) * sizeof(DWORD), iKey);
	blake2s_buffer ((PBYTE)InitKey, countof(InitKey) * sizeof(DWORD), iV);

	ECRYPT_keysetup(S, iKey, 128, 128);
	ECRYPT_ivsetup(S, iV);
	ECRYPT_process_bytes(1, S, Buffer, Buffer, Length);

	RtlZeroMemory(iKey, 32);
	RtlZeroMemory(iV, 32);
	RtlZeroMemory(S, sizeof(S[0]));
}

