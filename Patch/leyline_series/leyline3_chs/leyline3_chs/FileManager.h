#pragma once

#include "Huffman.h"
#include "WinFile.h"
#include <string>
#include <vector>
#include "lz4.h"
#include <vector>
#include <algorithm>

using std::vector;
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
	ULONG  FileOriSize;
	ULONG  FileCompSize;
	ULONG  CompMothod;
	ULONG  Offset;
	ULONG  Hash;
	ULONG  FileNameLength;
	string FileName;

	friend bool operator >(const FileChunk& lhs, const FileChunk& rhs)
	{
		return lhs.Hash > rhs.Hash;
	}

	friend bool operator >=(const FileChunk& lhs, const FileChunk& rhs)
	{
		return lhs.Hash >= rhs.Hash;
	}

	friend bool operator <(const FileChunk& lhs, const FileChunk& rhs)
	{
		return lhs.Hash < rhs.Hash;
	}

	friend bool operator <=(const FileChunk& lhs, const FileChunk& rhs)
	{
		return lhs.Hash <= rhs.Hash;
	}

	friend bool operator ==(const FileChunk& lhs, const FileChunk& rhs)
	{
		return lhs.Hash == rhs.Hash;
	}

	friend bool operator != (const FileChunk& lhs, const FileChunk& rhs)
	{
		return lhs.Hash != rhs.Hash;
	}
}FileChunk;

typedef struct FileChunkSub
{
	ULONG   FileOriSize;
	ULONG   FileCompSize;
	ULONG   CompMothod;
	ULONG   Offset;
	ULONG   Hash;
	ULONG   FileNameLength;
}FileChunkSub;
#pragma pack()


class FileManager
{
public:
	static FileManager* Handle;
	static FileManager* GetFileManager();

private:
	FileManager();

public:
	~FileManager();

	BOOL Init();
	BOOL QueryFile(LPCSTR FileName, PBYTE& Buffer, ULONG& Size);

private:
	vector<FileChunk> FileChunkPool;
	BOOL              Inited;
	WinFile           File;
};


