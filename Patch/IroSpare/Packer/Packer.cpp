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



BOOL EncodeFile(PBYTE Buffer, ULONG Size, PBYTE& OutBuffer, PBYTE KeyBuffer)
{
	for (ULONG i = 0; i < Size / 16; i++)
	{
		aes_encrypt(Buffer + i * 16, OutBuffer + i * 16, (uint*)(KeyBuffer + i * 512), 256);
	}
	return TRUE;
}

typedef struct FileInfoChunk
{
	string FileBase64Name;
	ULONG Offset;
	ULONG Size;
	ULONG KeySize;

	FileInfoChunk& operator = (const FileInfoChunk& o)
	{
		FileBase64Name = o.FileBase64Name;
		Offset = o.Offset;
		Size = o.Size;
		KeySize = o.KeySize;

		return *this;
	}
}FileInfoChunk;


typedef struct Header
{
	DWORD Offset;
}Header;


#define OffsetMark 0x52128463


BYTE EndMark[] = 
{
	0xFF, 0xFF, 0xFF, 0xAA, 0xAA, 0xAA, 0xBB, 0xBB, 0xBB
};

//
int wmain(int argc, WCHAR* argv[])
{
	if (argc != 2)
		return 0;

	srand((int)time(NULL));

	WinFile OutFile;
	WIN32_FIND_DATAW FileInfo;
	vector<wstring> FileList;
	HANDLE hFile = FindFirstFileW((wstring(argv[1]) + L"\\*.ks").c_str(), &FileInfo);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("No file....\n");
		return 0;
	}

	do
	{
		FileList.push_back(FileInfo.cFileName);
	} while (FindNextFileW(hFile, &FileInfo));
	FindClose(hFile);


	if (OutFile.Open(L"ino.bin", WinFile::FileWrite) != S_OK)
	{
		printf("Cannot write...\n");
		return 0;
	}


	vector<FileInfoChunk> ChunkList;
	ULONG iPos = sizeof(DWORD);
	OutFile.Write((PBYTE)&iPos, 4);

	for (auto& Name : FileList)
	{
		FileInfoChunk Chunk;
		CHAR UTF8FileName[1000];
		RtlZeroMemory(UTF8FileName, sizeof(UTF8FileName));
		WideCharToMultiByte(CP_UTF8, 0, Name.c_str(), Name.length(), UTF8FileName, 1000, 0, 0);

		Chunk.FileBase64Name = base64_encode((PBYTE)UTF8FileName, lstrlenA(UTF8FileName));

		wstring FullFileName(argv[1]);
		FullFileName += L"\\";
		FullFileName += Name;

		WinFile File;

		
		if (File.Open(FullFileName.c_str(), WinFile::FileRead) == S_OK)
		{
			ULONG Size = File.GetSize32();
			if (Size % 16 != 0)
			{
				wprintf(L"Invalid Size %s\n", FullFileName.c_str());
				File.Release();
				continue;
			}

			PBYTE Buffer = new BYTE[Size];
			PBYTE DecBuffer = new BYTE[Size];
			if (!Buffer || !DecBuffer)
			{
				wprintf(L"Insufficient memory %s\n", FullFileName.c_str());
				File.Release();
				continue;
			}

			File.Read(Buffer, Size);
			ULONG KeySize = (Size / 16) * 512;
			PBYTE KeyBuffer = new BYTE[KeySize];
			
			for (ULONG i = 0; i < KeySize; i++)
			{
				KeyBuffer[i] = rand();
			}

			EncodeFile(Buffer, Size, DecBuffer, KeyBuffer);

			Chunk.Offset = iPos;
			iPos += (Size + KeySize);

			OutFile.Write(DecBuffer, Size);
			OutFile.Write(KeyBuffer, KeySize);


			Chunk.Size = Size;
			Chunk.KeySize = KeySize;
			
			ChunkList.push_back(Chunk);

			delete[] DecBuffer;
			delete[] KeyBuffer;
			delete[] Buffer;
			File.Release();
		}
		else
		{
			wprintf(L"Cannot open %s\n", FullFileName.c_str());
			continue;
		}
	}

	vector<BYTE> ChunkData;
	for (auto& it : ChunkList)
	{
		for (ULONG i = 0; i < it.FileBase64Name.length(); i++)
		{
			ChunkData.push_back(it.FileBase64Name[i]);
		}
		ChunkData.push_back(NULL);
		BYTE Info[12];
		memcpy(Info, &it.Offset, 4);
		memcpy(Info + 4, &it.Size, 4);
		memcpy(Info + 8, &it.KeySize, 4);

		for (ULONG i = 0; i < 12; i++)
		{
			ChunkData.push_back(Info[i]);
		}
	}

	for (ULONG i = 0; i < sizeof(EndMark); i++)
	{
		ChunkData.push_back(EndMark[i]);
	}

	if (ChunkData.size() % 16)
	{
		ULONG Delta = 16 - (ChunkData.size() % 16);

		for (ULONG i = 0; i < Delta; i++)
		{
			ChunkData.push_back(rand());
		}
	}

	Twofish_initialise();
	Twofish_key S[1];

	Twofish_prepare_key(ChunkKey, 32, S);
	
	for (ULONG i = 0; i < ChunkData.size() / 16; i++)
	{
		BYTE Dec[16] = {0};
		Twofish_encrypt(S, &ChunkData[i * 16], Dec);
		OutFile.Write(Dec, 16);
		//OutFile.Write(&ChunkData[i * 16], 16);
	}

	printf("Offset : 0x%08x\n", iPos);
	iPos ^= OffsetMark;
	OutFile.Seek(0, FILE_BEGIN);
	OutFile.Write((PBYTE)&iPos, 4);

	OutFile.Release();
	return 0;
}


