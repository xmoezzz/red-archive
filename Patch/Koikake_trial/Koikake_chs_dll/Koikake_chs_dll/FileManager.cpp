#include "FileManager.h"

FileManager* FileManager::Handle = NULL;

#include "FileKey.h"
#include "HeaderDecodeKey.h"
#include "ChunkDecodeKey.h"
/*
extern unsigned char HeaderDecodeKey[];
extern unsigned char ChunkDecodeKey[];
extern unsigned char FileDecodeKey[];
*/

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


FileManager::FileManager() :
Inited(false)
{

}

FileManager::~FileManager()
{
	FileChunkPool.clear();
	File.Release();
}

FileManager* FileManager::Create()
{
	if (Handle == NULL)
	{
		Handle = new FileManager;
		if (Handle == NULL)
		{
			MessageBoxW(NULL, L"没有足够的空间运行文件系统", L"X'moe-CoreLib", MB_OK);
			ExitProcess(-1);
		}
	}
	return Handle;
}

FileManager* FileManager::GetFileManager()
{
	if (Handle == NULL)
	{
		MessageBoxW(NULL, L"破损的文件系统", L"X'moe-CoreLib", MB_OK);
		ExitProcess(-1);
	}
	return Handle;
}

bool FileManager::Init()
{
	if (File.Open(L"Koi.data", WinFile::FileRead) != S_OK)
	{
		//MessageBoxW(NULL, L"找不到虚拟文件系统文件(V2-LightVersion,signed)", L"X'moe-CoreLib", MB_OK);
		//ExitProcess(-1);
		this->Inited = false;
		return false;
	}
	FileHeader Header = { 0 };
	File.Read((PBYTE)(&Header), sizeof(FileHeader));

	DecodeHeader((PBYTE)(&Header), sizeof(FileHeader));

	File.Seek(Header.ChunkOffset, FILE_BEGIN);
	PBYTE ChunkBufferComp = (PBYTE)GlobalAlloc(0, Header.ChunkCompSize);
	PBYTE ChunkBuffer = (PBYTE)GlobalAlloc(0, Header.ChunkOriSize);

	if (!ChunkBuffer || !ChunkBufferComp)
	{
		MessageBoxW(NULL, L"内存不足", L"X'moe-CoreLib", MB_OK);
		ExitProcess(-1);
	}

	CHuffman HuffChunk;
	long UncompLength = 0;

	File.Seek(Header.ChunkOffset, FILE_BEGIN);
	File.Read(ChunkBufferComp, Header.ChunkCompSize);
	DecodeChunk(ChunkBufferComp, Header.ChunkCompSize);

	HuffChunk.Decode(ChunkBuffer, UncompLength, ChunkBufferComp, Header.ChunkCompSize);
	if ((ULONG)UncompLength != Header.ChunkOriSize)
	{
		MessageBoxW(NULL, L"文件错误", L"X'moe-CoreLib", MB_OK);
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

	std::sort(FileChunkPool.begin(), FileChunkPool.end());
	GlobalFree(ChunkBufferComp);
	GlobalFree(ChunkBuffer);

	Inited = true;
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

bool FileManager::QueryFile(const char* FileName, PBYTE& Buffer, ULONG& Size)
{
	WCHAR WideName[260] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, FileName, lstrlenA(FileName), WideName, 260);
	wstring FileNameFull(L"ProjectDir\\");
	FileNameFull += WideName;

#if 0
	WCHAR Info[260] = { 0 };
	wsprintfW(Info, L"try read directly : %s\n", FileNameFull.c_str());
	HANDLE hOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD nRet = 0;
	WriteConsoleW(hOutputHandle, Info, lstrlenW(Info), &nRet, NULL);
#endif

	WinFile DirectFile;
	if (DirectFile.Open(FileNameFull.c_str(), WinFile::FileRead) == S_OK)
	{
		Size = DirectFile.GetSize32();
		Buffer = (PBYTE)GlobalAlloc(0, Size);
		DirectFile.Read(Buffer, Size);
		DirectFile.Release();

#if 0
		WCHAR Info2[260] = { 0 };
		wsprintfW(Info2, L"Ok : %s\n", FileNameFull.c_str());
		HANDLE hOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD nRet = 0;
		WriteConsoleW(hOutputHandle, Info2, lstrlenW(Info2), &nRet, NULL);
#endif
		return true;
	}

	//return false;

	if (!Inited)
	{
		//MessageBoxW(NULL, L"文件系统初始化失败", L"X'moe-CoreLib", MB_OK);
		//ExitProcess(-1);
		return false;
	}
	
	ULONG Hash = MakeHash(FileName);
	FileChunk Chunk;
	Chunk.Hash = Hash;

	auto itBegin = std::lower_bound(FileChunkPool.begin(), FileChunkPool.end(), Chunk);
	auto itEnd = std::upper_bound(FileChunkPool.begin(), FileChunkPool.end(), Chunk);

	if (itBegin == FileChunkPool.end() && itEnd == FileChunkPool.end())
	{
		Size = 0;
		return false;
	}

	bool Found = false;
	vector<FileChunk>::iterator it;
	for (it = itBegin; it != itEnd; it++)
	{
		if (!stricmp(FileName, it->FileName.c_str()))
		{
			Found = true;
			break;
		}
	}
	if (Found == false)
	{
		Size = 0;
		return false;
	}

	PBYTE CompFileBuffer = (PBYTE)GlobalAlloc(0, it->FileCompSize);
	PBYTE FileBuffer = (PBYTE)GlobalAlloc(0, it->FileOriSize);

	File.Seek(it->Offset, FILE_BEGIN);
	File.Read(CompFileBuffer, it->FileCompSize);
	DecodeFile(CompFileBuffer, it->FileCompSize);

	long OriSize = 0;
	CHuffman FileHuff;
	FileHuff.Decode(FileBuffer, OriSize, CompFileBuffer, it->FileCompSize);
	if (OriSize != it->FileOriSize)
	{
		MessageBoxW(NULL, L"文件验证失败", L"X'moe-CoreLib", MB_OK);
		ExitProcess(-1);
	}

	Size = it->FileOriSize;
	Buffer = FileBuffer;
	GlobalFree(CompFileBuffer);
	return true;
}

