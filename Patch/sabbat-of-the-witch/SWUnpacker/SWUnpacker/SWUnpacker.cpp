#include "stdafx.h"
#include "CMem.h"
#include "WinFile.h"
#include "Huffman.h"
#include <string>
#include <vector>

using std::string;
using std::wstring;
using std::vector;


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
#pragma pack()


VOID WINAPI OnError(const WCHAR* lpString)
{
	MessageBoxW(NULL, lpString, L"Error", MB_OK);
	ExitProcess(-1);
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2)
	{
		return 0;
	}
	WinFile InputFile;
	if (InputFile.Open(argv[1], WinFile::FileRead))
	{
		OnError(L"不能打开输入文件");
		return 0;
	}

	PackerHeader Header;
	InputFile.Read((PBYTE)&Header, sizeof(PackerHeader));

	Header.ArchiveSize ^= ArchiveSizeXOR;
	Header.ArchiveSize ^= ChunkOffsetXOR;
	Header.IndexCount ^= IndexCountXOR;
	Header.isCompressed ^= CompressXOR;
	Header.RtcFileSize ^= RtcFileSizeXOR;

	PBYTE IndexBuffer = (PBYTE)CMem::Alloc(Header.RtcFileSize);
	ULONG IndexSize = Header.RtcFileSize;
	if (Header.isCompressed)
	{
		ULONG CompressSize = Header.ArchiveSize;
		PBYTE CompressBuffer = (PBYTE)CMem::Alloc(CompressSize);
		InputFile.Seek(Header.ChunkOffset, FILE_BEGIN);
		InputFile.Read(CompressBuffer, CompressSize);
		if (CompressBuffer == nullptr)
		{
			OnError(L"Bad Refer");
		}
		CHuffman Huffman;
		long DecodeSize = 0;
		Huffman.Decode(IndexBuffer, DecodeSize, CompressBuffer, CompressSize);
	}
	else
	{
		OnError(L"Index Compression Error");
	}

	vector<PackerChunk> ChunkList;
	ULONG iPos = 0;
	while (iPos < IndexSize)
	{
		PackerChunkNo* SubChunk = (PackerChunkNo*)(IndexBuffer + iPos);
		iPos += sizeof(PackerChunkNo);
		wstring FileName((WCHAR*)(IndexBuffer + iPos));
		iPos += (FileName.length() + 1) * 2;

		PackerChunk Chunk;
		Chunk.Magic = SubChunk->Magic;
		Chunk.Offset = SubChunk->Offset;
		Chunk.FileNameLength = SubChunk->FileNameLength;
		Chunk.HashValue = SubChunk->HashValue;
		Chunk.BufferSize = SubChunk->BufferSize;
		Chunk.FileName = FileName;

		ChunkList.push_back(Chunk);
	}

	for (auto it : ChunkList)
	{
		InputFile.Seek(it.Offset, FILE_BEGIN);
		PBYTE TempBuffer = (PBYTE)CMem::Alloc(it.BufferSize);
		if (!TempBuffer)
		{
			OnError(L"分配内存失败");
		}
		
		InputFile.Read(TempBuffer, it.BufferSize);

		WinFile File;
		if (File.Open(it.FileName.c_str(), WinFile::FileWrite) != S_OK)
		{
			OnError(L"写入失败");
		}

		File.Write(TempBuffer, it.BufferSize);
		File.Release();
		CMem::Free(TempBuffer);
	}

	InputFile.Release();
	CMem::Free(IndexBuffer);
	return 0;
}

