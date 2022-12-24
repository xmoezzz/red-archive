#include <Windows.h>
#include <my.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include "AES.h"
#include <vector>
#include "Base64.h"
#include "twofish.h"


#pragma comment(lib, "MyLibrary_x86_static.lib")

using std::vector;
using std::wstring;
using std::string;


BYTE ChunkKey[] =
{
	0xcf, 0xe1, 0x05, 0x93, 0x44, 0xf2, 0x09, 0x59,
	0x08, 0xfc, 0xe7, 0x65, 0x3f, 0xda, 0x30, 0xed,
	0x36, 0x87, 0x0e, 0xa5, 0x11, 0x91, 0x0b, 0x83,
	0x59, 0x57, 0xf7, 0x9a, 0x5d, 0xb4, 0xd0, 0xff,
	0x51, 0x17, 0x66, 0xbb, 0x8b, 0x2a, 0xcb, 0xc5,
	0x68, 0xbe, 0x94, 0xdc, 0x6d, 0x86, 0x9a, 0x2e,
	0xe6, 0x80, 0x47, 0x08, 0x5f, 0xd2, 0xf6, 0xbd,
	0xb4, 0x7d, 0x77, 0x7b, 0x7f, 0xca, 0xcd, 0xe6,
	0x1b, 0x4a, 0xc0, 0x47, 0x8d, 0x8b, 0x80, 0xf8,
	0xb9, 0x01, 0xc5, 0x79, 0x9c, 0x12, 0xe6, 0x7d,
	0x12, 0x35, 0x6d, 0xf6, 0x60, 0xe0, 0x43, 0xd6,
	0x54, 0xe5, 0x66, 0xfc, 0x6b, 0xdf, 0x10, 0x04,
	0xad, 0xd5, 0xa9, 0xfa, 0x6d, 0x6d, 0x75, 0xa1,
	0xe9, 0x16, 0x5a, 0xde, 0xbb, 0xce, 0xce, 0xe8,
	0xfd, 0x90, 0x09, 0x26, 0x0e, 0x3c, 0xc9, 0xeb,
	0x22, 0x78, 0xca, 0xdd, 0x13, 0x77, 0x53, 0x77
};



BOOL EncodeFile(PBYTE Buffer, ULONG Size, PBYTE& OutBuffer, PBYTE KeyBuffer)
{
	for (ULONG i = 0; i < Size / 16; i++)
	{
		aes_encrypt(Buffer + i * 16, OutBuffer + i * 16, (uint*)(KeyBuffer + i * 512), 256);
	}
	return TRUE;
}


BOOL EncodeFile2(PBYTE Buffer, ULONG Size, PBYTE& OutBuffer, PBYTE KeyBuffer)
{
	for (ULONG i = 0; i < Size / 16; i++)
	{
		aes_encrypt(Buffer + i * 16, OutBuffer + i * 16, (uint*)KeyBuffer, 256);
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

	NtFileDisk OutFile;
	WIN32_FIND_DATAW FileInfo;
	vector<wstring> FileList;
	HANDLE hFile = FindFirstFileW((wstring(argv[1]) + L"\\*.*").c_str(), &FileInfo);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("No file....\n");
		return 0;
	}

	do
	{
		if (FileInfo.cFileName[0] != L'.')
			FileList.push_back(FileInfo.cFileName);
	} while (FindNextFileW(hFile, &FileInfo));
	FindClose(hFile);


	if (NT_FAILED(OutFile.Create(L"im.vm")))
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

		NtFileDisk File;

		wprintf(L"reading : %s\n", FullFileName.c_str());
		if (NT_SUCCESS(File.Open(FullFileName.c_str())))
		{
			ULONG Size = File.GetSize32();

			PBYTE Buffer = new BYTE[ROUND_UP(Size, 32)];
			PBYTE DecBuffer = new BYTE[ROUND_UP(Size, 32)];
			RtlZeroMemory(Buffer, ROUND_UP(Size, 32));
			RtlZeroMemory(DecBuffer, ROUND_UP(Size, 32));
			if (!Buffer || !DecBuffer)
			{
				wprintf(L"Insufficient memory %s\n", FullFileName.c_str());
				File.Close();
				continue;
			}

			File.Read(Buffer, Size);
			ULONG KeySize = 32;
			PBYTE KeyBuffer = new BYTE[KeySize];
			
			for (ULONG i = 0; i < KeySize; i++)
			{
				KeyBuffer[i] = rand();
			}

			//aes_encrypt(Buffer, DecBuffer, (ULONG*)KeyBuffer, 256);
			//EncodeFile2(Buffer, Size, DecBuffer, KeyBuffer);
			//RtlCopyMemory(DecBuffer, Buffer, ROUND_UP(Size, 32));

			for (ULONG i = 0; i < Size; i++)
				DecBuffer[i] = Buffer[i] ^ KeyBuffer[i % 32];

			Chunk.Offset = iPos;
			iPos += Size + KeySize;

			OutFile.Write(DecBuffer, Size);
			OutFile.Write(KeyBuffer, KeySize);


			Chunk.Size = Size;
			Chunk.KeySize = KeySize;
			
			ChunkList.push_back(Chunk);

			delete[] DecBuffer;
			delete[] KeyBuffer;
			delete[] Buffer;
			File.Close();
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
	OutFile.Seek(0);
	OutFile.Write((PBYTE)&iPos, 4);

	OutFile.Close();
	return 0;
}


