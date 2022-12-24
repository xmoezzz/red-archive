#pragma once

#include <Windows.h>
#include <ntstatus.h>
#include <WinFile.h>
#include <string>
#include <map>

using std::wstring;
using std::map;
using std::pair;


#pragma pack(push, 1)
typedef struct HeaderInfo
{
	ULONG Magic;
	ULONG Size;
	ULONG Count;
	ULONG Key;
}HeaderInfo;

typedef struct ChunkInfo
{
	DWORD   Magic;
	DWORD   Size;
	DWORD   Offset;
	ULONG64 HashName;
	WCHAR   lpFileName[MAX_PATH];
}ChunkInfo;
#pragma pack(pop)

typedef struct ChunkAtom
{
	DWORD   Size;
	DWORD   Offset;
	WCHAR   lpFileName[MAX_PATH];

	ChunkAtom& operator = (ChunkAtom& rhs)
	{
		this->Size = rhs.Size;
		this->Offset = rhs.Offset;
		RtlCopyMemory(this->lpFileName, rhs.lpFileName, MAX_PATH);
		return *this;
	}
}ChunkAtom;

class FileLoader
{
	static FileLoader* m_Inst;
	FileLoader();

public:
	static FileLoader* GetFileSystem();

	NTSTATUS NTAPI Init();
	NTSTATUS NTAPI LoadFile(LPCSTR FileName, PBYTE& Buffer, SIZE_T& Size);
	
	static NTSTATUS NTAPI Alloc(SIZE_T Size, LPBYTE& lpMem);
	static NTSTATUS NTAPI Free (LPBYTE& lpMem);

private:
	WinFile SystemFile;
	BOOL    InitFileSystem;
	
	map<ULONG64, ChunkAtom> ChunkList;
};
