#ifndef _FileManager_
#define _FileManager_

#include "Huffman.h"
#include "WinFile.h"

#include <vector>
#include <string>
#include <algorithm>
#include "lz4.h"

#pragma comment(lib, "LZ4.lib")

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
	ULONG FileOriSize;
	ULONG FileCompSize;
	ULONG CompMothod;
	ULONG Offset;
	ULONG Hash;
	ULONG FileNameLength;
}FileChunkSub;
#pragma pack()


class FileManager
{
public:
	static FileManager* Handle;
	static FileManager* Create();
	static FileManager* GetFileManager();

private:
	FileManager();

public:
	~FileManager();

	bool Init();
	bool QueryFile(const char* FileName, PBYTE& Buffer, ULONG& Size);

private:
	vector<FileChunk> FileChunkPool;
	bool Inited;
	WinFile File;
};

#endif
