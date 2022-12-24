#include <Windows.h>
#include <WinFile.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include "AES.h"
#include <vector>
#include "Base64.h"
#include "twofish.h"

using std::vector;
using std::wstring;
using std::string;


BYTE ChunkKey[] =
{
	0xce, 0xd1, 0x08, 0xb1, 0x96, 0x8d, 0x0a,
	0x21, 0xa5, 0x52, 0x1a, 0x13, 0x95, 0x89, 0xfc,
	0x6d, 0x75, 0xb3, 0x28, 0x69, 0x46, 0xe1, 0x4a,
	0x12, 0x7f, 0x3c, 0xc2, 0xcb, 0x89, 0x7e, 0x24,
	0xc8, 0x43, 0x77, 0xb9, 0x93, 0x7f, 0x89, 0xfc,
	0x88, 0x83, 0x2c, 0x1e, 0x5a, 0x8a, 0x6b, 0x82,
	0x8c, 0x41, 0x64, 0x42, 0xf8, 0x4a, 0xcc, 0xa8,
	0x4b, 0xbb, 0x69, 0xb6, 0x87, 0xa0, 0x97, 0x9f,
	0x80, 0x75, 0xc4, 0x4b, 0x60, 0xae, 0xf4, 0xd7,
	0x23, 0x2f, 0x3d, 0x12, 0x1b, 0xd3, 0x4b, 0x01,
	0x6e, 0xe9, 0xdd, 0x5c, 0x93, 0xb3, 0x47, 0x0f,
	0xd8, 0xe6, 0xee, 0xbb, 0xdf, 0x2c, 0xcf, 0x32,
	0x1c, 0xa5, 0xf9, 0xfe, 0x58, 0x61, 0x0e, 0xdb,
	0x31, 0xe8, 0xc5, 0x38, 0x98, 0xb2, 0x6b, 0xba,
	0x52, 0xaf, 0x5e, 0xb8, 0x78, 0xc1, 0x90, 0xc0,
	0xf8, 0x3d, 0x0a, 0x11, 0x11, 0x6e, 0x67, 0x1f,
	0xda
};



BOOL DecodeFile(PBYTE Buffer, ULONG Size, PBYTE& OutBuffer, PBYTE KeyBuffer)
{
	for (ULONG i = 0; i < Size / 16; i++)
	{
		aes_decrypt(Buffer + i * 16, OutBuffer + i * 16, (uint*)(KeyBuffer + i * 512), 256);
	}
	return TRUE;
}


typedef struct FileInfoChunk
{
	wstring FileName;
	ULONG Offset;
	ULONG Size;
	ULONG KeySize;

	FileInfoChunk& operator = (const FileInfoChunk& o)
	{
		FileName = o.FileName;
		Offset = o.Offset;
		Size = o.Size;
		KeySize = o.KeySize;

		return *this;
	}
}FileInfoChunk;


#define OffsetMark 0x52128463


BYTE EndMark[] =
{
	0xFF, 0xFF, 0xFF, 0xAA, 0xAA, 0xAA, 0xBB, 0xBB, 0xBB
};


int wmain(int argc, WCHAR* argv[])
{
	if (argc != 2)
		return 0;

	WinFile File;
	DWORD   ChunkOffset;
	DWORD   FileSize;
	if (File.Open(argv[1], WinFile::FileRead) != S_OK)
	{
		printf("cannot open\n");
		return 0;
	}

	File.Read((PBYTE)&ChunkOffset, 4);
	ChunkOffset ^= OffsetMark;

	FileSize = File.GetSize32();
	if (ChunkOffset >= FileSize)
	{
		printf("Invalid Offset\n");
		Sleep(3000);
	}
	
	File.Seek(ChunkOffset, FILE_BEGIN);
	ULONG ChunkSize = FileSize - ChunkOffset;

	PBYTE ChunkData = new BYTE[ChunkSize];
	File.Read(ChunkData, ChunkSize);

	Twofish_initialise();
	Twofish_key S[1];

	Twofish_prepare_key(ChunkKey, 32, S);

	for (ULONG i = 0; i < ChunkSize / 16; i++)
	{
		BYTE Dec[16] = { 0 };
		Twofish_decrypt(S, &ChunkData[i * 16], Dec);
		memcpy(&ChunkData[i * 16], Dec, 16);
	}

	vector<FileInfoChunk> FileList;

	ULONG iPos = 0;
	while (iPos < ChunkSize)
	{
		string UTF8Str = (LPCSTR)(ChunkData + iPos);
		iPos += UTF8Str.length() + 1;

		UTF8Str = base64_decode(UTF8Str);

		WCHAR FileName[400] = { 0 };
		FileInfoChunk Info;

		MultiByteToWideChar(CP_UTF8, 0, UTF8Str.c_str(), UTF8Str.length(), FileName, 400);

		Info.FileName = FileName;
		
		Info.Offset = *(PDWORD)(ChunkData + iPos);
		iPos += 4;
		Info.Size = *(PDWORD)(ChunkData + iPos);
		iPos += 4;
		Info.KeySize = *(PDWORD)(ChunkData + iPos);
		iPos += 4;

		FileList.push_back(Info);

		if (!memcmp((ChunkData + iPos), EndMark, sizeof(EndMark)))
			break;
	}

	delete[] ChunkData;

	for (auto& it : FileList)
	{
		File.Seek(it.Offset, FILE_BEGIN);

		WinFile OutFile;
		if (OutFile.Open(it.FileName.c_str(), WinFile::FileWrite) == S_OK)
		{
			ULONG ChunkSize = it.Size + it.KeySize;
			PBYTE  PreBuffer = new BYTE[ChunkSize];
			PBYTE  Buffer = new BYTE[it.Size];
			File.Read(PreBuffer, ChunkSize);

			DecodeFile(PreBuffer, it.Size, Buffer, PreBuffer + it.Size);

			OutFile.Write(Buffer, it.Size);

			delete[] Buffer;
			delete[] PreBuffer;
			OutFile.Release();
		}
		else
		{
			wprintf(L"Cannot write : %s\n", it.FileName.c_str());
		}
	}


	File.Release();
	return 0;
}

