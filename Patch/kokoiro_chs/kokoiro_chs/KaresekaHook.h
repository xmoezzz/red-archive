#pragma once

#include "my.h"
#include "tp_stub.h"
#include "IStreamExXP3.h"
#include "StreamHolderXP3.h"
#include <vector>
#include <string>

typedef HRESULT(NTAPI *FuncV2Link)(iTVPFunctionExporter *);
typedef tTJSBinaryStream* (FASTCALL * FuncCreateStream)(const ttstr &, tjs_uint32);
typedef PVOID(CDECL * FuncHostAlloc)(ULONG);

using std::vector;
using std::wstring;

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

class KaresekaHook
{
public:
	KaresekaHook();
	static KaresekaHook* Handle;

	BOOL     Init(HMODULE hModule);
	BOOL     UnInit();
	NTSTATUS InitFileSystemXP3();
	IStream* CreateLocalStream(LPCWSTR lpFileName);
	NTSTATUS InitKrkrHook(LPCWSTR lpFileName, PVOID Module);
	NTSTATUS QueryFile(LPCWSTR QueryPathName, LPCWSTR FileName, PBYTE& FileBuffer, ULONG& FileSize, ULONG64& Hash);
	NTSTATUS QueryFileXP3(LPWSTR FileName, PBYTE& Buffer, ULONG& Size, ULONG64& Hash);

	FuncCreateStream      StubTVPCreateStream;
	FuncHostAlloc         StubHostAlloc;
	FuncV2Link            StubV2Link;
	ULONG_PTR             IStreamAdapterVtable;
	iTVPFunctionExporter* TVPFunctionExporter;
	PVOID                 m_SelfModule;

	vector<FileInfoChunkXP3> XP3FileList;

private:
	BOOL       Inited;
	BOOL       FileSystemInited;
	BOOL       IsBIG5;
	NtFileDisk XP3PackFile;
};

KaresekaHook* FASTCALL GetKareseka();
