#include "StreamDecoder.h"
#include "GlobalMap.h"
#include "NodeKeyDef.h"
#include "Blake2Wrapper.h"

u8 n = 0;
u8 s = 0;


void InitKey(PBYTE key, PBYTE iv, PBYTE P)
{
	long m, i;
	u8 temp;
	s = 0;

	for (i = 0; i != 256; i++)
	{
		P[i] = (u8)i;
	}

	for (m = 0; m != 768; m++)
	{
		s = P[(s + P[m & 0xff] + key[m % 64]) & 0xff];
		temp = P[m & 0xff];
		P[m & 0xff] = P[s & 0xff];
		P[s & 0xff] = temp;
	}

	for (m = 0; m != 768; m++)
	{
		s = P[(s + P[m & 0xff] + iv[m % 64]) & 0xff];
		temp = P[m & 0xff];
		P[m & 0xff] = P[s & 0xff];
		P[s & 0xff] = temp;
	}

	for (m = 0; m != 768; m++)
	{
		s = P[(s + P[m & 0xff] + key[m % 64]) & 0xff];
		temp = P[m & 0xff];
		P[m & 0xff] = P[s & 0xff];
		P[s & 0xff] = temp;
	}

	n = 0;
}

void Crypt(PBYTE input, PBYTE output, PBYTE P, ULONG_PTR len)
{
	u8 z, temp;
	for (ULONG_PTR i = 0; i != len; i++)
	{
		s = P[(s + P[n & 0xff]) & 0xff];
		z = P[(P[(P[s & 0xff]) & 0xff] + 1) & 0xff];
		temp = P[n & 0xff];
		P[n & 0xff] = P[s & 0xff];
		P[s & 0xff] = temp;
		n = (u8)((n + 1) & 0xff);
		output[i] = (u8)(input[i] ^ z);
	}
}


Void NTAPI VMPCDecoder(PBYTE Buffer, ULONG Length, DWORD Key)
{
	PBYTE iKey, iV, PContainer, NodeKey;
	DWORD InitKey[4];
	BYTE  Reg;

	NodeKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(VMPC_BaseKey_Mask);
	if (!NodeKey)
		Ps::ExitProcess(0);

	iKey = (PBYTE)AllocStack(64);
	iV   = (PBYTE)AllocStack(64);
	PContainer = (PBYTE)AllocStack(256);
	
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
	blake2bp_buffer((PBYTE)InitKey, countof(InitKey) * sizeof(DWORD), iKey);
	blake2b_buffer ((PBYTE)InitKey, countof(InitKey) * sizeof(DWORD), iV);

	Reg = (BYTE)Key;
	for (ULONG_PTR i = 0; i < 256; i++)
	{
		PContainer[i] = NodeKey[i] ^ Key;
	}

	Crypt(Buffer, Buffer, PContainer, Length);

	RtlZeroMemory(iKey, 64);
	RtlZeroMemory(iV, 64);
	RtlZeroMemory(PContainer, 256);
}
