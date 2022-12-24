#include "ScriptInfo.h"


void WINAPI ExtractionFilter(tTVPXP3ExtractionFilterInfo2 *Info)
{
	int v2; // eax@2
	int v3; // ST14_4@3
	int TempHash; // eax@5
	DWORD zPos; // ecx@5
	unsigned int iPos; // esi@7
	unsigned char InfoTable[32] = { 0 }; // [sp+8h] [bp-24h]@1


	TempHash = ((Info->FileHash & 0x7FFFFFFF) << 31) | Info->FileHash & 0x7FFFFFFF;
	zPos = 0;
	do
	{
		InfoTable[zPos++] = TempHash;
		TempHash = ((TempHash & 0xFFFFFFFE) << 23) | ((unsigned int)TempHash >> 8);
	} while (zPos < 31);
	iPos = 0;
	unsigned char Key = 0;
	if (Info->BufferSize)
	{
		do
		{
			TempHash = InfoTable[(Info->Offset + iPos) % 0x1F];
			Key = TempHash & 0xFF;
			*((BYTE *)Info->Buffer + iPos++) ^= TempHash;
		} while (iPos < Info->BufferSize);
	}
}


void WINAPI BufferDecoder(unsigned char* pBuffer, DWORD Size)
{
	tTVPXP3ExtractionFilterInfo2 info;
	info.Buffer = pBuffer;
	info.BufferSize = Size;
	info.FileHash = 0xF56AC2E1;
	info.Offset = 0;
	info.SizeOfSelf = sizeof(tTVPXP3ExtractionFilterInfo2);

	ExtractionFilter(&info);
}

