#include "stdafx.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <cctype>
#include "tp_stub.h"
#include "Message.h"

using std::vector;
using std::wstring;

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "ntdll.lib")

static CHAR* AnthString = "XmoeSTmoe>_<";

BOOL WINAPI XmoeMakeSure(const CHAR* lpAnthString)
{
	if (RtlCompareMemory(lpAnthString, AnthString, lstrlenA(AnthString)) == lstrlenA(AnthString))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL WINAPI AnthFirst()
{
	HMODULE hModule = GetModuleHandleW(L"xmoe_sabbat_chs.dll");
	if (hModule == NULL)
	{
		ExitProcess(0);
		return FALSE;
	}
	else
	{
		if (GetModuleHandleW(L"KrkrExtract.dll"))
		{
			ExitProcess(0);
		}
		return TRUE;
	}
}

VOID WINAPI Init()
{
	AnthFirst();

	//tmd checked
	/*
	STARTUPINFOW si;
	if ((si.dwX != 0) || (si.dwY != 0) || (si.dwXCountChars != 0) || (si.dwYCountChars != 0) ||
		(si.dwFillAttribute != 0) || (si.dwXSize != 0) || (si.dwYSize != 0) ||
		(si.dwFlags & STARTF_FORCEOFFFEEDBACK))
	{
		ExitProcess(-1);
	}
	*/

	DWORD ThisPid = GetCurrentProcessId();
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ThisPid);

	vector<wstring> ModuleList;
	MODULEENTRY32W me32 = {0};
	me32.dwSize = sizeof(MODULEENTRY32W);
	if (!Module32FirstW(hProcessSnap, &me32))
	{
	}
	do
	{
		ModuleList.push_back(me32.szExePath);
	}
	while (Module32NextW(hProcessSnap, &me32));

	for (auto it : ModuleList)
	{
		wstring LowerCaseString;
		for (int i = 0; i < it.length(); i++)
		{
			if (it[i] >= 0 && it[i] <= 0x7F)
			{
				UCHAR NarrowChar = (UCHAR)it[i];
				if (isalpha(NarrowChar))
				{
					if (isupper(NarrowChar))
					{
						WCHAR WideChar = (WCHAR)tolower(NarrowChar);
						LowerCaseString += WideChar;
					}
					else
					{
						LowerCaseString += it[i];
					}
				}
				else
				{
					LowerCaseString += it[i];
				}
			}
			else
			{
				LowerCaseString += it[i];
			}
		}
		//Translation ends here

		if (wcsstr(LowerCaseString.c_str(), L"krkrextract") ||
			wcsstr(LowerCaseString.c_str(), L"krkrviewer"))
		{
			ExitProcess(0);
		}
	}
}

VOID WINAPI UnInit()
{
	if (hThread != INVALID_HANDLE_VALUE )
	{
		PostThreadMessageW(Threadid, XMOE_TERM_CHECKER, NULL, NULL);
	}
	ULONG ThreadInfo = 0;
	WaitForSingleObject(hThread, 200);
	GetExitCodeThread(hThread, &ThreadInfo);
	if (ThreadInfo == STILL_ACTIVE)
	{
		TerminateThread(hThread, -1);
	}
}



HANDLE hThread = INVALID_HANDLE_VALUE;
ULONG  Threadid = 0;


ULONG NTAPI CheckWorker(LPVOID lpParam)
{
	MSG msg;
	while (GetMessageW(&msg, NULL, NULL, NULL))
	{
		GetMessageW(&msg, NULL, NULL, NULL);
		TranslateMessage(&msg);

		if (GetModuleHandleW(L"KrkrExtract.dll"))
			ExitProcess(-1);

		Sleep(200);
	}
}

HRESULT WINAPI SetupThreadChecker()
{
	
}


extern "C" __declspec(dllexport) HRESULT _stdcall V2Link(iTVPFunctionExporter *exporter)
{
	TVPInitImportStub(exporter);

	//tmd checked
	/*
	STARTUPINFOW si;
	GetStartupInfoW(&si);

	if ((si.dwX != 0) || (si.dwY != 0) || (si.dwXCountChars != 0) || (si.dwYCountChars != 0) ||
		(si.dwFillAttribute != 0) || (si.dwXSize != 0) || (si.dwYSize != 0) ||
		(si.dwFlags & STARTF_FORCEOFFFEEDBACK))
	{
		ExitProcess(-1);
	}
	*/

	return S_OK;
}
//---------------------------------------------------------------------------
extern "C" __declspec(dllexport) HRESULT _stdcall V2Unlink()
{
	TVPUninitImportStub();
	return S_OK;
}
