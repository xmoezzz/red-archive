#pragma once
#include "my.h"
#include "tp_stub.h"
#include <vector>
#include <string>
#include <algorithm>

using std::vector;
using std::wstring;

typedef struct FileInfoChunk
{
	wstring FileName;
	ULONG   Offset;
	ULONG   Size;
	ULONG   KeySize;
	ULONG64 Hash;

	FileInfoChunk& operator = (const FileInfoChunk& o)
	{
		FileName = o.FileName;
		Offset   = o.Offset;
		Size     = o.Size;
		KeySize  = o.KeySize;
		Hash     = o.Hash;

		return *this;
	}


	friend bool operator >(const FileInfoChunk& lhs, const FileInfoChunk& rhs)
	{
		return lhs.Hash > rhs.Hash;
	}

	friend bool operator >=(const FileInfoChunk& lhs, const FileInfoChunk& rhs)
	{
		return lhs.Hash >= rhs.Hash;
	}

	friend bool operator <(const FileInfoChunk& lhs, const FileInfoChunk& rhs)
	{
		return lhs.Hash < rhs.Hash;
	}

	friend bool operator <=(const FileInfoChunk& lhs, const FileInfoChunk& rhs)
	{
		return lhs.Hash <= rhs.Hash;
	}

	friend bool operator ==(const FileInfoChunk& lhs, const FileInfoChunk& rhs)
	{
		return lhs.Hash == rhs.Hash;
	}

	friend bool operator != (const FileInfoChunk& lhs, const FileInfoChunk& rhs)
	{
		return lhs.Hash != rhs.Hash;
	}
}FileInfoChunk;



typedef struct FileInfoChunkXP3
{
	wstring FileName;
	ULONG   Offset;
	ULONG64 Hash;
	ULONG   Size;

	FileInfoChunkXP3& operator = (const FileInfoChunkXP3& o)
	{
		FileName = o.FileName;
		Offset = o.Offset;
		Size = o.Size;
		Hash = o.Hash;

		return *this;
	}


	friend bool operator >(const FileInfoChunkXP3& lhs, const FileInfoChunkXP3& rhs)
	{
		return lhs.Hash > rhs.Hash;
	}

	friend bool operator >=(const FileInfoChunkXP3& lhs, const FileInfoChunkXP3& rhs)
	{
		return lhs.Hash >= rhs.Hash;
	}

	friend bool operator <(const FileInfoChunkXP3& lhs, const FileInfoChunkXP3& rhs)
	{
		return lhs.Hash < rhs.Hash;
	}

	friend bool operator <=(const FileInfoChunkXP3& lhs, const FileInfoChunkXP3& rhs)
	{
		return lhs.Hash <= rhs.Hash;
	}

	friend bool operator ==(const FileInfoChunkXP3& lhs, const FileInfoChunkXP3& rhs)
	{
		return lhs.Hash == rhs.Hash;
	}

	friend bool operator != (const FileInfoChunkXP3& lhs, const FileInfoChunkXP3& rhs)
	{
		return lhs.Hash != rhs.Hash;
	}
}FileInfoChunkXP3;

typedef HRESULT(_stdcall *tV2Link)(iTVPFunctionExporter *);
typedef tTJSBinaryStream* (FASTCALL * FuncCreateStream)(const ttstr &, tjs_uint32);

class ShinkuHook
{
	static ShinkuHook* m_Inst;
	ShinkuHook();

public:

	~ShinkuHook();

	static ShinkuHook* GetHook();

	NTSTATUS Init(HMODULE hModule);
	NTSTATUS UnInit(HMODULE hModule);
	NTSTATUS NotifyThreadAttach(HMODULE hModule);
	NTSTATUS NotifyThreadDetach(HMODULE hMoudle);

	NTSTATUS InitKrkrHook(LPCWSTR lpFileName, PVOID Module);

	NTSTATUS QueryFile(LPWSTR FileName, PBYTE& Buffer, ULONG& Size);
	NTSTATUS QueryFileXP3(LPWSTR FileName, PBYTE& Buffer, ULONG& Size, ULONG64& Hash);
	IStream* CreateLocalStream(LPCWSTR lpFileName);

private: 
	NTSTATUS InitAddtion();
	NTSTATUS LaunchAntiLiveWorker();
	NTSTATUS InitFileSystemXP3();
	NTSTATUS InitFileSystem();
	NTSTATUS LoadDirectShowFilter();

	BOOL                     Inited;
	NtFileDisk               PackFile, XP3PackFile;
	
	vector<FileInfoChunk>    FileList;
	vector<FileInfoChunkXP3> XP3FileList;

public:
	FuncCreateStream         StubCreateIStream;
	tV2Link                  StubV2Link;
	iTVPFunctionExporter *   TVPFunctionExporter;
	HMODULE                  m_SelfModule;
	API_POINTER(LdrLoadDll)  OldLdrLoadDll;
	BOOL                     DebugPort;
};

