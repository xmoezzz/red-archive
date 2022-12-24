#include "stdafx.h"
#include "ChunkDecodeKey.h"
#include "FileKey.h"
#include "HeaderDecodeKey.h"
#include "WinFile.h"
#include "Huffman.h"
#include <string>
#include <vector>
#include <Windows.h>

using std::string;
using std::wstring;
using std::vector;

#pragma pack(1)
typedef struct FileHeader
{
	ULONG Magic; //'EOMX'
	ULONG FileCount;
	ULONG ChunkOffset;
	ULONG ChunkOriSize;
	ULONG ChunkCompSize;
	ULONG CompMethod;
}FileHeader;

typedef struct FileChunk
{
	ULONG FileOriSize;
	ULONG FileCompSize;
	ULONG CompMothod;
	ULONG Offset;
	ULONG Hash;
	ULONG FileNameLength;
	string FileName;
}FileChunk;

typedef struct FileChunkSub
{
	ULONG FileOriSize;
	ULONG FileCompSize;
	ULONG CompMothod;
	ULONG Offset;
	ULONG Hash;
	ULONG FileNameLength;
}FileChunkSub;
#pragma pack()


void DecodeHeader(PBYTE FileBuffer, ULONG Size)
{
	for (ULONG i = 0; i < Size; i++)
	{
		FileBuffer[i] ^= HeaderDecodeKey[i % 256];
	}
}

void DecodeChunk(PBYTE FileBuffer, ULONG Size)
{
	for (ULONG i = 0; i < Size; i++)
	{
		FileBuffer[i] ^= ChunkDecodeKey[i % 256];
	}
}

void DecodeFile(PBYTE FileBuffer, ULONG Size)
{
	for (ULONG i = 0; i < Size; i++)
	{
		FileBuffer[i] ^= FileDecodeKey[i % 1024];
	}
}


vector<FileChunk> FileChunkPool;

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2)
	{
		return -1;
	}
	WinFile File;
	if (File.Open(argv[1], WinFile::FileRead) != S_OK)
	{
		MessageBoxW(NULL, L"无法打开输入文件", L"Error", MB_OK);
		ExitProcess(-1);
	}
	FileHeader Header = { 0 };
	File.Read((PBYTE)(&Header), sizeof(FileHeader));

	DecodeHeader((PBYTE)(&Header), sizeof(FileHeader));

	File.Seek(Header.ChunkOffset, FILE_BEGIN);
	PBYTE ChunkBufferComp = (PBYTE)GlobalAlloc(0, Header.ChunkCompSize);
	PBYTE ChunkBuffer = (PBYTE)GlobalAlloc(0, Header.ChunkOriSize);

	if (!ChunkBuffer || !ChunkBufferComp)
	{
		MessageBoxW(NULL, L"不能为Chunk分配内存", L"Error", MB_OK);
		ExitProcess(-1);
	}

	CHuffman HuffChunk;
	long UncompLength = 0;

	File.Seek(Header.ChunkOffset, FILE_BEGIN);
	File.Read(ChunkBufferComp, Header.ChunkCompSize);
	DecodeChunk(ChunkBufferComp, Header.ChunkCompSize);

	printf("解压Chunk, Offset = %08x\n", Header.ChunkOffset);
	HuffChunk.Decode(ChunkBuffer, UncompLength, ChunkBufferComp, Header.ChunkCompSize);
	if ((ULONG)UncompLength != Header.ChunkOriSize)
	{
		MessageBoxW(NULL, L"Chunk解压失败", L"Error", MB_OK);
		ExitProcess(-1);
	}

	ULONG iPos = 0;
	for (ULONG i = 0; i < Header.FileCount; i++)
	{
		FileChunkSub ChunkInfoSub = { 0 };
		memcpy((PBYTE)(&ChunkInfoSub), (ChunkBuffer + iPos), sizeof(FileChunkSub));
		iPos += sizeof(FileChunkSub);

		string FileName((CHAR*)(ChunkBuffer + iPos));
		iPos += ChunkInfoSub.FileNameLength + 1;

		printf("Found File: %s, ArcLen = %08x, OriLen = %08x,\nNameLen = %08x, iPos = %08x\n\n", FileName.c_str(), 
			ChunkInfoSub.FileCompSize, ChunkInfoSub.FileOriSize, ChunkInfoSub.FileNameLength, iPos);
		
		FileChunk ChunkInfo;
		ChunkInfo.CompMothod = ChunkInfoSub.CompMothod;
		ChunkInfo.FileCompSize = ChunkInfoSub.FileCompSize;
		ChunkInfo.FileOriSize = ChunkInfoSub.FileOriSize;
		ChunkInfo.Hash = ChunkInfoSub.Hash;
		ChunkInfo.FileNameLength = ChunkInfoSub.FileNameLength; 
		ChunkInfo.Offset = ChunkInfoSub.Offset;
		ChunkInfo.FileName = FileName;

		FileChunkPool.push_back(ChunkInfo);
	}

	for (auto it : FileChunkPool)
	{
		printf("File : %s\n", it.FileName.c_str());
	}

	for (auto it : FileChunkPool)
	{
		WinFile OutFile;
		PBYTE CompFileBuffer = (PBYTE)GlobalAlloc(0, it.FileCompSize);
		PBYTE FileBuffer = (PBYTE)GlobalAlloc(0, it.FileOriSize);

		File.Seek(it.Offset, FILE_BEGIN);
		File.Read(CompFileBuffer, it.FileCompSize);

		DecodeFile(CompFileBuffer, it.FileCompSize);

		long OriSize = 0;
		CHuffman FileHuff;
		FileHuff.Decode(FileBuffer, OriSize, CompFileBuffer, it.FileCompSize);
		if (OriSize != it.FileOriSize)
		{
			MessageBoxW(NULL, L"文件解压失败", L"Error", MB_OK);
			ExitProcess(-1);
		}
		
		printf("Extracting : %s\n", it.FileName.c_str());
		WCHAR WideFileName[260] = { 0 };
		MultiByteToWideChar(CP_ACP, 0, it.FileName.c_str(), it.FileName.length(), WideFileName, 260);

		if (OutFile.Open(WideFileName, WinFile::FileWrite) != S_OK)
		{
			MessageBoxW(NULL, L"无法打开写入文件", L"Error", MB_OK);
			ExitProcess(-1);
		}

		OutFile.Write(FileBuffer, it.FileOriSize);
		OutFile.Release();

		GlobalFree(CompFileBuffer);
		GlobalFree(FileBuffer);
	}


	GlobalFree(ChunkBufferComp);
	GlobalFree(ChunkBuffer);
	File.Release();
	return 0;
}

