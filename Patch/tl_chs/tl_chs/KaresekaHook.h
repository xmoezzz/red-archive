#pragma once

#include "my.h"
#include "tp_stub.h"
#include "IStreamExXP3.h"
#include "StreamHolderXP3.h"
#include "AES.h"
#include "twofish.h"

#include <vector>
#include <string>
#include <atomic>
#include <map>

typedef HRESULT(NTAPI *FuncV2Link)(iTVPFunctionExporter *);
typedef tTJSBinaryStream* (FASTCALL * FuncCreateStream)(const ttstr &, tjs_uint32);
typedef PVOID(CDECL * FuncHostAlloc)(ULONG);
typedef iTJSTextReadStream*(FASTCALL *FuncTVPCreateTextStreamForRead)(const ttstr & name, const ttstr & modestr);

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
		Offset = o.Offset;
		Size = o.Size;
		KeySize = o.KeySize;
		Hash = o.Hash;

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

class ScaleHelper
{
public:
	ScaleHelper();
	UINT GetScaleFactor();
	void SetScaleFactor(__in UINT iDPI);
	int ScaleValue(int value);
	void ScaleRectangle(__inout RECT *pRectangle);
	void ScalePoint(__inout POINT *pPoint);
	HFONT CreateScaledFont(int nFontHeight);

private:
	UINT m_nScaleFactor;
};

class CLOGFONTW
{
public:
	CLOGFONTW(LOGFONTW* lf)
	{
		RtlCopyMemory(&Font, lf, sizeof(LOGFONTW));
	}

	CLOGFONTW& operator = (const CLOGFONTW& Other)
	{
		RtlCopyMemory(&Font, &Other.Font, sizeof(LOGFONTW));
		return *this;
	}

	LOGFONTW Font;
};

class KaresekaHook
{
public:
	KaresekaHook();
	static KaresekaHook* Handle;

	BOOL     Init(HMODULE hModule);
	BOOL     UnInit();
	NTSTATUS InitFileSystemXP3();
	IStream* CreateLocalStream(LPCWSTR lpFileName, tTJSBinaryStream* OriStream);
	NTSTATUS QueryFile(LPCWSTR QueryPathName, LPCWSTR FileName, PBYTE& FileBuffer, ULONG& FileSize, ULONG64& Hash);

	FuncCreateStream      StubTVPCreateStream;
	FuncHostAlloc         StubHostAlloc;
	FuncV2Link            StubV2Link;
	FuncTVPCreateTextStreamForRead StubTVPCreateTextStreamForRead;
	ULONG_PTR             IStreamAdapterVtable;
	iTVPFunctionExporter* TVPFunctionExporter;
	PVOID                 m_SelfModule;
	NtFileDisk            XP3PackFile;

	//vector<FileInfoChunk>     FileList;
	std::map<std::wstring, FileInfoChunk>   FileList;
	vector<HWND>              WindowList;
	std::map<void*, CLOGFONTW> FontList;
	ScaleHelper               g_ScaleHelper;

	BYTE                      m_HardwareCode[64];

private:
	BOOL   Inited;
	BOOL   FileSystemInited;
};

KaresekaHook* FASTCALL GetKareseka();
