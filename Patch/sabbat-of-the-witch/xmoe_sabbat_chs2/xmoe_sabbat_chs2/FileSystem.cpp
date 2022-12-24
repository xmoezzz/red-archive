#include "FileSystem.h"

FileSystem* FileSystem::Handle = nullptr;

VOID WINAPI OnError(const WCHAR* lpString)
{
	MessageBoxW(NULL, lpString, L"X'moe CoreLib", MB_OK);
	ExitProcess(-1);
}

FileSystem* WINAPI FileSystem::InitFileSystem()
{
	if (Handle == nullptr)
	{
		Handle = new FileSystem;
		if (Handle == nullptr)
		{
			MessageBoxW(NULL, L"系统初始化失败", L"X'moe CoreLib", MB_OK);
			ExitProcess(-1);
		}
	}
	Handle->InitFile();
	return Handle;
}

FileSystem* WINAPI FileSystem::GetFileSystem()
{
	if (Handle == nullptr)
	{
		MessageBoxW(NULL, L"系统未被初始化", L"X'moe CoreLib", MB_OK);
		ExitProcess(-1);
	}
	return Handle;
}


FileSystem::FileSystem()
{
	ChunkList.clear();
}

FileSystem::~FileSystem()
{
	File.Release();
	ChunkList.clear();
}


HRESULT WINAPI FileSystem::InitFile()
{
	//ZeroMemory(&File, sizeof(WinFile));
	//File.Drop();
	if (File.Open(L"sabbat.xmoe", WinFile::FileRead) != S_OK)
	{
		MessageBoxW(NULL, L"无法打开汉化文件", L"X'moe CoreLib", MB_OK);
		ExitProcess(-1);
	}
	ChunkList.clear();
	
	PackerHeader Header;
	File.Read((PBYTE)&Header, sizeof(PackerHeader));

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
		File.Seek(Header.ChunkOffset, FILE_BEGIN);
		File.Read(CompressBuffer, CompressSize);
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
		OnError(L"Bad Data");
	}

	
	ULONG iPos = 0;
	while (iPos < IndexSize)
	{
		PackerChunkNo* SubChunk = (PackerChunkNo*)(IndexBuffer + iPos);
		iPos += sizeof(PackerChunkNo);
		wstring FileName((WCHAR*)(IndexBuffer + iPos));
		iPos += (FileName.length() + 1) * 2;
		ULONG64 HashValue64 = SubChunk->HashValue;

		PackerChunkMap Chunk;
		Chunk.Magic = SubChunk->Magic;
		Chunk.Offset = SubChunk->Offset;
		Chunk.FileNameLength = SubChunk->FileNameLength;
		Chunk.BufferSize = SubChunk->BufferSize;
		Chunk.FileName = FileName;

		ChunkList.insert(std::make_pair(HashValue64, Chunk));
	}

	CMem::Free(IndexBuffer);
	return S_OK;
}


PBYTE WINAPI FileSystem::QueryFile(wstring& FileName, ULONG& BufferSize)
{
	PBYTE RawBuffer = nullptr;
	BufferSize = 0;
	
	ULONG64 HashValue = Hash64(FileName.c_str(), FileName.length() * 2);
	auto it = ChunkList.find(HashValue);
	if (it != ChunkList.end())
	{
		if (!wcscmp(it->second.FileName.c_str(), FileName.c_str()))
		{
			BufferSize = it->second.BufferSize;
			RawBuffer = (PBYTE)CMem::Alloc(BufferSize);
			if (RawBuffer == nullptr)
			{
				BufferSize = 0;
				return nullptr;
			}
			File.Seek(it->second.Offset, FILE_BEGIN);
			File.Read(RawBuffer, BufferSize);
			return RawBuffer;
		}
	}
	return nullptr;
}
