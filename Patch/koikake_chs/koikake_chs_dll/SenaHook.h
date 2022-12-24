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

	NTSTATUS NTAPI ChangeToCommonMode();
	NTSTATUS NTAPI ChangeToIntelMode();
	NTSTATUS NTAPI ChangeToNVIDIAMode();

	NTSTATUS NTAPI ChangeToTraditionalChinese();
	NTSTATUS NTAPI ChangeToSimplifiedChinese();

	NTSTATUS NTAPI CreateInfoWindowAndCheck(HWND hWnd);

	NTSTATUS NTAPI ApplyOptimization();
	NTSTATUS NTAPI ReadOrCreateConfig();


public:
	HMODULE hSelfModule;
	HMODULE hHostModule;


	PVOID OldGetOEMCP;
	PVOID OldGetACP;
	API_POINTER(LoadLibraryExA)     OldLoadLibraryExA;
	API_POINTER(LdrLoadDll)         OldLdrLoadDll;
	API_POINTER(GetGlyphOutlineW)   OldGetGlyphOutlineA;
	API_POINTER(RegisterClassA)     OldRegisterClassA;
	WNDPROC                         OldMainWindowProc;
	LPSTR(*OldLoadText)(ULONG Args1, ULONG Args2, ULONG Args3, ULONG Args4);


	ULONG                    OriHeight, OriWidth;
	HMENU                    hMenuPop, hMenuPop2;
	HMENU                    hMenuMain;

	BOOL                     DllPatchFlag;
	ULONG                    TextCount;
	std::vector<std::string> TextPool;
	ITextCompiler*           XmoeCompiler;
	HWND                     hWnd, hInfoWnd;
	BOOL                     IsWindowInited;
	WNDPROC                  OldWindowProc;

	BOOL                     UseTraditionalChinese;

	INT                      CurIndex;

	enum { OP_COMMON, OP_NV, OP_INTEL, OP_UNK };

	enum
	{
		SENA_HAS_INTEL  = 1,
		SENA_HAS_NVIDIA = 2
	};

	LONG                     OptimizationMode;
	ULONG                    OptimizationFlag;

	API_POINTER(my_memcpy_inline)      XmoeCopyMemory;
	typedef PVOID(CDECL* ZeroMemoryProc)(PVOID, size_t);
	ZeroMemoryProc                     XmoeZeroMemory;

	PBYTE                              PrivateMemory;
private:
	BOOL                               FileSystemInit;
};
