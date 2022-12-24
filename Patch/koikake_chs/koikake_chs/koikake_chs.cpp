#include "Koikake_chs.h"
#include <Windows.h>
#include "detours.h"

#pragma comment(lib,"detours.lib")

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
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	PWCHAR pszDllPath = L"Koikake_chs.dll";

	WCHAR szDllPath[1024] = { 0 };
	CHAR mDllPath[1024] = { 0 };
	PWCHAR pszFilePart = NULL;

	if (!GetFullPathNameW(pszDllPath, ARRAYSIZE(szDllPath), szDllPath, &pszFilePart))
	{
		MessageBoxW(NULL, L"内部错误[0]", L"游戏启动失败", MB_OK);
		return false;
	}

	HMODULE hDll = LoadLibraryEx(pszDllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
	if (hDll == NULL)
	{
		MessageBoxW(NULL, L"无法加载汉化文件", L"游戏启动失败", MB_OK);
		return false;
	}

	ExportContext ec;
	ec.fHasOrdinal1 = FALSE;
	ec.nExports = 0;
	DetourEnumerateExports(hDll, &ec, ExportCallback);
	FreeLibrary(hDll);

	if (!ec.fHasOrdinal1)
	{
		MessageBoxW(NULL, L"汉化Dll文件验证失败", L"游戏启动失败", MB_OK);
		return false;
	}
	//////////////////////////////////////////////////////////////////////////////////
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	WCHAR szCommand[2048] = { 0 };
	WCHAR szExe[1024] = { 0 };
	WCHAR szFullExe[1024] = L"\0";
	PWCHAR pszFileExe = NULL;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	szCommand[0] = L'\0';

#if 1
	lstrcpyW(szExe, L"GameCore.dll");
	lstrcpyW(szCommand, L"GameCore.dll");
#else
	lstrcpyW(szExe, L"koikake.exe");
	lstrcpyW(szCommand, L"koikake.exe");
#endif

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
		MessageBoxW(NULL, L"无法启动游戏进程", L"游戏启动失败", MB_OK);
		ExitProcess(-1);
	}

	ResumeThread(pi.hThread);

	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD dwResult = 0;
	if (!GetExitCodeProcess(pi.hProcess, &dwResult))
	{
		MessageBoxW(NULL, L"内部错误（游戏可能会正常运行）", L"游戏启动失败", MB_OK);
		return false;
	}
	return true;
}

