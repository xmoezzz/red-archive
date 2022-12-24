#include "stdafx.h"
#include <Windows.h>
#include "CMem.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>
#include "WinFile.h"
#include <set>
#include "Hash64.h"
#include "Huffman.h"

using std::string;
using std::wstring;
using std::vector;
using std::fstream;
using std::set;

//Exe info Path

/******************************/
//Header Info
//DataBuffer
//IndexBuffer
/******************************/

static ULONG HeaderMagic = (ULONG)"EOMX";
static ULONG ChunkMagic = (ULONG)"eliF";

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
#pragma pack()

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 3)
	{
		return 0;
	}
	wstring PackerPath(argv[2]);
	char szFileName[MAX_PATH] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, argv[1], lstrlenW(argv[1]), szFileName, MAX_PATH, nullptr, nullptr);

	fstream fin(szFileName);
	if (!fin)
	{
		return 0;
	}

	vector<string> FileList;
	string ReadLine;
	while (getline(fin, ReadLine))
	{
		if (ReadLine.length())
		{
			FileList.push_back(ReadLine);
		}
	}
	fin.close();
	
	PackerHeader Header;
	Header.Magic = HeaderMagic;
	Header.isCompressed = TRUE;
	Header.IndexCount = FileList.size();
	
	WinFile FileOut;
	if (FileOut.Open(L"sabbat.xmoe", WinFile::FileWrite) != S_OK)
	{
		MessageBoxW(NULL, L"Couldn't Write output file", L"SWPacker", MB_OK);
		return 0;
	}
	FileOut.Write((PBYTE)&Header, sizeof(PackerHeader));

	vector<PackerChunk> ChunkList;
	vector<ULONG64> Hash64BufferPool;

	ULONG Offset = sizeof(PackerHeader);
	ULONG ByteTransfered = 0;
	for (auto it : FileList)
	{
		WCHAR WideName[MAX_PATH] = { 0 };
		MultiByteToWideChar(CP_UTF8, 0, it.c_str(), it.length(), WideName, MAX_PATH);
		wstring FullPathName = PackerPath;
		FullPathName += L"\\";
		FullPathName += WideName;

		WinFile File;
		if (File.Open(FullPathName.c_str(), WinFile::FileRead) != S_OK)
		{
			MessageBoxW(NULL, (wstring(L"Failed to open : \n") + FullPathName).c_str(), L"Error", MB_OK);
			return 0;
		}
		PackerChunk PackerChunkInfo;
		PackerChunkInfo.Magic = ChunkMagic;
		PackerChunkInfo.FileName = WideName;
		PackerChunkInfo.FileNameLength = lstrlenW(WideName);
		PackerChunkInfo.BufferSize = File.GetSize32();
		PackerChunkInfo.Offset = Offset;
		Offset += File.GetSize32();

		ULONG64 Hash64Value = Hash64(PackerChunkInfo.FileName.c_str(), PackerChunkInfo.FileName.length() * 2);

		BOOL HasSame = FALSE;
		for (auto itr : Hash64BufferPool)
		{
			if (itr == Hash64Value)
			{
				HasSame = TRUE;
				break;
			}
		}
		if (HasSame)
		{
			MessageBoxW(NULL, L"Has the same value", L"Error", MB_OK);
			return 0;
		}

		PBYTE Buffer = (PBYTE)CMem::Alloc(File.GetSize32());
		File.Read(Buffer, File.GetSize32());
		FileOut.Write(Buffer, File.GetSize32());
		PackerChunkInfo.HashValue = Hash64Value;
		
		ChunkList.push_back(PackerChunkInfo);
	}

	for (auto it : ChunkList)
	{
		ByteTransfered += sizeof(it.BufferSize);
		ByteTransfered += sizeof(it.FileNameLength);
		ByteTransfered += sizeof(it.HashValue);
		ByteTransfered += sizeof(it.Magic);
		ByteTransfered += sizeof(it.Offset);
		ByteTransfered += (it.FileName.length() + 1) * 2;
	}

	PBYTE PrecompressIndex = (PBYTE)CMem::Alloc(ByteTransfered);
	PBYTE CompressedIndex = (PBYTE)CMem::Alloc(ByteTransfered * 2);
	ULONG iPos = 0;

	for (auto it : ChunkList)
	{
		RtlCopyMemory((PrecompressIndex + iPos), &(it.Magic), sizeof(it.Magic));
		iPos += sizeof(it.Magic);
		RtlCopyMemory((PrecompressIndex + iPos), &(it.HashValue), sizeof(it.HashValue));
		iPos += sizeof(it.HashValue);
		RtlCopyMemory((PrecompressIndex + iPos), &(it.Offset), sizeof(it.Offset));
		iPos += sizeof(it.Offset);
		RtlCopyMemory((PrecompressIndex + iPos), &(it.BufferSize), sizeof(it.BufferSize));
		iPos += sizeof(it.BufferSize);
		RtlCopyMemory((PrecompressIndex + iPos), &(it.FileNameLength), sizeof(it.FileNameLength));
		iPos += sizeof(it.FileNameLength);
		RtlCopyMemory((PrecompressIndex + iPos), (it.FileName.c_str()), (it.FileName.length() + 1) * 2);
		iPos += (it.FileName.length() + 1) * 2;
	}

	CHuffman Huffman;
	long CompressSize = 0;
	Huffman.Encode(CompressedIndex, CompressSize, PrecompressIndex, ByteTransfered);
	FileOut.Write(CompressedIndex, CompressSize);

	Header.RtcFileSize = ByteTransfered;
	Header.ArchiveSize = CompressSize;
	Header.isCompressed = TRUE;
	Header.ChunkOffset = Offset;

	Header.ArchiveSize ^= ArchiveSizeXOR;
	Header.ArchiveSize ^= ChunkOffsetXOR;
	Header.IndexCount ^= IndexCountXOR;
	Header.isCompressed ^= CompressXOR;
	Header.RtcFileSize ^= RtcFileSizeXOR;

	FileOut.Seek(0, FILE_BEGIN);
	FileOut.Write((PBYTE)&Header, sizeof(PackerHeader));

	FileOut.Release();
	return 0;
}

