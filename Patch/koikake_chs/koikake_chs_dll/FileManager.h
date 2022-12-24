#pragma once

#include "FastLz.h"
#include "lz4.h"
#include "my.h"
#include <string>
#include <vector>

using std::vector;
using std::string;

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

NTSTATUS NTAPI XmoeGetFile(LPWSTR FileName, HANDLE Heap, PBYTE* Buffer, PULONG Size);

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

	API_POINTER(XmoeGetFile)        StubXmoeGetFile;
private:
	vector<FileChunk> FileChunkPool;
	BOOL              Inited;
	NtFileDisk        File;
};


