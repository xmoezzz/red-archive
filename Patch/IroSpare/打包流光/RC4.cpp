#include "StreamDecoder.h"

Void NTAPI RC4Init(PByte s, PByte key, ULONG Len)
{
	ULONG i = 0, j = 0;
	PByte k = (PByte)AllocStack(256);
	Byte tmp = 0;
	for (i = 0; i<256; i++)
	{
		s[i] = i;
		k[i] = key[i%Len];
	}
	for (i = 0; i<256; i++)
	{
		j = (j + s[i] + k[i]) % 256;
		tmp = s[i];
		s[i] = s[j];
		s[j] = tmp;
	}
	RtlZeroMemory(k, 256);
}

Void NTAPI Rc4Worker(PBYTE s, PBYTE Data, ULONG Len)
{
	ULONG i = 0, j = 0, t = 0;
	BYTE tmp;

	for (ULONG k = 0; k < Len; k++)
	{
		i = (i + 1) % 256;
		j = (j + s[i]) % 256;
		tmp = s[i];
		s[i] = s[j];
		s[j] = tmp;
		t = (s[i] + s[j]) % 256;
		Data[k] ^= s[t];
	}
}


Void NTAPI RC4Decoder(PBYTE Buffer, ULONG_PTR Length, DWORD Key)
{
	PBYTE KeyTable = (PBYTE)AllocStack(256);

	DWORD seed = 0xb29d5a0c;
	DWORD crc = 0;

	for (DWORD n = 0; n < 256; n++) 
	{
		KeyTable[n] = n;
		KeyTable[n] ^= seed;
		crc = (u16)((32 - n) * KeyTable[n]) + crc;
		seed = ~(((seed >> 15) & 1) + 2 * seed - 0x5c4c8937);
		KeyTable[n] = seed;
	}

	Rc4Worker(KeyTable, Buffer, Length);
	RtlZeroMemory(KeyTable, 256);
}


