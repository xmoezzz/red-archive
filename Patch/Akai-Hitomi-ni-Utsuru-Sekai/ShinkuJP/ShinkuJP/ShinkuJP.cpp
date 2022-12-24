#include <Windows.h>
#include "detours.h"
//#include "Loader.cpp"

#pragma comment(lib,"detours.lib")


// 函数声明
typedef BOOL(WINAPI* Proc_CreateProcessW)(LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation);

typedef HMODULE(WINAPI* Func_LoadLibraryW)(LPCWSTR lpLibFileName);


BYTE* mov_eax_xx(BYTE* lpCurAddres, DWORD eax)
{
	*lpCurAddres = 0xB8;
	*(DWORD*)(lpCurAddres + 1) = eax;
	return lpCurAddres + 5;
}

BYTE* mov_ebx_xx(BYTE* lpCurAddres, DWORD ebx)
{
	*lpCurAddres = 0xBB;
	*(DWORD*)(lpCurAddres + 1) = ebx;
	return lpCurAddres + 5;
}

BYTE* mov_ecx_xx(BYTE* lpCurAddres, DWORD ecx)
{
	*lpCurAddres = 0xB9;
	*(DWORD*)(lpCurAddres + 1) = ecx;
	return lpCurAddres + 5;
}

BYTE* mov_edx_xx(BYTE* lpCurAddres, DWORD edx)
{
	*lpCurAddres = 0xBA;
	*(DWORD*)(lpCurAddres + 1) = edx;
	return lpCurAddres + 5;
}

BYTE* mov_esi_xx(BYTE* lpCurAddres, DWORD esi)
{
	*lpCurAddres = 0xBE;
	*(DWORD*)(lpCurAddres + 1) = esi;
	return lpCurAddres + 5;
}

BYTE* mov_edi_xx(BYTE* lpCurAddres, DWORD edi)
{
	*lpCurAddres = 0xBF;
	*(DWORD*)(lpCurAddres + 1) = edi;
	return lpCurAddres + 5;
}

BYTE* mov_ebp_xx(BYTE* lpCurAddres, DWORD ebp)
{
	*lpCurAddres = 0xBD;
	*(DWORD*)(lpCurAddres + 1) = ebp;
	return lpCurAddres + 5;
}

BYTE* mov_esp_xx(BYTE* lpCurAddres, DWORD esp)
{
	*lpCurAddres = 0xBC;
	*(DWORD*)(lpCurAddres + 1) = esp;
	return lpCurAddres + 5;
}

BYTE* mov_eip_xx(BYTE* lpCurAddres, DWORD eip, DWORD newEip)
{
	if (!newEip)
	{
		newEip = (DWORD)lpCurAddres;
	}

	*lpCurAddres = 0xE9;
	*(DWORD*)(lpCurAddres + 1) = eip - (newEip + 5);
	return lpCurAddres + 5;
}

BYTE* push_xx(BYTE* lpCurAddres, DWORD dwAdress)
{

	*lpCurAddres = 0x68;
	*(DWORD*)(lpCurAddres + 1) = dwAdress;

	return lpCurAddres + 5;
}

BYTE* Call_xx(BYTE* lpCurAddres, DWORD eip, DWORD newEip)
{
	if (!newEip)
	{
		newEip = (DWORD)lpCurAddres;
	}

	*lpCurAddres = 0xE8;
	*(DWORD*)(lpCurAddres + 1) = eip - (newEip + 5);
	return lpCurAddres + 5;
}

BOOL SuspendTidAndInjectCode(HANDLE hProcess, HANDLE hThread, DWORD dwFuncAdress, const BYTE * lpShellCode, size_t uCodeSize)
{
	SIZE_T NumberOfBytesWritten = 0;
	BYTE ShellCodeBuf[0x480];
	CONTEXT Context;
	DWORD flOldProtect = 0;
	LPBYTE lpCurESPAddress = NULL;
	LPBYTE lpCurBufAdress = NULL;
	BOOL bResult = FALSE;


	// 挂载起线程
	SuspendThread(hThread);

	memset(&Context, 0, sizeof(Context));
	Context.ContextFlags = CONTEXT_FULL;

	if (GetThreadContext(hThread, &Context))
	{
		// 在对方线程中开辟一个 0x480 大小的局部空
		lpCurESPAddress = (LPBYTE)((Context.Esp - 0x480) & 0xFFFFFFE0);

		// 获取指针 用指针来操作
		lpCurBufAdress = &ShellCodeBuf[0];

		if (lpShellCode)
		{
			memcpy(ShellCodeBuf + 128, lpShellCode, uCodeSize);
			lpCurBufAdress = push_xx(lpCurBufAdress, (DWORD)lpCurESPAddress + 128); // push
			lpCurBufAdress = Call_xx(lpCurBufAdress, dwFuncAdress, (DWORD)lpCurESPAddress + (DWORD)lpCurBufAdress - (DWORD)&ShellCodeBuf); //Call
		}

		lpCurBufAdress = mov_eax_xx(lpCurBufAdress, Context.Eax);
		lpCurBufAdress = mov_ebx_xx(lpCurBufAdress, Context.Ebx);
		lpCurBufAdress = mov_ecx_xx(lpCurBufAdress, Context.Ecx);
		lpCurBufAdress = mov_edx_xx(lpCurBufAdress, Context.Edx);
		lpCurBufAdress = mov_esi_xx(lpCurBufAdress, Context.Esi);
		lpCurBufAdress = mov_edi_xx(lpCurBufAdress, Context.Edi);
		lpCurBufAdress = mov_ebp_xx(lpCurBufAdress, Context.Ebp);
		lpCurBufAdress = mov_esp_xx(lpCurBufAdress, Context.Esp);
		lpCurBufAdress = mov_eip_xx(lpCurBufAdress, Context.Eip, (DWORD)lpCurESPAddress + (DWORD)lpCurBufAdress - (DWORD)&ShellCodeBuf);
		Context.Esp = (DWORD)(lpCurESPAddress - 4);
		Context.Eip = (DWORD)lpCurESPAddress;

		if (VirtualProtectEx(hProcess, lpCurESPAddress, 0x480, PAGE_EXECUTE_READWRITE, &flOldProtect)
			&& WriteProcessMemory(hProcess, lpCurESPAddress, &ShellCodeBuf, 0x480, &NumberOfBytesWritten)
			&& FlushInstructionCache(hProcess, lpCurESPAddress, 0x480)
			&& SetThreadContext(hThread, &Context))
		{
			bResult = TRUE;
		}

	}

	// 回复线程
	ResumeThread(hThread);

	return TRUE;
}

DWORD GetFuncAdress()
{
	return (DWORD)GetProcAddress(GetModuleHandleA("Kernel32"), "LoadLibraryW");
}

BOOL WINAPI CreateProcessWithDllW(LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation,
	LPWSTR lpDllFullPath,
	Proc_CreateProcessW FuncAdress
	)
{
	BOOL bResult = FALSE;
	size_t uCodeSize = 0;
	DWORD dwCreaFlags;
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	if (FuncAdress == NULL)
	{
		FuncAdress = CreateProcessW;
	}


	// 设置创建就挂起进程
	dwCreaFlags = dwCreationFlags | CREATE_SUSPENDED;
	if (CreateProcessW(lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreaFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		&pi
		))
	{
		if (lpDllFullPath)
			uCodeSize = 2 * wcslen(lpDllFullPath) + 2;
		else
			uCodeSize = 0;

		// 得到LoadLibraryW 的地址
		DWORD dwLoadDllProc = GetFuncAdress();

		// 挂起线程 写入Shellcode
		if (SuspendTidAndInjectCode(pi.hProcess, pi.hThread, dwLoadDllProc, (BYTE*)lpDllFullPath, uCodeSize))
		{
			if (lpProcessInformation)
				memcpy(lpProcessInformation, &pi, sizeof(PROCESS_INFORMATION));

			if (!(dwCreationFlags & CREATE_SUSPENDED))
				ResumeThread(pi.hThread);

			bResult = TRUE;
		}
	}

	return bResult;
}


void Init()
{
	WCHAR* wszPath = L"info.exe";
	WCHAR* wszDll = L"kiminago_chs.dll";

	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (FALSE == CreateProcessWithDllW(NULL, L"Shinku.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi, L"FVPUnicodeModule.dll", NULL))
	{
		MessageBoxW(NULL, L"False", NULL, MB_OK);
	}
	else
	{
		MessageBoxW(NULL, L"Ok", NULL, MB_OK);
	}

}


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

	PWCHAR pszDllPath = L"FVPUnicodeModule.dll";

	WCHAR szDllPath[1024] = { 0 };
	CHAR mDllPath[1024] = { 0 };
	PWCHAR pszFilePart = NULL;

	if (!GetFullPathNameW(pszDllPath, ARRAYSIZE(szDllPath), szDllPath, &pszFilePart))
	{
		MessageBoxW(NULL, L"内部错误", L"游戏启动失败", MB_OK);
		return false;
	}

	HMODULE hDll = LoadLibraryEx(pszDllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
	if (hDll == NULL)
	{
		MessageBoxW(NULL, L"无法加载修复文件", L"游戏启动失败", MB_OK);
		return false;
	}

	ExportContext ec;
	ec.fHasOrdinal1 = FALSE;
	ec.nExports = 0;
	DetourEnumerateExports(hDll, &ec, ExportCallback);
	FreeLibrary(hDll);
	
	if (!ec.fHasOrdinal1)
	{
		MessageBoxW(NULL, L"Dll文件验证失败", L"游戏启动失败", MB_OK);
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
	lstrcpyW(szExe, L"Shinku.exe");
	lstrcpyW(szCommand, L"Shinku.exe");
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

