// ShinkuDecoder.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>
#include <Windows.h>

using std::wstring;

#pragma pack(push, 4)
struct tTVPXP3ExtractionFilterInfo2
{
	DWORD SizeOfSelf; // structure size of tTVPXP3ExtractionFilterInfo itself
	DWORD Offset; // offset of the buffer data in uncompressed stream position
	void * Buffer; // target data buffer
	DWORD BufferSize; // buffer size in bytes pointed by "Buffer"
	DWORD FileHash; // hash value of the file (since inteface v2)
};
#pragma pack(pop)


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


int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2)
	{
		return -1;
	}

	FILE* fin = _wfopen(argv[1], L"rb");
	rewind(fin);
	fseek(fin, 0, SEEK_END);
	DWORD Size = ftell(fin);
	rewind(fin);
	unsigned char* pBuffer = new unsigned char[Size];
	fread(pBuffer, 1, Size, fin);
	fclose(fin);

	tTVPXP3ExtractionFilterInfo2 info;
	info.Buffer = pBuffer;
	info.BufferSize = Size;
	info.FileHash = 0xF56AC2E1;
	info.Offset = 0;
	info.SizeOfSelf = sizeof(tTVPXP3ExtractionFilterInfo2);

	ExtractionFilter(&info);

	wstring FileName(argv[1]);
	FileName += L".out";
	FILE* file = _wfopen(FileName.c_str(), L"wb");
	fwrite(pBuffer, 1, Size, file);
	fclose(file);

	ExtractionFilter(&info);
	wstring FileName2(argv[1]);
	FileName2 += L".in";
	FILE* file2 = _wfopen(FileName2.c_str(), L"wb");
	fwrite(pBuffer, 1, Size, file2);
	fclose(file2);
	return 0;
}

