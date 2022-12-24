#include "stdafx.h"
#include <Windows.h>
#include <string>
#include "detours.h"

#pragma comment(lib, "detours.lib")

using std::wstring;


wstring GetFileName(const WCHAR* lpString)
{
	wstring tmp(lpString);
	return tmp.substr(tmp.find_last_of(L"\\") + 1, wstring::npos);
}

PVOID pfMultiByteToWideChar = NULL;
typedef int (WINAPI *PfunMultiByteToWideChar)(
	_In_      UINT   CodePage,
	_In_      DWORD  dwFlags,
	_In_      LPCSTR lpMultiByteStr,
	_In_      int    cbMultiByte,
	_Out_opt_ LPWSTR lpWideCharStr,
	_In_      int    cchWideChar
	);

int WINAPI HookMultiByteToWideChar(
	_In_      UINT   CodePage,
	_In_      DWORD  dwFlags,
	_In_      LPCSTR lpMultiByteStr,
	_In_      int    cbMultiByte,
	_Out_opt_ LPWSTR lpWideCharStr,
	_In_      int    cchWideChar
	)
{
	if (CodePage == CP_ACP)
	{
		return (PfunMultiByteToWideChar(pfMultiByteToWideChar))(932, dwFlags, lpMultiByteStr,
			cbMultiByte, lpWideCharStr, cchWideChar);
	}
	else
	{
		return (PfunMultiByteToWideChar(pfMultiByteToWideChar))(CodePage, dwFlags, lpMultiByteStr,
			cbMultiByte, lpWideCharStr, cchWideChar);
	}
}



PVOID pfOldCreateFileW = NULL;
typedef HANDLE(WINAPI *PfunHookCreateFile)(LPCWSTR lpFileName, DWORD  dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD  dwFlagsAndAttributes,
	HANDLE hTemplateFile);

HANDLE WINAPI HookCreateFile(
	_In_     LPCWSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
	)
{
	wstring FileName = GetFileName(lpFileName);
	if (!wcsicmp(FileName.c_str(), L"patch2.xp3") || 
		!wcsicmp(FileName.c_str(), L"patch3.xp3") ||
		!wcsicmp(FileName.c_str(), L"patch4.xp3"))
	{
		return INVALID_HANDLE_VALUE;
	}
	else
	{

#if 0
		HANDLE hOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD nRet = 0;
		WriteConsoleW(hOutputHandle, lpFileName, lstrlenW(lpFileName), &nRet, NULL);
		WriteConsoleW(hOutputHandle, L"\n", 1, &nRet, NULL);
#endif

		return (PfunHookCreateFile(pfOldCreateFileW))(lpFileName, dwDesiredAccess, dwShareMode,
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes,
			hTemplateFile);
	}
}

PVOID pfLoadLibraryW = NULL;
typedef HMODULE (WINAPI *PfunLoadLibraryW)( LPCTSTR lpFileName);

HMODULE WINAPI HookLoadLibraryW(LPCTSTR lpFileName)
{
	wstring FileName = GetFileName(lpFileName);
	if (wcsstr(FileName.c_str(), L"KrkrExtract"))
	{
		return NULL;
	}
	else
	{
		return (PfunLoadLibraryW(pfLoadLibraryW))(lpFileName);
	}
}


__declspec(dllexport)WCHAR* WINAPI XmoeLinkProc()
{
	static WCHAR* Result = L"LinkVersion-SabbatInformation";
	return Result;
}


HRESULT WINAPI Init()
{

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfOldCreateFileW = DetourFindFunction("Kernel32.dll", "CreateFileW");
	DetourAttach(&pfOldCreateFileW, HookCreateFile);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfMultiByteToWideChar = DetourFindFunction("Kernel32.dll", "MultiByteToWideChar");
	DetourAttach(&pfMultiByteToWideChar, HookMultiByteToWideChar);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pfLoadLibraryW = DetourFindFunction("Kernel32.dll", "LoadLibraryW");
	DetourAttach(&pfLoadLibraryW, HookLoadLibraryW);
	DetourTransactionCommit();

	return S_OK;
}

HRESULT WINAPI UnInit()
{
	return S_OK;
}

