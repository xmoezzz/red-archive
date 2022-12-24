#include "my.h"
#include "MyCxdec.h"
#include "blake2.h"
#include <string>

static CCxdec* g_cxdec = NULL;


ForceInline Void blake2bp_buffer(LPCBYTE buffer, size_t length, PBYTE resstream)
{
	blake2bp_state S[1];

	blake2bp_init(S, BLAKE2B_OUTBYTES);
	blake2bp_update(S, buffer, length);
	blake2bp_final(S, resstream, BLAKE2B_OUTBYTES);
}

Void EncodeString(LPSTR Buffer, ULONG Size, ULONG Hash, ULONG Index)
{
	LARGE_INTEGER Offset;

	auto DecryptCxdecInternal = [](ULONG Hash, LARGE_INTEGER Offset, PVOID lpBuffer, ULONG BufferSize, ULONG Index)->BOOL
	{
		PBYTE           pbBuffer;
		ULONG           Mask, Mask2;
		LARGE_INTEGER   CurrentPos;
		CCxdec         *pCxdec;
		Byte            HashTable[64];

		pCxdec = g_cxdec;
		if (pCxdec == NULL)
		{
			pCxdec = g_cxdec = new CCxdec;
		}

		pbBuffer = (PBYTE)lpBuffer;
		Mask = pCxdec->GetMask(Hash);

		Mask2 = LOWORD(Mask);
		CurrentPos.QuadPart = Offset.QuadPart + BufferSize;

		if (Mask2 >= Offset.QuadPart && Mask2 < CurrentPos.QuadPart)
		{
			*(pbBuffer + Mask2 - Offset.LowPart) ^= Hash >> 16;
		}

		Mask2 = HIWORD(Mask);
		if (Mask2 >= Offset.QuadPart && Mask2 < CurrentPos.QuadPart)
		{
			*(pbBuffer + Mask2 - Offset.LowPart) ^= Hash >> 8;
		}

		XorMemory(pbBuffer, Hash, BufferSize);

		union
		{
			ULONG InfoHash[2];
			BYTE  ByteInfo[8];
		}PreHash;

		PreHash.InfoHash[0] = Hash;
		PreHash.InfoHash[1] = Index;

		blake2bp_buffer(PreHash.ByteInfo, 8, HashTable);

		PreHash.InfoHash[0] = PreHash.InfoHash[1] = 0;

		for (ULONG i = 0; i < BufferSize; i++)
			((PBYTE)lpBuffer)[i] ^= HashTable[i % countof(HashTable)];

		RtlZeroMemory(HashTable, sizeof(HashTable));
		return TRUE;
	};

	Offset.QuadPart = 0;
	DecryptCxdecInternal(Hash, Offset, Buffer, Size, Index);
}


BYTE TempTable[512] =
{
	0x7d, 0x54, 0x11, 0xd1, 0xfa, 0xad, 0xb0,
	0xc2, 0x77, 0x0f, 0xe2, 0x5a, 0x95, 0x19, 0x4e,
	0x93, 0x27, 0x43, 0x15, 0x7d, 0xb5, 0x54, 0x48,
	0xba, 0xfb, 0xe9, 0x6d, 0x02, 0x6b, 0xf4, 0xfe,
	0xa0, 0xa7, 0x3b, 0xe9, 0xf3, 0x08, 0xd5, 0x11,
	0xee, 0x1a, 0xb0, 0xcb, 0x98, 0x1d, 0x0e, 0x62,
	0x8d, 0x86, 0x03, 0x94, 0x7b, 0x7b, 0xf9, 0x13,
	0xa5, 0x5b, 0x2c, 0x05, 0x64, 0x34, 0x2f, 0x84,
	0xa1, 0x4b, 0x65, 0x1e, 0x5c, 0x97, 0x88, 0x56,
	0x29, 0x46, 0x25, 0x21, 0xad, 0x37, 0x1e, 0x6b,
	0x25, 0x7e, 0x27, 0x90, 0xe0, 0xe3, 0x4a, 0xe3,
	0xc0, 0x63, 0x63, 0x2a, 0xbd, 0xae, 0xa4, 0x1f,
	0x61, 0xa7, 0x12, 0xf1, 0x4d, 0xe8, 0x06, 0xc1,
	0xb3, 0x3b, 0xad, 0x25, 0xda, 0x21, 0x89, 0xa9,
	0x9d, 0x4f, 0xee, 0x49, 0xec, 0x2c, 0x86, 0xf8,
	0x4a, 0x55, 0xcd, 0x1c, 0x4d, 0x19, 0x95, 0x10,
	0x21, 0xfd, 0x83, 0xa0, 0x05, 0x39, 0x90, 0x91,
	0xcc, 0x39, 0x89, 0x16, 0x5e, 0x1e, 0x8f, 0x5c,
	0x34, 0x3a, 0x98, 0xff, 0xe0, 0x97, 0xed, 0x93,
	0x83, 0x70, 0xaa, 0x1c, 0x54, 0xb6, 0x41, 0x95,
	0x20, 0x8d, 0xf7, 0x6d, 0xc4, 0xcc, 0x65, 0x06,
	0xb5, 0x81, 0xf8, 0x35, 0x79, 0x6b, 0x71, 0xc4,
	0x2b, 0x7e, 0x66, 0xf3, 0xfb, 0x62, 0xbf, 0xf2,
	0xab, 0xf4, 0x3a, 0x69, 0x13, 0xc4, 0xe8, 0xf1,
	0x9e, 0x95, 0xae, 0x98, 0xcb, 0xe1, 0xc5, 0x60,
	0xad, 0x52, 0x3a, 0xc0, 0x6b, 0x49, 0x6e, 0x22,
	0xc1, 0x5b, 0x97, 0x64, 0x7d, 0xcf, 0x3d, 0x57,
	0x02, 0x22, 0xbe, 0x43, 0xc9, 0x83, 0xcb, 0x61,
	0xdb, 0x57, 0xe8, 0x5f, 0x58, 0xb6, 0xf0, 0xe0,
	0xf4, 0xec, 0x8f, 0xf9, 0x74, 0xf9, 0xc6, 0xb5,
	0x36, 0x12, 0x6b, 0x92, 0xa6, 0x1d, 0xa6, 0x02,
	0xc9, 0x39, 0x75, 0xeb, 0xb6, 0x33, 0x28, 0x27,
	0x18, 0x12, 0xe6, 0x04, 0xad, 0x8d, 0x26, 0xc5,
	0xca, 0x8f, 0x37, 0x1f, 0xd5, 0xba, 0xb9, 0xbd,
	0xca, 0xe1, 0x22, 0xbd, 0xb7, 0x8d, 0x3a, 0x31,
	0x3f, 0x79, 0x9f, 0x9f, 0x1a, 0x15, 0x41, 0x81,
	0x94, 0x07, 0xe7, 0xc6, 0x0a, 0xa5, 0xa8, 0x4f,
	0x70, 0x7c, 0x73, 0x73, 0xcd, 0xcc, 0x88, 0x7b,
	0xbd, 0x0a, 0xfc, 0x26, 0xee, 0x5d, 0x39, 0x26,
	0xa4, 0x22, 0x7c, 0xa1, 0x36, 0x68, 0x55, 0xb2,
	0x8f, 0x74, 0x2b, 0xe5, 0xad, 0x3e, 0xb5, 0xbe,
	0x25, 0xf2, 0x82, 0x33, 0x9d, 0x70, 0x72, 0x2e,
	0x50, 0xcc, 0x3a, 0x0c, 0x8e, 0xcf, 0xe4, 0x20,
	0x39, 0x74, 0x4d, 0x30, 0x49, 0x6c, 0xa5, 0xf7,
	0x49, 0x9b, 0xf2, 0xa2, 0xd8, 0x98, 0x8e, 0x53,
	0x29, 0x31, 0xa4, 0xa1, 0x83, 0xe5, 0xb7, 0x16,
	0xc2, 0x68, 0x1b, 0xaf, 0xd4, 0x22, 0x7a, 0x5f,
	0x3d, 0xb0, 0x50, 0x8d, 0x93, 0x62, 0x70, 0x92,
	0x02, 0xbb, 0x7d, 0x3c, 0xca, 0xf4, 0x71, 0x4d,
	0xbc, 0x79, 0x1a, 0xfc, 0xc1, 0x6b, 0x97, 0x73,
	0x53, 0x1c, 0xdf, 0x4f, 0x01, 0x97, 0x3b, 0x24,
	0xf0, 0x15, 0xc7, 0xf7, 0x55, 0x88, 0xf6, 0xc1,
	0xfb, 0x14, 0x0b, 0xf3, 0xc3, 0x91, 0xa0, 0xec,
	0x1f, 0x0b, 0x22, 0x84, 0x96, 0x42, 0x53, 0x85,
	0x43, 0x2a, 0xc7, 0x2d, 0x56, 0x6c, 0x68, 0xae,
	0x92, 0xe3, 0xf2, 0xae, 0xcd, 0x20, 0x77, 0xc7,
	0x73, 0xe7, 0xdc, 0x07, 0x03, 0xaf, 0x5a, 0x71,
	0x91, 0x26, 0xfe, 0x7a, 0x42, 0xab, 0x2a, 0x8d,
	0xd3, 0xd2, 0x12, 0x88, 0x12, 0xe3, 0x3f, 0x3d,
	0x63, 0x5b, 0x0f, 0xf2, 0x3d, 0x69, 0x33, 0xe1,
	0xab, 0x73, 0x30, 0xb8, 0xcb, 0x8f, 0xdf, 0x1a,
	0x52, 0x0a, 0xed, 0x1d, 0x06, 0xe4, 0x5c, 0xca,
	0x42, 0x52, 0x00, 0xa0, 0x76, 0x3b, 0x02, 0x11,
	0xa4, 0xbc, 0x60, 0x03, 0xe4, 0xa4, 0x6b, 0x51,
	0xe1
};

ULONG MyHashMask(ULONG Hash)
{
	union
	{
		BYTE  ByteInfo[4];
		ULONG Info;
	};

	ByteInfo[0] = TempTable[Hash & 0xFF];
	ByteInfo[1] = TempTable[((Hash >> 3) * 4) % countof(TempTable)];
	ByteInfo[2] = TempTable[((Hash * 6) ^ 0x1542) % countof(TempTable)];
	ByteInfo[3] = TempTable[Hash % countof(TempTable)];

	return Info;
}

int wmain(int argc, WCHAR* argv[])
{
	if (argc != 2)
		return 0;
	
	ml::MlInitialize();

	FILE* file = fopen("Sena.bin", "rb");
	if (!file)
		return 0;

	FILE* fout = fopen("out.txt", "wb");
	if (!fout)
		return 0;

	fseek(file, 0, SEEK_END);
	ULONG FileSize = ftell(file);
	rewind(file);

	//MessageBox(0, 0, 0, 0);
	//__asm int 3;
	
	fseek(file, 4, SEEK_SET);
	ULONG iPos = 4;
	while (iPos < FileSize)
	{
		ULONG Length;
		ULONG Hash;
		ULONG Index;

		fread(&Length, 1, 4, file);
		fread(&Hash, 1, 4, file);
		fread(&Index, 1, 4, file);

		iPos += 12;

		Length ^= MyHashMask(Hash);
		Index ^= MyHashMask(~Hash);

		CHAR Info[500];
		RtlZeroMemory(Info, 500);
		fread(Info, 1, Length, file);
		EncodeString(Info, Length, Hash, Index);

		fprintf(fout, "[0x%08x]%08x - %d[%s]\r\n", Index, Hash, Length, Info);
		iPos += Length;
	}

	fclose(file);
	fclose(fout);
	return 0;
}
