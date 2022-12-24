#ifndef _FileSystem_
#define _FileSystem_

#include <Windows.h>
#include "Hash64.h"
#include "CMem.h"
#include "WinFile.h"
#include "Huffman.h"
#include <string>
#include <map>

using std::wstring;
using std::map;

static ULONG CompressXOR = 0x516F41AB;
static ULONG ChunkOffsetXOR = 0x91CD32AE;
static ULONG IndexCountXOR = 0x51AA62E1;
static ULONG ArchiveSizeXOR = 0xFF416722;
static ULONG RtcFileSizeXOR = 0xFD315A09;

#pragma pack(1)
typedef struct PackerHeader
{
	ULONG Magic;
	BOOL  isCompressed;
	ULONG ChunkOffset;
	ULONG IndexCount;
	ULONG ArchiveSize;
	ULONG RtcFileSize;

}PackerHeader, *PtrPackerHeader;

typedef struct PackerChunk
{
	ULONG   Magic;
	ULONG64 HashValue;
	ULONG   Offset;
	ULONG   BufferSize;
	ULONG   FileNameLength;
	wstring FileName;

}PackerChunk, *PtrPackerChunk;

typedef struct PackerChunkNo
{
	ULONG   Magic;
	ULONG64 HashValue;
	ULONG   Offset;
	ULONG   BufferSize;
	ULONG   FileNameLength;
	//wstring FileName;

}PackerChunkNo, *PtrPackerChunkNo;

typedef struct PackerChunkMap
{
	ULONG   Magic;
	ULONG   Offset;
	ULONG   BufferSize;
	ULONG   FileNameLength;
	wstring FileName;

}PackerChunkMap, *PtrPackerChunkMap;
#pragma pack()

class FileSystem
{
private:
	static FileSystem* Handle;
	FileSystem();
	~FileSystem();

	WinFile File;
	map<ULONG64, PackerChunkMap> ChunkList;

public:
	static FileSystem* WINAPI InitFileSystem();
	static FileSystem* WINAPI GetFileSystem();

	PBYTE WINAPI QueryFile(wstring& FileName, ULONG& BufferSize);
private:
	HRESULT WINAPI InitFile();
};

#endif
