#include "stdafx.h"
#include "detours.h"
#include "GeneralFontHook.h"
#include "ScriptInfo.h"
#include "BinaryCode.h"
#include <Shlobj.h>
#include <string>
#include "AnzHeader.h"

using std::wstring;

BOOL StartHook(LPCSTR szDllName, PROC pfnOrg, PROC pfnNew)
{
	HMODULE hmod;
	LPCSTR szLibName;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	PIMAGE_THUNK_DATA pThunk;
	DWORD dwOldProtect, dwRVA;
	PBYTE pAddr;

	hmod = GetModuleHandleW(NULL);
	pAddr = (PBYTE)hmod;
	pAddr += *((DWORD*)&pAddr[0x3C]);
	dwRVA = *((DWORD*)&pAddr[0x80]);

	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)hmod + dwRVA);

	for (; pImportDesc->Name; pImportDesc++)
	{
		szLibName = (LPCSTR)((DWORD)hmod + pImportDesc->Name);
		if (!stricmp(szLibName, szDllName))
		{
			pThunk = (PIMAGE_THUNK_DATA)((DWORD)hmod + pImportDesc->FirstThunk);

			for (; pThunk->u1.Function; pThunk++)
			{
				if (pThunk->u1.Function == (DWORD)pfnOrg)
				{
					VirtualProtect((LPVOID)&pThunk->u1.Function, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
					pThunk->u1.Function = (DWORD)pfnNew;
					VirtualProtect((LPVOID)&pThunk->u1.Function, 4, dwOldProtect, &dwOldProtect);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}


int WINAPI HooklStrcmpiA(LPCSTR lpString1, LPCSTR lpString2)
{
	int ret = CompareStringA(0x411, 1, lpString1, -1, lpString2, -1);
	return ret - 2;
}

int WINAPI HookMessageBoxA( HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	WCHAR* uTitle = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024 * 2);
	WCHAR* uInfo = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024 * 2);
	MultiByteToWideChar(932, 0, lpText, -1, uInfo, 1024);
	if (!strncmp(lpCaption, "[X'moe", strlen("[X'moe")))
	{
		MultiByteToWideChar(936, 0, lpCaption, -1, uTitle, 1024);
	}
	else
	{ 
		MultiByteToWideChar(932, 0, lpCaption, -1, uTitle, 1024); 
	}
	int result =  MessageBoxW(hWnd, uInfo, uTitle, uType);
	HeapFree(GetProcessHeap(), 0, uTitle);
	HeapFree(GetProcessHeap(), 0, uInfo);
	return result;
}


HWND WINAPI HookCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	RECT    rcWordArea;
	ULONG   Length;
	LPWSTR  ClassName, WindowName;

	Length = lstrlenA(lpClassName) + 1;
	ClassName = (LPWSTR)alloca(Length * sizeof(WCHAR));
	RtlZeroMemory(ClassName, Length * sizeof(WCHAR));
	MultiByteToWideChar(932, 0, lpClassName, Length, ClassName, Length * sizeof(WCHAR));

	Length = lstrlenA(lpWindowName) + 1;
	WindowName = (LPWSTR)alloca(Length * sizeof(WCHAR));
	RtlZeroMemory(WindowName, Length * sizeof(WCHAR));
	MultiByteToWideChar(936, 0, lpWindowName, Length, WindowName, Length * sizeof(WCHAR));

	return CreateWindowExW(dwExStyle, ClassName, WindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

HFONT WINAPI HookCreateFontA(int nHeight, int  nWidth, int  nEscapement, int nOrientation,
	int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut,
	DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision,
	DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
	WCHAR wszFace[128] = {0};
	MultiByteToWideChar(CP_ACP, 0, lpszFace, lstrlenA(lpszFace), wszFace, 128);

	return  CreateFontW(nHeight, nWidth, nEscapement, nOrientation,
		fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut,
		0x86, fdwOutputPrecision, fdwClipPrecision,
		fdwQuality, fdwPitchAndFamily, wszFace);
}

wstring WINAPI RedirectSavePath(const wchar_t* str)
{
	wstring a(str);
	wstring b = a.substr(a.find_last_of(L'/') + 1, a.length());
	wstring result(L"save_chs/");
	result += b;
	return b;
}

static HANDLE hScript = INVALID_HANDLE_VALUE;


static BOOL ReadOnce = TRUE;
BOOL WINAPI HookReadFileV2(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
	LPDWORD  lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	if (hFile != INVALID_HANDLE_VALUE && 
		hFile != (HANDLE)0 &&
		hFile == hScript)
	{
		if (ReadOnce == FALSE)
		{
			return TRUE;
		}

		HANDLE hHandle = CreateFileW(L"Shinku_chs.anz", GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD iRet = 0;
		DWORD Size = GetFileSize(hHandle, &iRet);
		HeapReAlloc(GetProcessHeap(), 0, lpBuffer, Size);
		
		BOOL result = ReadFile(hFile, lpBuffer, Size, lpNumberOfBytesRead, lpOverlapped);
		BufferDecoder((unsigned char*)lpBuffer, Size);
		CloseHandle(hHandle);
		ReadOnce = FALSE;
		return result;
	}
	else
	{
		return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	}
}


BOOL WINAPI HookCloseHandle(HANDLE hObject)
{
	if (hObject != INVALID_HANDLE_VALUE &&
		hObject != (HANDLE)0 && 
		hObject == hScript)
	{
		hScript = INVALID_HANDLE_VALUE;
	}
	return CloseHandle(hObject);
}


using std::wstring;

wstring WINAPI ReFormatPath(const wchar_t* str)
{
	wstring a(str);
	wstring b = a.substr(a.find_last_of('/') + 1, a.length());
	static const wchar_t* Prefix = L"save_chs\\";
	wstring result(Prefix);
	result += b;
	return result;
}


HANDLE WINAPI HookFindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
	return FindFirstFileA("*.anz", lpFindFileData);
}


VOID WINAPI OutputStringA(const CHAR* lpString)
{
	HANDLE hOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD nRet = 0;
	WriteConsoleA(hOutputHandle, lpString, lstrlenA(lpString), &nRet, NULL);
}



HANDLE WINAPI HookCreateFileA(
	_In_     LPCSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
	)
{
	OutputStringA(lpFileName);
	OutputStringA("\n");
	if (!strcmp(lpFileName, "Shinku_chs.anz"))
	{
		hScript = CreateFileW(L"Shinku_chs.anz", dwDesiredAccess, dwShareMode,
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		return hScript;
	}
	else
	{
		return CreateFileA(lpFileName, dwDesiredAccess, dwShareMode,
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
}

LPVOID pfCreateFileW = NULL;
typedef HANDLE(WINAPI* PfunCreateFileW)(LPCTSTR lpFileName,DWORD  dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
	DWORD  dwFlagsAndAttributes, HANDLE hTemplateFile);

HANDLE WINAPI HookCreateFileW(
	_In_     LPCWSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
	)
{
	if (wcsstr(lpFileName, L"movie/01.wmv") || wcsstr(lpFileName, L"movie/01.mpg"))
	{
		return (PfunCreateFileW(pfCreateFileW))(L"movie\\Shinku_op.wmv", dwDesiredAccess, dwShareMode,
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	else
	{
		return (PfunCreateFileW(pfCreateFileW))(lpFileName, dwDesiredAccess, dwShareMode,
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
}


BOOL WINAPI HookGetVersionExA(LPOSVERSIONINFOA lpVersionInfo)
{
	BOOL Result = GetVersionExA(lpVersionInfo);
	lpVersionInfo->dwMajorVersion = 5;
	lpVersionInfo->dwMinorVersion = 1;
	return Result;
}


VOID WINAPI OutputStringW(const WCHAR* lpString)
{
	HANDLE hOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD nRet = 0;
	WriteConsoleW(hOutputHandle, lpString, lstrlenW(lpString), &nRet, NULL);
	WriteConsoleW(hOutputHandle, L"\n", 1, &nRet, NULL);
}


int WINAPI HookMultiByteToWideChar(
	_In_      UINT   CodePage,
	_In_      DWORD  dwFlags,
	_In_      LPCSTR lpMultiByteStr,
	_In_      int    cbMultiByte,
	_Out_opt_ LPWSTR lpWideCharStr,
	_In_      int    cchWideChar
	)
{
	int Result = 0;
	/*
	\movie\01.wmv
	\movie\01.mpg
	*/
	if (strstr(lpMultiByteStr, "\\movie\\01.wmv") || strstr(lpMultiByteStr, "\\movie\\01.mpg"))
	{
		RtlZeroMemory((void*)lpWideCharStr, MAX_PATH * 2);
		GetCurrentDirectoryW(MAX_PATH, lpWideCharStr);
		lstrcatW(lpWideCharStr, L"\\");
		lstrcatW(lpWideCharStr, L"\\movie\\Shinku_op.wmv");

		//OutputStringW(lpWideCharStr);
		Result = lstrlenW(lpWideCharStr);
	}
	else
	{
		Result = MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr,
			cbMultiByte, lpWideCharStr, cchWideChar);

		//OutputStringW(lpWideCharStr);
	}
	return Result;
}

BOOL WINAPI HookIsDebuggerPresent()
{
	return 0;
}


UINT WINAPI HookGetOEMCP()
{
	return (UINT)936;
}

UINT WINAPI HookGetACP()
{
	return (UINT)936;
}

BOOL WINAPI HookGetCPInfo(UINT CodePage, LPCPINFO lpCPInfo)
{
	return GetCPInfo(936, lpCPInfo);
}

int WINAPI HookEnumFontFamiliesExA(HDC hdc, LPLOGFONTA lpLogfont, FONTENUMPROCA lpProc, LPARAM lParam, DWORD dwFlags)
{
	lpLogfont->lfCharSet = GB2312_CHARSET;
	return EnumFontFamiliesExA(hdc, lpLogfont, lpProc, lParam, dwFlags);
}


FARPROC pfEnumFontFamiliesExA = NULL;
FARPROC pfGetCPInfo = NULL;
FARPROC pfGetACP = NULL;
FARPROC pfGetOEMCP = NULL;
FARPROC pfIsDebuggerPresent = NULL;
FARPROC pfCreateWindowA = NULL;
FARPROC pfMessageBoxA = NULL;
FARPROC pflStrcmpiA = NULL;
FARPROC pfCreateFontA = NULL;
FARPROC pfCreateFileA = NULL;
FARPROC pfGetFileSize = NULL;
FARPROC pfReadFile = NULL;
FARPROC pfSHCreateDirectory = NULL;
FARPROC pfFindFirstFileA = NULL;
FARPROC pfCloseHandle = NULL;
FARPROC pfGetVersionExA = NULL;
FARPROC pfMultiByteToWideChar = NULL;

static WCHAR* ErrorHeader = L"[X'moe - ´íÎó±¨¸æ]";

BOOL WINAPI CheckHcbScript()
{
	HANDLE hFavo = CreateFileW(L"Shinku.hcb", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFavo == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	else
	{
		CloseHandle(hFavo);
		return TRUE;
	}
}

__declspec(dllexport)const WCHAR* WINAPI XmoePatch()
{
	static const WCHAR Info[] = L"X'moe_LinkProc";
	return Info;
}




/***********************************/
BOOL WINAPI InitHook()
{
	//AllocConsole();

	BOOL bCheck = CheckHcbScript();
	if (!bCheck)
	{
		MessageBoxW(NULL, L"È±ÉÙÔ­Ê¼ÎÄ¼þ:Shinku.hcb", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}

	InstallFont();

	pfCreateWindowA = GetProcAddress(GetModuleHandleW(L"User32.dll"), "CreateWindowExA");
	pfMessageBoxA   = GetProcAddress(GetModuleHandleW(L"User32.dll"), "MessageBoxA");
	pflStrcmpiA     = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "lstrcmpiA");
	pfCreateFontA   = GetProcAddress(GetModuleHandleW(L"Gdi32.dll"), "CreateFontA");
	pfCreateFileA   = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "CreateFileA");
	pfSHCreateDirectory = GetProcAddress(GetModuleHandleW(L"Shell32.dll"), "SHCreateDirectory");
	

	if (!StartHook("User32.dll", pfCreateWindowA, (PROC)HookCreateWindowExA))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0x00000000]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}
	if (!StartHook("User32.dll", pfMessageBoxA, (PROC)HookMessageBoxA))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0x00000001]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}
	if (!StartHook("Kernel32.dll", pflStrcmpiA, (PROC)HooklStrcmpiA))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0x00000002]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}
	if (!StartHook("Gdi32.dll", pfCreateFontA, (PROC)HookCreateFontA))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0x00000003]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}
	if (!StartHook("Kernel32.dll", pfCreateFileA, (PROC)HookCreateFileA))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0x00000004]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}

	pfFindFirstFileA = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "FindFirstFileA");
	if (!StartHook("Kernel32.dll", pfFindFirstFileA, (PROC)HookFindFirstFileA))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0xFFFF0000]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}
	
	pfReadFile = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "ReadFile");
	if (!StartHook("Kernel32.dll", pfReadFile, (PROC)HookReadFileV2))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0xFFFF0001]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}

	pfCloseHandle = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "CloseHandle");
	if (!StartHook("Kernel32.dll", pfCloseHandle, (PROC)HookCloseHandle))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0xFFFF0002]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}

	pfGetVersionExA = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "GetVersionExA");
	if (!StartHook("Kernel32.dll", pfGetVersionExA, (PROC)HookGetVersionExA))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0xFFFF0003]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}

	pfMultiByteToWideChar = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "MultiByteToWideChar");
	if (!StartHook("Kernel32.dll", pfMultiByteToWideChar, (PROC)HookMultiByteToWideChar))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0xFFFF0004]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}

	pfIsDebuggerPresent = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "IsDebuggerPresent");
	if (!StartHook("Kernel32.dll", pfIsDebuggerPresent, (PROC)HookIsDebuggerPresent))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0xFFFF0005]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}

	pfGetOEMCP = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "GetOEMCP");
	if (!StartHook("Kernel32.dll", pfGetOEMCP, (PROC)HookGetOEMCP))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0xFFFF0006]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}

	pfGetACP = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "GetACP");
	if (!StartHook("Kernel32.dll", pfGetACP, (PROC)HookGetACP))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0xFFFF0007]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}

	pfGetCPInfo = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "GetCPInfo");
	if (!StartHook("Kernel32.dll", pfGetCPInfo, (PROC)HookGetCPInfo))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0xFFFF0008]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}

	pfEnumFontFamiliesExA = GetProcAddress(GetModuleHandleW(L"GDI32.dll"), "EnumFontFamiliesExA");
	if (!StartHook("GDI32.dll", pfEnumFontFamiliesExA, (PROC)HookEnumFontFamiliesExA))
	{
		MessageBoxW(NULL, L"Æô¶¯Ê§°Ü[code : 0xFFFF0009]", ErrorHeader, MB_OK);
		ExitProcess(-1);
		return FALSE;
	}

	return TRUE;
}


BOOL WINAPI UninitHook()
{
	return TRUE;
}

