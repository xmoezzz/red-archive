#include "ShinkuHook.h"
#include <tlhelp32.h>
#include <Psapi.h>
#include <string>

using std::wstring;

wstring GetFileName(LPCWSTR lpFileName)
{
	wstring FileName(lpFileName);
	wstring::size_type pos;

	pos = FileName.find_last_of(L"\\");
	if (pos != wstring::npos)
		return FileName.substr(pos + 1, wstring::npos);
	else
		return FileName;
}


BOOL CALLBACK EnumChildWindowCallBack(HWND hWnd, LPARAM lParam)
{
	DWORD dwPid = 0;
	GetWindowThreadProcessId(hWnd, &dwPid); 
	if (dwPid == lParam) 
	{
		EnumChildWindows(hWnd, EnumChildWindowCallBack, lParam);    // 递归查找子窗口   
	}
	return  TRUE;
}

BOOL CALLBACK EnumWindowCallBack(HWND hWnd, LPARAM lParam)
{
	DWORD dwPid = 0;
	GetWindowThreadProcessId(hWnd, &dwPid); // 获得找到窗口所属的进程   
	if (dwPid == lParam) // 判断是否是目标进程的窗口   
	{
		DestroyWindow(hWnd);
		//EnumChildWindows(hWnd, EnumChildWindowCallBack, lParam);    // 继续查找子窗口   
	}
	return  TRUE;
}

DWORD NTAPI LaunchErrorInfo()
{
	BOOLEAN  bl;
	ULONG    Response;

	EnumWindows(EnumWindowCallBack, GetCurrentProcessId());
	RtlAdjustPrivilege(19, TRUE, FALSE, &bl);
	NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, NULL, 6, &Response); 
	MessageBoxW(NULL, L"Unsupported Environment", L"X'moe-CoreLib", MB_OK | MB_ICONERROR);
	Ps::ExitProcess(0x23333333);
}

DWORD NTAPI AntiLiveWorker(LPVOID)
{
	HANDLE          procSnap;
	PROCESSENTRY32W procEntry;
	BOOL            bRet;
	HANDLE          hProcess;
	BOOL            Wow64Process;
	DWORD           cbNeeded;
	HMODULE         hMods[1000];
	WCHAR           ModName[MAX_PATH];
	HWND            hWnd;
	wstring         FileName;

	LOOP_FOREVER
	{
		hWnd = FindWindowW(L"哔哩哔哩直播姬", NULL);
		if (hWnd)
		{
			LaunchErrorInfo();
		}

		//LiveEncoder.dll
		hWnd = FindWindowW(L"直播伴侣", NULL);
		if (hWnd)
		{
			GetWindowThreadProcessId(hWnd, &cbNeeded);
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, cbNeeded);
			cbNeeded = 0;
			EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_32BIT);

			for (UINT i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
			{
				RtlZeroMemory(ModName, countof(ModName) * sizeof(ModName[0]));
				GetModuleFileNameEx(hProcess, hMods[i], ModName, countof(ModName));
				FileName = GetFileName(ModName);
				if (!StrICompareW(FileName.c_str(), L"LiveEncoder.dll", StrCmp_ToLower))
				{
					LaunchErrorInfo();
				}
			}
			CloseHandle(hProcess);
		}

		//ZhanqiLiveTool.dll
		hWnd = FindWindowW(L"战旗主播工具登陆窗口", NULL);
		if (hWnd)
		{
			cbNeeded = 0;
			EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_32BIT);

			for (UINT i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
			{
				RtlZeroMemory(ModName, countof(ModName) * sizeof(ModName[0]));
				GetModuleFileNameEx(hProcess, hMods[i], ModName, countof(ModName));
				FileName = GetFileName(ModName);
				if (!StrICompareW(FileName.c_str(), L"ZhanqiLiveTool.dll", StrCmp_ToLower))
				{
					LaunchErrorInfo();
				}
			}
			CloseHandle(hProcess);
		}

		hWnd = FindWindowW(NULL, L"OBSWindowClass");
		if (hWnd)
		{
			//yyvip.dll
			hWnd = FindWindowW(L"虎牙YY", NULL);
			cbNeeded = 0;
			EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_32BIT);

			for (UINT i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
			{
				RtlZeroMemory(ModName, countof(ModName) * sizeof(ModName[0]));
				GetModuleFileNameEx(hProcess, hMods[i], ModName, countof(ModName));
				FileName = GetFileName(ModName);
				if (!StrICompareW(FileName.c_str(), L"yyvip.dll", StrCmp_ToLower))
				{
					LaunchErrorInfo();
				}
			}
			CloseHandle(hProcess);
		}

		hWnd = FindWindowW(L"熊猫TV直播助手", NULL);
		if (hWnd)
		{
			LaunchErrorInfo();
		}

		hWnd = FindWindowW(L"XSplit Gamecaster", NULL);
		if (hWnd)
		{
			LaunchErrorInfo();
		}

		//wire.dll
		hWnd = FindWindowW(NULL, L"evaSplitterBarClass");
		if (hWnd)
		{
			cbNeeded = 0;
			EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_32BIT);

			for (UINT i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
			{
				RtlZeroMemory(ModName, countof(ModName) * sizeof(ModName[0]));
				GetModuleFileNameEx(hProcess, hMods[i], ModName, countof(ModName));
				FileName = GetFileName(ModName);
				if (!StrICompareW(FileName.c_str(), L"wire.dll", StrCmp_ToLower))
				{
					LaunchErrorInfo();
				}
			}
			CloseHandle(hProcess);
		}

		//Telestream, LLC - Online Store
		hWnd = FindWindowW(NULL, L"Telestream, LLC - Online Store");
		if (hWnd)
		{
			LaunchErrorInfo();
		}
		Ps::Sleep(20);
	}
}


NTSTATUS ShinkuHook::LaunchAntiLiveWorker()
{
	DWORD  ThreadId;
	HANDLE Handle;

	Handle = CreateThread(NULL, NULL, AntiLiveWorker, NULL, NULL, &ThreadId);

	if (Handle == INVALID_HANDLE_VALUE)
		return STATUS_UNSUCCESSFUL;
	else
		return STATUS_SUCCESS;
}
