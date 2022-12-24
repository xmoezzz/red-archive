#include <Windows.h>
#include "MyNative.h"
#include "detours.h"
#include "BGIraw.h"
#include "lz4.h"
#include <string>
#include <map>
#include "WinFile.h"
#include "Hash64.h"

#define BaseAppName L"[城彩学院汉化组]Friend To Lover HD"

using std::wstring;
using std::string;
using std::map;
using std::pair;

#define CP_GB2312   936
#define CP_SHIFTJIS 932

#define MY_BURIKO_SCRIPT_MAGIC "STmoeSTmoeChu>_<" //@16


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
	ULONG   Magic;
	ULONG   Size;
	ULONG   Offset;
	ULONG64 HashName;
	WCHAR   lpFileName[MAX_PATH];
}ChunkInfo;
#pragma pack(pop)


#if 0

#include <new>

class BGIArcLoader
{
	static BGIArcLoader* m_Inst;
	
	BGIArcLoader();

public:

	struct BGIArc
	{
		HANDLE hFile; //real handle
		string lpArcName;
		ULONG  Hash;
		
	};
	
	static BGIArcLoader* GetManager()
	{
		if (m_Inst == nullptr)
		{
			m_Inst = (BGIArcLoader*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BGIArcLoader));
			new (m_Inst) BGIArcLoader;
		}
		return m_Inst;
	}

	BOOL AddNode(LPCSTR lpName)
	{

	}

	BOOL Release()
	{

	}
	
private:
	
};


typedef struct BGIChunkInfo
{
	ULONG Offset;
	ULONG Size;
	
};

#endif

typedef struct ChunkAtom
{
	ULONG   Size;
	ULONG   Offset;
	WCHAR   lpFileName[MAX_PATH];

	ChunkAtom& operator = (ChunkAtom& rhs)
	{
		this->Size = rhs.Size;
		this->Offset = rhs.Offset;
		RtlCopyMemory(this->lpFileName, rhs.lpFileName, MAX_PATH);
		return *this;
	}
}ChunkAtom;

typedef LONG(CDECL* StubDecompressFile)(PVOID  pvDecompressed,
	PULONG pOutSize,
	PVOID  pvCompressed,
	ULONG  InSize,
	ULONG  SkipBytes,
	ULONG  OutBytes);


typedef LONG(CDECL* StubLoadFile)(PVOID  pvDecompressed,
	PULONG pOutSize,
	LPCSTR lpFileName,
	ULONG  SkipBytes,
	ULONG  OutBytes);

class FriendToLoverHook
{
	FriendToLoverHook();
	static FriendToLoverHook* Handle;

public:
	static FriendToLoverHook* GetGlobalData();

	~FriendToLoverHook();

	HRESULT WINAPI Init(HMODULE hSelf);
	HRESULT WINAPI UnInit(HMODULE hSelf);

	HFONT WINAPI DuplicateFontW(HDC hdc, UINT LangId);
	ULONG WINAPI AnsiToUnicode(LPCSTR lpAnsi,
		ULONG  Length,
		LPWSTR lpUnicodeBuffer,
		ULONG  BufferCount,
		ULONG  CodePage = CP_ACP);

	//@Once
	HRESULT WINAPI SetAppName();
	HRESULT WINAPI GetAppName(wstring& Name);
	wstring WINAPI GetPackageName(wstring& fileName);

	static UINT WINAPI ExitMessage(const WCHAR* Info = L"遇到致命内部错误\n游戏将会立即退出",
		const WCHAR* Title = L"Friend To Lover HD");

	HRESULT WINAPI LoadFileBuffer(LPCSTR lpFileName, PBYTE& Buffer, ULONG& OutSize);
	HRESULT WINAPI LoadFileSystem();

	HFONT WINAPI GetDefFont(){ return DefFont; }


#if 0
	HANDLE WINAPI CreateFileBoxedA(LPCSTR lpFileName);
#endif

	static BOOL WINAPI WriteInfo(const WCHAR* lpInfo, BOOL ForRelease = FALSE);
	static BOOL WINAPI WriteInfo(const CHAR*  lpInfo, BOOL ForRelease = FALSE);
	static BOOL WINAPI WriteInfo(const ULONG  lpInfo, BOOL ForRelease = FALSE);;

	static PVOID OldDecompressFile;
	static PVOID OldCheckOSDefaultLangID;
	static PVOID OldLoadFileImm;
	static PVOID OldCheckFont1;
	static PVOID OldCheckFont2;
	static PVOID OldCheckFont3;

	HWND MainWin;

private:

	WinFile SystemFile;
	HMODULE hSelfModule;
	wstring AppName;
	BOOL    InitFileSystem;
	PROC    pfTextOutA;
	PROC    pfTextOutW;
	PROC    pfCreateFontA;
	PROC    pfGetOEMCP;
	PROC    pfGetACP;
	PROC    pfCreateWindowExA;
	PROC    pfSetWindowTextA;
	PROC    pfCreateFileA;
	PROC    pfMessageBoxA;
	PROC    pfMultiByteToWideChar;
	PROC    pfRegisterClassExA;

	HFONT   DefFont;
	ChunkInfo* IndexBuffer;
	map<ULONG64, ChunkAtom> ChunkList;

#if 0
	SIZE_T CurHandle;
	map<HANDLE, coid*> BoxedFileInfo;
#endif
};

