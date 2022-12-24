#include "stdafx.h"
#include <string>
#include <vector>
#include <fstream>
#include <Windows.h>
#include "WinFile.h"
#include "FileKey.h"
#include "ChunkDecodeKey.h"
#include "HeaderDecodeKey.h"
#include "Huffman.h"

using std::string;
using std::wstring;
using std::vector;
using std::fstream;

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
	LONG Offset;
	ULONG Hash;
	ULONG FileNameLength;
	string FileName;
}FileChunk;
#pragma pack()


ULONG MakeHash(const char* key)
{
	ULONG h = 0;
	ULONG g;
	while (*key)
	{
		h = (h << 4) + *key++;
		g = h & 0xF0000000;
		if (g)
		{
			h ^= g >> 24;
		}
		h &= ~g;
	}
	return h;
}

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

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 3)
	{
		return -1;
	}
	CHAR MbsFileListName[260] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, argv[1], lstrlenW(argv[1]), MbsFileListName, 260, nullptr, nullptr);

	fstream fin(MbsFileListName);
	if (!fin)
	{
		MessageBoxW(NULL, L"无法打开文件列表", L"Error", MB_OK);
		return -1;
	}

	wstring FilePath(argv[2]);
	vector<wstring> FileNamePool;
	vector<FileChunk> FileChunkPool;

	string ReadLine;
	while (getline(fin, ReadLine))
	{
		if (ReadLine.length())
		{
			WCHAR WideName[260] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, ReadLine.c_str(), ReadLine.length(), WideName, 260);
			FileNamePool.push_back(WideName);
			ReadLine.clear();
		}
	}
	fin.close();

	WinFile File;
	if (File.Open(L"Koi.data", WinFile::FileWrite) != S_OK)
	{
		MessageBoxW(NULL, L"无法打开写入文件", L"Error", MB_OK);
		return -1;
	}

	ULONG Offset = sizeof(FileHeader);
	FileHeader Header;
	Header.Magic = (DWORD)'EOMX';
	Header.FileCount = FileNamePool.size();

	File.Write((PBYTE)(&Header), sizeof(Header));

	for (auto it : FileNamePool)
	{
		wstring FullName = FilePath;
		FullName += L"\\";
		FullName += it;

		WinFile InFile;
		if (InFile.Open(FullName.c_str(), WinFile::FileRead) == S_OK)
		{
			PBYTE FileBytes = (PBYTE)GlobalAlloc(0, InFile.GetSize32());
			PBYTE FileComp = (PBYTE)GlobalAlloc(0, InFile.GetSize32() * 2);
			if (!FileBytes || !FileComp)
			{
				MessageBoxW(NULL, it.c_str(), L"内存分配失败", MB_OK);
				return -1;
			}
			InFile.Read(FileBytes, InFile.GetSize32());
			CHuffman HaffComp;
			long CompLength = 0;
			HaffComp.Encode(FileComp, CompLength, FileBytes, InFile.GetSize32());

			DecodeFile(FileComp, CompLength);
			File.Write(FileComp, CompLength);

			GlobalFree(FileComp);
			GlobalFree(FileBytes);

			FileChunk Chunk;
			Chunk.CompMothod = 1;
			Chunk.FileOriSize = InFile.GetSize32();
			Chunk.FileCompSize = CompLength;
			Chunk.Offset = Offset;
			CHAR MbsName[260] = { 0 };
			WideCharToMultiByte(CP_ACP, 0, it.c_str(), it.length(), MbsName, 260, nullptr, nullptr);
			Chunk.FileName = MbsName;
			Chunk.FileNameLength = Chunk.FileName.length();

			FileChunkPool.push_back(Chunk);
			InFile.Release();
			Offset += CompLength;
		}
		else
		{
			MessageBoxW(NULL, FullName.c_str(), L"读取失败", MB_OK);
			return -1;
		}
	}

	ULONG ChunkSize = 0;
	for (auto it : FileChunkPool)
	{
		ChunkSize += sizeof(it.CompMothod);
		ChunkSize += sizeof(it.FileCompSize);
		ChunkSize += sizeof(it.FileOriSize);
		ChunkSize += sizeof(it.Offset);
		ChunkSize += sizeof(it.FileNameLength);
		ChunkSize += sizeof(it.Hash);
		ChunkSize += it.FileName.length() + 1;
	}

	PBYTE ChunkBuffer = (PBYTE)GlobalAlloc(0, ChunkSize);
	PBYTE ChunkComp = (PBYTE)GlobalAlloc(0, ChunkSize * 2);
	if (!ChunkBuffer || !ChunkComp)
	{
		MessageBoxW(NULL, L"为Chunk分配内存失败", L"Error", MB_OK);
		ExitProcess(-1);
	}

	ULONG iPos = 0;
	for (auto it : FileChunkPool)
	{
		memcpy((ChunkBuffer + iPos), &(it.FileOriSize), sizeof(ULONG));
		iPos += 4;
		memcpy((ChunkBuffer + iPos), &(it.FileCompSize), sizeof(ULONG));
		iPos += sizeof(ULONG);
		memcpy((ChunkBuffer + iPos), &(it.CompMothod), sizeof(ULONG));
		iPos += sizeof(ULONG);
		memcpy((ChunkBuffer + iPos), &(it.Offset), sizeof(ULONG));
		iPos += sizeof(ULONG);
		ULONG Hash = MakeHash(it.FileName.c_str());
		memcpy((ChunkBuffer + iPos), &(Hash), sizeof(ULONG));
		iPos += sizeof(ULONG);
		memcpy((ChunkBuffer + iPos), &(it.FileNameLength), sizeof(ULONG));
		iPos += sizeof(ULONG);
		memcpy((ChunkBuffer + iPos), it.FileName.c_str(), it.FileName.length() + 1);
		iPos += it.FileName.length() + 1;
	}

	CHuffman HuffDecode;
	long ChunkCompSize = 0;
	HuffDecode.Encode(ChunkComp, ChunkCompSize, ChunkBuffer, ChunkSize);
	DecodeChunk(ChunkComp, ChunkCompSize);

#if 0
	FILE* out = fopen("index", "wb");
	fwrite(ChunkBuffer, 1, ChunkSize, out);
	fclose(out);
#endif

	File.Write(ChunkComp, ChunkCompSize);
	File.Seek(0, FILE_BEGIN);
	Header.CompMethod = 1;
	Header.ChunkOffset = Offset;
	Header.ChunkCompSize = ChunkCompSize;
	Header.ChunkOriSize = ChunkSize;

	DecodeHeader((PBYTE)(&Header), sizeof(FileHeader));

	File.Write((PBYTE)(&Header), sizeof(FileHeader));

	GlobalFree(ChunkBuffer);
	GlobalFree(ChunkComp);
	File.Release();

	printf("Chunk Offset = %08x\n", Offset);
	return 0;
}

