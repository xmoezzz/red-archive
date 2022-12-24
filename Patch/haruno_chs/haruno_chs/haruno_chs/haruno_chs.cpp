// haruno_chs.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "haruno_chs.h"
#include "CCImage.h"
#include <windows.h>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Advapi32.lib")
//gdi32.lib
//Advapi32.lib



#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名


#include <Windows.h>
#include "CRC64.h"

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


BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(
		NULL,            // lookup privilege on local system   
		lpszPrivilege,   // privilege to lookup    
		&luid))        // receives LUID of privilege   
	{
		//printf("LookupPrivilegeValue error: %u\n", GetLastError() );   
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.   

	if (!AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		//printf("AdjustTokenPrivileges error: %u\n", GetLastError() );   
		return FALSE;
	}

	return TRUE;
}



CCImage image;
COLORREF rgb;
HWND ghWnd;
HBITMAP hBitmap;
int bitheight;
int bitwidth;


BOOL ImageFromIDResource()
{
	UINT nID = IDB_PNG1;
	LPCTSTR sTR = _T("PNG");

	//UINT nID = IDB_BITMAP1;
	//LPCTSTR sTR = RT_BITMAP;

	HINSTANCE hInst = GetModuleHandle(NULL);
	HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nID), sTR); // type
	if (!hRsrc)
		return FALSE;

	// load resource into memory
	DWORD len = SizeofResource(hInst, hRsrc);
	BYTE* lpRsrc = (BYTE*)LoadResource(hInst, hRsrc);
	if (!lpRsrc)
		return FALSE;

	// Allocate global memory on which to create stream
	HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, len);
	BYTE* pmem = (BYTE*)GlobalLock(m_hMem);
	memcpy(pmem, lpRsrc, len);
	IStream* pstm;
	CreateStreamOnHGlobal(m_hMem, FALSE, &pstm);

	// load from stream
	image.Load(pstm, true);
	// free/release stuff
	GlobalUnlock(m_hMem);
	pstm->Release();
	FreeResource(lpRsrc);
	return TRUE;
}


void DrawBmp(HDC hDC, HBITMAP bitmap, int nWidth, int nHeight)
{
	//BITMAP            bm;
	//HDC hdcImage;
	//HDC hdcMEM;
	//hdcMEM = CreateCompatibleDC(hDC);
	//hdcImage = CreateCompatibleDC(hDC);
	//HBITMAP bmp = ::CreateCompatibleBitmap(hDC, nWidth, nHeight);
	//GetObject(bitmap, sizeof(bm), (LPSTR)&bm);
	//SelectObject(hdcMEM, bmp);
	//SelectObject(hdcImage, bitmap);
	//StretchBlt(hdcMEM, 0, 0, nWidth, nHeight, hdcImage, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	//StretchBlt(hDC, 0, 0, nWidth, nHeight, hdcMEM, 0, 0, nWidth, nHeight, SRCCOPY);
	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.bottom = image.GetHeight();
	rect.right = image.GetWidth();
	image.Draw(hDC, rect);

	//DeleteObject(hdcImage);
	//DeleteDC(hdcImage);
	//DeleteDC(hdcMEM);
}

int CALLBACK TimerProc()
{
	static int wndAlp = 0;
	static int flag = 0;

	if (flag)
	{
		if (flag == 1)
		{
			Sleep(200);
			flag = 2;
		}
		wndAlp -= 3;
		if (wndAlp == 0)
			DestroyWindow(ghWnd);
		SetLayeredWindowAttributes(ghWnd, -1, wndAlp, LWA_ALPHA);
	}
	else
	{
		wndAlp += 5;
		if (wndAlp == 255)
			flag = 1;
		SetLayeredWindowAttributes(ghWnd, -1, wndAlp, LWA_ALPHA);
	}
	return 0;
}

LRESULT CALLBACK WindowProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	static HDC compDC = 0;
	static RECT rect;
	if (uMsg == WM_CREATE)
	{
		ghWnd = hwnd;
		SetLayeredWindowAttributes(hwnd, -1, 0, LWA_ALPHA);
		SetTimer(hwnd, 1003, 1, (TIMERPROC)TimerProc);

		int scrWidth, scrHeight;

		scrWidth = GetSystemMetrics(SM_CXSCREEN);
		scrHeight = GetSystemMetrics(SM_CYSCREEN);
		GetWindowRect(hwnd, &rect);
		rect.left = (scrWidth - rect.right) / 2;
		rect.top = (scrHeight - rect.bottom) / 2;
		SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);

		DrawBmp(GetDC(hwnd), hBitmap, 448, 216);
	}
	if (uMsg == WM_PAINT)
	{
		RECT rect;
		GetWindowRect(hwnd, &rect);

	}
	if (uMsg == WM_CLOSE)
	{
		DestroyWindow(hwnd);
	}
	if (uMsg == WM_DESTROY)
	{
		PostQuitMessage(0);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI Init(LPVOID lpParam)
{
	DWORD routeAddr;
	DWORD oldProtect;
	ImageFromIDResource();
	do
	{
		//if(tree)
		//hBitmap = ::LoadBitmapW(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1));
		//hBitmap = (HBITMAP)LoadImageA(NULL, "X'moe_eyecatch_loge.bmp",IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_DEFAULTSIZE|LR_LOADFROMFILE);
		WNDCLASSEXA wcex;
		memset(&wcex, 0, sizeof(wcex));
		wcex.cbSize = sizeof(wcex);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WindowProc;
		wcex.hInstance = hInst;
		wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcex.lpszClassName = "XmoeLogo";
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.cbWndExtra = DLGWINDOWEXTRA;
		wcex.hbrBackground = (HBRUSH)NULL;
		rgb = 0xFFFFFFFF;
		RegisterClassExA(&wcex);
		HWND hWnd = CreateWindowExA(WS_EX_LAYERED | WS_EX_TOPMOST, "XmoeLogo", "XmoeLogo", WS_POPUP /*| WS_SYSMENU | WS_SIZEBOX*/, 0, 0, image.GetWidth(), image.GetHeight(), NULL, NULL, hInst, NULL);
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	} while (0);

	return 0;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	Init(NULL);
	/*
	DWORD threadID;
	HANDLE hThread;
	hThread = CreateThread(NULL, 0, Init, NULL, 0, &threadID);
	*/

	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	WCHAR wszPath[] = L"haruno.exe";
	WCHAR wszDll[] = L"xmoe_haruno_chs.dll";

	FILE *hDll = NULL;
	FILE *hExe = NULL;


	bool Check = true;
	do
	{
		hExe = _wfopen(wszPath, L"rb");
		if (hExe == NULL)
		{
			MessageBoxW(NULL, L"无法找到游戏原始程序haruno.exe", L"Error", MB_OK);
			Check = false;
			break;
		}
		hDll = _wfopen(wszDll, L"rb");
		if (hDll == NULL)
		{
			MessageBoxW(NULL, L"QAQ无法找到汉化文件xmoe_haruno_chs.dll", L"Error", MB_OK);
			Check = false;
			break;
		}
	} while (false);

	if (!Check)
		return -1;
	fclose(hDll);

	rewind(hExe);
	fseek(hExe, 0, SEEK_END);
	unsigned long size = ftell(hExe);
	rewind(hExe);
	unsigned char* Buffer = new unsigned char[size];
	fread(Buffer, 1, size, hExe);
	fclose(hExe);

	Large pInfo;
	pInfo.I64 = crc64(0, Buffer, size);
	delete[] Buffer;

	if (pInfo.I32[0] != 0xc7507650 || pInfo.I32[1] != 0xbfa42041)
	{
		return 0;
	}


	ZeroMemory(&si, sizeof(si)); 
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	BOOL ret = CreateProcessWithDllW(NULL, wszPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi, wszDll, NULL);
	if (ret == FALSE)
	{
		MessageBoxW(NULL, L"不能启动游戏QwQ。。\n", L"Error", MB_OK);
		return 0;
	}

	return 0;

}

