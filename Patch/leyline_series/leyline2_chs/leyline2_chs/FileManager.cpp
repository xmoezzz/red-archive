#include "FileManager.h"
#include "lz4.h"
#include "my.h"
#include <algorithm>

FileManager* FileManager::Handle = NULL;

#include "FileKey.h"
#include "HeaderDecodeKey.h"
#include "ChunkDecodeKey.h"


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


void Tolower(String& str)
{
	str = str.ToLower();
}

FileManager::FileManager() :
Inited(FALSE)
{

}

FileManager::~FileManager()
{
	FileChunkPool.clear();
	File.Release();
}


FileManager* FileManager::GetFileManager()
{
	if (Handle == NULL)
	{
		Handle = new FileManager;
		if (Handle == NULL)
		{
			MessageBoxW(NULL, L"没有足够的空间运行文件系统", L"X'moe-CoreLib", MB_OK);
			Ps::ExitProcess(-1);
		}
	}
	return Handle;
}

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

BOOL FileManager::Init()
{
	if (File.Open(L"Leyline.bin", WinFile::FileRead) != S_OK)
	{
		//MessageBoxW(NULL, L"找不到虚拟文件系统文件", L"X'moe-CoreLib", MB_OK);
		//Ps::ExitProcess(-1);
		Inited = FALSE;
	}
	else
	{
		FileHeader Header;
		File.Read((PBYTE)(&Header), sizeof(FileHeader));

		DecodeHeader((PBYTE)(&Header), sizeof(FileHeader));

		File.Seek(Header.ChunkOffset, FILE_BEGIN);
		PBYTE ChunkBufferComp = (PBYTE)HeapAlloc(GetProcessHeap(), 0, Header.ChunkCompSize);
		PBYTE ChunkBuffer     = (PBYTE)HeapAlloc(GetProcessHeap(), 0, Header.ChunkOriSize);

		if (!ChunkBuffer || !ChunkBufferComp)
		{
			MessageBoxW(NULL, L"内存不足", L"X'moe-CoreLib", MB_OK);
			Ps::ExitProcess(-1);
			return FALSE;
		}

		CHuffman HuffChunk;
		LONG UncompLength = 0;

		File.Seek(Header.ChunkOffset, FILE_BEGIN);
		File.Read(ChunkBufferComp, Header.ChunkCompSize);
		DecodeChunk(ChunkBufferComp, Header.ChunkCompSize);

		HuffChunk.Decode(ChunkBuffer, UncompLength, ChunkBufferComp, Header.ChunkCompSize);
		if ((ULONG)UncompLength != Header.ChunkOriSize)
		{
			MessageBoxW(NULL, L"文件错误", L"X'moe-CoreLib", MB_OK);
			Ps::ExitProcess(-1);
		}

		ULONG iPos = 0;
		for (ULONG i = 0; i < Header.FileCount; i++)
		{
			FileChunkSub ChunkInfoSub;
			RtlCopyMemory((PBYTE)(&ChunkInfoSub), (ChunkBuffer + iPos), sizeof(FileChunkSub));
			iPos += sizeof(FileChunkSub);

			String FileName((CHAR*)(ChunkBuffer + iPos));
			iPos += ChunkInfoSub.FileNameLength + 1;

			String FileNameLowerCase = FileName;
			Tolower(FileNameLowerCase);

			FileChunk ChunkInfo;
			ChunkInfo.CompMothod = ChunkInfoSub.CompMothod;
			ChunkInfo.FileCompSize = ChunkInfoSub.FileCompSize;
			ChunkInfo.FileOriSize = ChunkInfoSub.FileOriSize;
			ChunkInfo.Hash = /*ChunkInfoSub.Hash;*/ MakeHash(FileNameLowerCase.CString());
			ChunkInfo.FileNameLength = ChunkInfoSub.FileNameLength;
			ChunkInfo.Offset = ChunkInfoSub.Offset;
			ChunkInfo.FileName = FileName;

			FileChunkPool.push_back(ChunkInfo);
		}

		std::sort(FileChunkPool.begin(), FileChunkPool.end());
		HeapFree(GetProcessHeap(), 0, ChunkBufferComp);
		HeapFree(GetProcessHeap(), 0, ChunkBuffer);
		
		Inited = TRUE;
	}
	return TRUE;
}


ULONG myLowerBound(Vector<FileChunk> &data, ULONG Hash)
{
	ULONG start = 0;
	ULONG last = data.Size();
	while (start < last)
	{
		ULONG mid = (start + last) / 2;
		if (data[mid].Hash >= Hash)
		{
			last = mid;
		}
		else
		{
			start = mid + 1;
		}
	}
	return start;
}

ULONG UpperBound(Vector<FileChunk> &data, ULONG Hash)
{
	ULONG start = 0;
	ULONG last = data.Size();
	while (start < last)
	{
		ULONG mid = (start + last) / 2;
		if (data[mid].Hash <= Hash)
		{
			start = mid + 1;
		}
		else
		{
			last = mid;
		}
	}
	return start;
}

BOOL FileManager::QueryFile(LPCSTR FileName, PBYTE& Buffer, ULONG& Size)
{
	WCHAR WideName[MAX_PATH], FileNameFull[MAX_PATH];
	RtlZeroMemory(WideName,     MAX_PATH * sizeof(WCHAR));
	RtlZeroMemory(FileNameFull, MAX_PATH * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, FileName, StrLengthA(FileName), WideName, MAX_PATH);
	
	lstrcatW(FileNameFull, L"ProjectDir\\");
	lstrcatW(FileNameFull, WideName);
	
	Size = 0;
	Buffer = NULL;

	WinFile DirectFile;
	if (DirectFile.Open(FileNameFull, WinFile::FileRead) == S_OK)
	{
		Size = DirectFile.GetSize32();
		Buffer = (PBYTE)HeapAlloc(GetProcessHeap(), 0, Size);
		DirectFile.Read(Buffer, Size);
		DirectFile.Release();
		return TRUE;
	}

	if (!Inited)
	{
		Size   = 0;
		Buffer = NULL;
		return FALSE;
	}

	String FileNameLowerCase = FileName;
	Tolower(FileNameLowerCase);
	ULONG Hash = MakeHash(FileNameLowerCase.CString());
	FileChunk Chunk;
	Chunk.Hash = Hash;


	auto itBegin = std::lower_bound(FileChunkPool.begin(), FileChunkPool.end(), Chunk);
	auto itEnd =   std::upper_bound(FileChunkPool.begin(), FileChunkPool.end(), Chunk);

	if (itBegin == FileChunkPool.end() && itEnd == FileChunkPool.end())
	{
		Size = 0;
		return FALSE;
	}

	BOOL Found = FALSE;
	vector<FileChunk>::iterator it;
	for (it = itBegin; it != itEnd; it++)
	{
		String InArcLowerCase = it->FileName;
		Tolower(InArcLowerCase);
		if (!lstrcmpiA(FileNameLowerCase.CString(), InArcLowerCase.CString()))
		{
			Found = TRUE;
			break;
		}
	}

	if (Found == FALSE)
	{
		Size = 0;
		return FALSE;
	}

	PBYTE CompFileBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), 0, it->FileCompSize);
	PBYTE FileBuffer     = (PBYTE)HeapAlloc(GetProcessHeap(), 0, it->FileOriSize);

	File.Seek(it->Offset, FILE_BEGIN);
	File.Read(CompFileBuffer, it->FileCompSize);
	DecodeFile(CompFileBuffer, it->FileCompSize);

	long ReadCompSize = 0;
	//CHuffman FileHuff;
	ReadCompSize = LZ4_uncompress((char*)CompFileBuffer, (char*)FileBuffer, it->FileOriSize);
	//FileHuff.Decode(FileBuffer, OriSize, CompFileBuffer, it->FileCompSize);
	if (ReadCompSize != it->FileCompSize)
	{
		MessageBoxW(NULL, L"文件验证失败", L"X'moe-CoreLib", MB_OK);
		Ps::ExitProcess(-1);
	}

	Size   = it->FileOriSize;
	Buffer = FileBuffer;
	HeapFree(GetProcessHeap(), 0, CompFileBuffer);
	return TRUE;
}


