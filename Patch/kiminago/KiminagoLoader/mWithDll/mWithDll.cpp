// mWithDll.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "mWithDll.h"
#include <windows.h>
#include "detours.h"


//////////////////////////////////////////////////////////////////////////////
//
//  This code verifies that the named DLL has been configured correctly
//  to be imported into the target process.  DLLs must export a function with
//  ordinal #1 so that the import table touch-up magic works.
//
struct ExportContext
{
    BOOL    fHasOrdinal1;
    ULONG   nExports;
};

static BOOL CALLBACK ExportCallback(PVOID pContext,
                                    ULONG nOrdinal,
                                    PCHAR pszSymbol,
                                    PVOID pbTarget)
{
    (void)pContext;
    (void)pbTarget;
    (void)pszSymbol;

    ExportContext *pec = (ExportContext *)pContext;

    if (nOrdinal == 1) {
        pec->fHasOrdinal1 = TRUE;
    }
    pec->nExports++;

    return TRUE;
}
//////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	PWCHAR pszDllPath = L"kiminago_chs.dll";

	WCHAR szDllPath[1024] = {0};
	CHAR mDllPath[1024] = { 0 };
    PWCHAR pszFilePart = NULL;

	if (!GetFullPathNameW(pszDllPath, ARRAYSIZE(szDllPath), szDllPath, &pszFilePart)) 
	{
        MessageBoxW(NULL, L"GetFullPathName Failed\n", L"Error", MB_OK);
        return false;
    }

	HMODULE hDll = LoadLibraryEx(pszDllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (hDll == NULL) 
	{
		MessageBoxW(NULL, L"Failed to load dll\n", L"��Ϸ����ʧ��", MB_OK);
        return false;
    }

	ExportContext ec;
    ec.fHasOrdinal1 = FALSE;
    ec.nExports = 0;
    DetourEnumerateExports(hDll, &ec, ExportCallback);
    FreeLibrary(hDll);

	if (!ec.fHasOrdinal1) 
	{
		MessageBoxW(NULL, L"����Dll�ļ���֤ʧ��", L"��Ϸ����ʧ��", MB_OK);
        return false;
    }
	//////////////////////////////////////////////////////////////////////////////////
	STARTUPINFO si;
    PROCESS_INFORMATION pi;
	WCHAR szCommand[2048] = {0};
	WCHAR szExe[1024] = {0};
    WCHAR szFullExe[1024] = L"\0";
    PWCHAR pszFileExe = NULL;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    szCommand[0] = L'\0';
	lstrcpyW(szExe, L"GameCore.dll");
	lstrcpyW(szCommand, L"GameCore.dll");
	//////////////////////////////////////////////////////////////////////////////////
	DWORD dwFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;

	WideCharToMultiByte(CP_ACP, 0, szDllPath, lstrlenW(szDllPath), mDllPath, 1024, nullptr, nullptr);

    SetLastError(0);
    SearchPathW(NULL, szExe, L".exe", ARRAYSIZE(szFullExe), szFullExe, &pszFileExe);
    if (!DetourCreateProcessWithDllExW(szFullExe[0] ? szFullExe : NULL, szCommand,
                                      NULL, NULL, TRUE, dwFlags, NULL, NULL,
                                      &si, &pi, mDllPath, NULL)) 
	{
        DWORD dwError = GetLastError();
		MessageBoxW(NULL, L"�޷�������Ϸ����", L"��Ϸ����ʧ��", MB_OK);
        ExitProcess(-1);
    }

    ResumeThread(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD dwResult = 0;
    if (!GetExitCodeProcess(pi.hProcess, &dwResult)) 
	{
		MessageBoxW(NULL, L"�ڲ�������Ϸ���ܻ��������У�", L"��Ϸ����ʧ��", MB_OK);
        return false;
    }

    return true;
}