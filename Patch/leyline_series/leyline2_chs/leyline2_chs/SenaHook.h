#include <Windows.h>
#include <ntstatus.h>
#include <vector>
#include <string>
#include "my.h"
#include "TextCompiler.h"

class SenaHook
{
	static SenaHook* m_Inst;
	SenaHook();

public:
	~SenaHook();

	static SenaHook* GetSenaHook();

	NTSTATUS NTAPI SetSelfModule(HMODULE  hModule);
	NTSTATUS NTAPI GetSelfModule(HMODULE& hModule);
	NTSTATUS NTAPI GetHostModule(HMODULE& hModule);

	NTSTATUS NTAPI Init();
	NTSTATUS NTAPI UnInit();

	//Old Binary Format
	NTSTATUS NTAPI AttachTextBuffer(PBYTE FileBuffer, ULONG Length);
	NTSTATUS NTAPI LoadBuffer(LPCSTR  lpFileName, PBYTE& Buffer, ULONG_PTR& Size);
	NTSTATUS NTAPI LoadBuffer(LPWSTR  lpFileName, PBYTE& Buffer, ULONG_PTR& Size);
	NTSTATUS NTAPI InitFileSystem();

public:
	HMODULE hSelfModule;
	HMODULE hHostModule;


	PVOID OldGetOEMCP;
	PVOID OldGetACP;
	API_POINTER(LoadLibraryExA)     OldLoadLibraryExA;
	API_POINTER(LdrLoadDll)         OldLdrLoadDll;
	API_POINTER(GetGlyphOutlineW)   OldGetGlyphOutlineA;
	LPSTR(*OldLoadText)(ULONG Args1, ULONG Args2, ULONG Args3, ULONG Args4);

	BOOL                     DllPatchFlag;
	ULONG                    TextCount;
	std::vector<std::string> TextPool;
	ITextCompiler*           XmoeCompiler;
	HWND                     hWnd;
	BOOL                     IsWindowInited;
	WNDPROC                  OldWindowProc;

private:
	BOOL                     FileSystemInit;
};
