// WhiteEternityCHS.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "WhiteEternityCHS.h"
#include <Mmsystem.h>
#include "SDL\SDL.h"

#include "detours.h"

#pragma comment(lib,"SDL2main.lib")
#pragma comment(lib,"SDL2.lib")
#pragma comment(lib,"Advapi32.lib")
#pragma comment(lib,"Detours.lib")


#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
/*
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
*/



#include "stdafx.h"
#include <Windows.h>

int ThreadProc(void *pBuffer);

DWORD WINAPI ThreadProcWin32(void *pBuffer);




// 函数声明
typedef BOOL (WINAPI* Proc_CreateProcessW)(LPCWSTR lpApplicationName,
                       LPWSTR lpCommandLine,
                       LPSECURITY_ATTRIBUTES lpProcessAttributes,
                       LPSECURITY_ATTRIBUTES lpThreadAttributes,
                       BOOL bInheritHandles,
                       DWORD dwCreationFlags,
                       LPVOID lpEnvironment,
                       LPCWSTR lpCurrentDirectory,
                       LPSTARTUPINFOW lpStartupInfo,
                       LPPROCESS_INFORMATION lpProcessInformation);

typedef HMODULE (WINAPI* Func_LoadLibraryW)(LPCWSTR lpLibFileName);


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
  if ( !newEip )
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
  if ( !newEip )
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

  memset(&Context,0,sizeof(Context));
  Context.ContextFlags = CONTEXT_FULL;

  if ( GetThreadContext(hThread, &Context))
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
    
    if ( VirtualProtectEx(hProcess, lpCurESPAddress, 0x480, PAGE_EXECUTE_READWRITE, &flOldProtect)
      && WriteProcessMemory(hProcess, lpCurESPAddress, &ShellCodeBuf, 0x480, &NumberOfBytesWritten)
      && FlushInstructionCache(hProcess, lpCurESPAddress, 0x480)
      && SetThreadContext(hThread, &Context) )
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

BOOL WINAPI CreateProcessWithDllW(  LPCWSTR lpApplicationName,
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

  //SDL_CreateThread(ThreadProc, NULL, NULL);

  BOOL bResult = FALSE;
  size_t uCodeSize = 0;
  DWORD dwCreaFlags;
  PROCESS_INFORMATION pi;
  ZeroMemory( &pi, sizeof(pi) );

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
    if ( lpDllFullPath )
      uCodeSize = 2 * wcslen(lpDllFullPath) + 2;
    else
      uCodeSize = 0;

    // 得到LoadLibraryW 的地址
    DWORD dwLoadDllProc = GetFuncAdress();

    // 挂起线程 写入Shellcode
    if (SuspendTidAndInjectCode(pi.hProcess, pi.hThread, dwLoadDllProc, (BYTE*)lpDllFullPath, uCodeSize))
    {
      if ( lpProcessInformation )
        memcpy(lpProcessInformation, &pi, sizeof(PROCESS_INFORMATION));

      if ( !(dwCreationFlags & CREATE_SUSPENDED) )
        ResumeThread(pi.hThread);

      bResult = TRUE;
    }
  }

  return bResult;
}


SDL_Renderer *ren = NULL;
SDL_Window   *win = NULL;
SDL_Surface *sur = NULL;
SDL_Texture *tex = NULL;

DWORD WINAPI ThreadProcWin32(void *pBuffer)
{

	sur = SDL_LoadBMP("whiteEternity.chs");
	if(sur == NULL)
	{
		//MessageBoxW(NULL, L"Failed to load", L"xxx", MB_OK);
		return 0;
	}

	char tmp[100];
	sprintf(tmp, "%s", SDL_GetError());
	MessageBoxA(NULL, tmp, "xxx", MB_OK);
	win = SDL_CreateWindow(NULL, 
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		sur->w,
		sur->h,
		SDL_WINDOW_BORDERLESS);

	if(win == 0)
	{
		//MessageBoxW(NULL, L"Failed to Create Windows", L"xxx", MB_OK);
		return 0;
	}

	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	
	if(ren == 0)
	{
		//MessageBoxW(NULL, L"Failed to Create Render", L"xxx", MB_OK);
		return 0;
	}

	tex = SDL_CreateTextureFromSurface(ren, sur);
	//SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	SDL_Rect rect;
	SDL_QueryTexture(tex, NULL, NULL, &(rect.w), &(rect.h));
	rect.x = 0;
	rect.y = 0;


	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
	SDL_RenderClear(ren);

	SDL_SetRenderDrawColor(ren, 0, 0, 0, 0xff);
	SDL_RenderCopy(ren, tex, NULL, &rect);
	SDL_RenderPresent(ren);
	Sleep(5000);
	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(ren);
	
	return 0;
}


int ThreadProc(void *pBuffer)
{

	sur = SDL_LoadBMP("whiteEternity.chs");
	if(sur == NULL)
	{
		//MessageBoxW(NULL, L"Failed to load", L"xxx", MB_OK);
		return 0;
	}
	win = SDL_CreateWindow(NULL, 
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		sur->w,
		sur->h,
		SDL_WINDOW_BORDERLESS);

	if(win == 0)
	{
		//MessageBoxW(NULL, L"Failed to Create Windows", L"xxx", MB_OK);
		return 0;
	}

	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	
	if(ren == 0)
	{
		//MessageBoxW(NULL, L"Failed to Create Render", L"xxx", MB_OK);
		return 0;
	}

	tex = SDL_CreateTextureFromSurface(ren, sur);
	//SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	SDL_Rect rect;
	SDL_QueryTexture(tex, NULL, NULL, &(rect.w), &(rect.h));
	rect.x = 0;
	rect.y = 0;


	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
	SDL_RenderClear(ren);

	SDL_SetRenderDrawColor(ren, 0, 0, 0, 0xff);
	SDL_RenderCopy(ren, tex, NULL, &rect);
	SDL_RenderPresent(ren);
	Sleep(5000);
	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(ren);
	
	return 0;
}

BOOL SetPrivilege(HANDLE hToken,LPCTSTR lpszPrivilege,BOOL bEnablePrivilege)   
{  
    TOKEN_PRIVILEGES tp;  
    LUID luid;  
  
    if ( !LookupPrivilegeValue(   
        NULL,            // lookup privilege on local system   
        lpszPrivilege,   // privilege to lookup    
        &luid ) )        // receives LUID of privilege   
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
  
    if ( !AdjustTokenPrivileges(  
        hToken,   
        FALSE,   
        &tp,   
        sizeof(TOKEN_PRIVILEGES),   
        (PTOKEN_PRIVILEGES) NULL,   
        (PDWORD) NULL) )  
    {   
        //printf("AdjustTokenPrivileges error: %u\n", GetLastError() );   
        return FALSE;   
    }

	return TRUE;
}

BOOL __stdcall InjectDLL(char *Cmdstr)
{
	BOOL isWINXP = FALSE;
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize=sizeof(os);
	if(!GetVersionEx((OSVERSIONINFO *)&os))
	{
		return FALSE;
	}
	switch(os.dwMajorVersion)
	{
		case 5:
			isWINXP = TRUE;
		break;

		default:
		break;
	}


	//SDL_CreateThread(ThreadProc, NULL, NULL);

	DWORD threadID;
	HANDLE hHandle = CreateThread(NULL,0,ThreadProcWin32,NULL,0,&threadID);

	//SDL_Delay(5000);

	char* szDllPath = "xmoe_chs.dll";
	HANDLE hProcess = NULL;
	HMODULE hMod = NULL;
	LPVOID  pBuf = NULL;

	FILE *fin = fopen(szDllPath, "rb");
	if(!fin)
	{
		MessageBoxW(NULL, L"缺少[xmoe_chs.dll]文件，游戏无法进行\n", L"WhiteEternityCHS", MB_OK);
		ExitProcess(0);
	}


	//BOOL Res = DetourCreateProcessWithDllA()
	
	STARTUPINFOA si = {sizeof(si)};
	//si.lpTitle = "Debug";
	PROCESS_INFORMATION pi;

	//DetourCreateProcessWithDllExA(NULL, Cmdstr, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi, );
	
	BOOL CreRet = CreateProcessA(NULL, Cmdstr, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
	if(CreRet == FALSE)
	{
		MessageBoxW(NULL, L"进程创建失败，游戏无法进行\n", L"WhiteEternityCHS", MB_OK);
		//CloseHandle(hProcess);
		ExitProcess(0);
	}
	//查询pID 
	//SuspendThread(pi.hThread);

	if(isWINXP == TRUE)
	{
		HANDLE hToken;  
		BOOL bRet = OpenProcessToken(GetCurrentProcess(),TOKEN_ALL_ACCESS,&hToken);  
		BOOL PriRet = SetPrivilege(hToken,SE_DEBUG_NAME,TRUE);
		if(PriRet == FALSE)
		{
			MessageBoxW(NULL, L"提升权限失败\n你现在使用的是WinXP系统，\n更换高版本的系统可以分分钟解决此问题\n", L"WhiteEternityCHS", MB_OK);
			CloseHandle(hProcess);
			ExitProcess(-1);
		}
	}

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);
	 
	if(hProcess == NULL)
	{
		char szMsg[MAX_PATH];
		sprintf(szMsg, "Failed to launch game![0x%08x]", GetLastError());
		MessageBoxA(NULL, szMsg,"WhiteEternityCHS",NULL);
		return FALSE;
	}
	DWORD dwBufSize = (DWORD)((strlen("xmoe_chs.dll") + 1) * sizeof(char));
	
	pBuf = VirtualAllocEx(hProcess, NULL, dwBufSize, MEM_COMMIT, PAGE_READWRITE);
	BOOL WriteRet = WriteProcessMemory(hProcess, pBuf, (LPVOID)szDllPath, dwBufSize, NULL);
	if(WriteRet == FALSE)
	{
		MessageBoxW(NULL, L"无法写入进程\n请关闭和杀毒有关的软件", L"WhiteEternityCHS", MB_OK);
		CloseHandle(hProcess);
		ExitProcess(-1);
	}


	//HMODULE hMod = NULL;
	hMod = GetModuleHandleA("kernel32.dll");
	HANDLE hThread = NULL;
	LPTHREAD_START_ROUTINE pThreadProc;

	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "LoadLibraryA");
	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, pBuf, 0, NULL);
	if(!hThread)
	{
		wchar_t msg[MAX_PATH*2];
		wmemset(msg, 0, MAX_PATH*2/sizeof(wchar_t));
		wsprintf(msg, L"线程创建失败，游戏将无法进行\n[%08x]", GetLastError());
		MessageBoxW(NULL, msg, L"WhiteEternityCHS", MB_OK);
		CloseHandle(hThread);
		CloseHandle(hProcess);
		ExitProcess(-1);
	}


	//Fix: Crash
	ResumeThread(pi.hThread);
	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);
	

	//ResumeThread(pi.hThread);
	

	return TRUE;
}

COLORREF rgb;
HWND ghWnd;
HBITMAP hBitmap;
int bitheight;
int bitwidth;

void DrawBmp(HDC hDC, HBITMAP bitmap,int nWidth,int nHeight)
{
    BITMAP            bm;
    HDC hdcImage;
    HDC hdcMEM;
    hdcMEM = CreateCompatibleDC(hDC);
    hdcImage = CreateCompatibleDC(hDC);
    HBITMAP bmp = ::CreateCompatibleBitmap(hDC, nWidth, nHeight);
    GetObject(bitmap, sizeof(bm),(LPSTR)&bm);
    SelectObject(hdcMEM, bmp);
    SelectObject(hdcImage, bitmap);
    StretchBlt(hdcMEM, 0, 0, nWidth, nHeight, hdcImage, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    StretchBlt(hDC, 0, 0, nWidth, nHeight, hdcMEM, 0, 0, nWidth, nHeight, SRCCOPY);

    DeleteObject(hdcImage);
    DeleteDC(hdcImage);
    DeleteDC(hdcMEM);
}

int CALLBACK TimerProc()
{
    static int wndAlp = 0;
    static int flag = 0;

    if(flag)
    {
        if(flag == 1)
        {
            Sleep(1000);
            flag = 2;
        }
        wndAlp-=3;
        if(wndAlp==0)
            DestroyWindow(ghWnd);
        SetLayeredWindowAttributes(ghWnd,-1,wndAlp,LWA_ALPHA);
    }
    else
    {
        wndAlp+=5;
        if(wndAlp==255)
            flag = 1;
        SetLayeredWindowAttributes(ghWnd,-1,wndAlp,LWA_ALPHA);
    }
    return 0;
}

LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    static HDC compDC=0;
    static RECT rect;
    if(uMsg == WM_CREATE)
    {
        ghWnd = hwnd;
        SetLayeredWindowAttributes(hwnd,-1,0,LWA_ALPHA);
        SetTimer(hwnd,1003,1,(TIMERPROC)TimerProc);

        int scrWidth,scrHeight;

        scrWidth = GetSystemMetrics(SM_CXSCREEN);
        scrHeight = GetSystemMetrics(SM_CYSCREEN);
        GetWindowRect(hwnd,&rect);
        rect.left = (scrWidth-rect.right)/2;
        rect.top = (scrHeight-rect.bottom)/2;
        SetWindowPos(hwnd,HWND_TOP,rect.left,rect.top,rect.right,rect.bottom,SWP_SHOWWINDOW);

        DrawBmp(GetDC(hwnd),hBitmap,703,252);
    }
    if(uMsg == WM_PAINT)
    {
        RECT rect;
        GetWindowRect(hwnd,&rect);

    }
    if(uMsg==WM_CLOSE)
    {
        DestroyWindow(hwnd);
    }
    if(uMsg == WM_DESTROY)
    {
        PostQuitMessage(0);
    }
    return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

void __stdcall Init()
{
    char szFilePath[MAX_PATH];
    DWORD routeAddr;
    DWORD oldProtect;

	/*
	bool imgFile = false;
	FILE* img = NULL;
	img = fopen("whiteEternity.chs", "rb");
	if(img)
		imgFile = true;
	fclose(img);

    if(imgFile)
    {
        //if(tree)
        //{
			GetCurrentDirectoryA(sizeof(szFilePath),szFilePath);
			strcat(szFilePath,"\\whiteEternity.chs");
            //char szTempPath[MAX_PATH];
            char szTempFile[MAX_PATH];
			//sprintf(szTempFile, "%s", "whiteEternity.chs");


            hBitmap = (HBITMAP)LoadImageA(NULL,szFilePath,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_DEFAULTSIZE|LR_LOADFROMFILE);
       // }
	   */
		
		hBitmap=::LoadBitmapW(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1));
        WNDCLASSEXA wcex;
        memset(&wcex,0,sizeof(wcex));
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW ;
        wcex.lpfnWndProc = WindowProc;
        wcex.hInstance = hInst;
        wcex.hbrBackground    = (HBRUSH)(COLOR_BTNFACE+1);
        wcex.lpszClassName    = "XmoeLogo";
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.cbWndExtra = DLGWINDOWEXTRA;
        rgb = 0xFFFFFFFF;
        RegisterClassExA(&wcex);
        HWND hWnd = CreateWindowExA(WS_EX_LAYERED|WS_EX_TOPMOST,"XmoeLogo","XmoeLogo",WS_POPUP /*| WS_SYSMENU | WS_SIZEBOX*/ ,0, 0, 703, 252, NULL, NULL, hInst, NULL);
        ShowWindow(hWnd,SW_SHOW);
        UpdateWindow(hWnd);
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    //}

}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//SDL_CreateThread(ThreadProc, NULL, NULL);

	//DWORD threadID;
	//HANDLE hHandle = CreateThread(NULL,0,ThreadProcWin32,NULL,0,&threadID);

	Init();

 	// TODO: 在此放置代码。
	FILE *fin = NULL;
	fin = fopen("WhiteEternity.exe", "rb");
	if(fin == NULL)
	{
		MessageBoxA(NULL, "Cannot find WhiteEternity.exe", "WhiteEternityCHS", MB_OK);
		fclose(fin);
		return -1;
	}
	fclose(fin);
	fin = NULL;
	fin = fopen("xmoe_chs.dll", "rb");
	if(fin == NULL)
	{
		MessageBoxA(NULL, "Cannot find xmoe_chs.dll", "WhiteEternityCHS", MB_OK);
		fclose(fin);
		return -1;
	}
	fclose(fin);
	//InjectDLL("WhiteEternity.exe");

	/*
	BOOL isWINXP = FALSE;
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize=sizeof(os);
	if(!GetVersionEx((OSVERSIONINFO *)&os))
	{
		return FALSE;
	}
	switch(os.dwMajorVersion)
	{
		case 5:
			isWINXP = TRUE;
		break;

		default:
		break;
	}
	*/


	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	WCHAR wszPath[] = L"WhiteEternity.exe";
	WCHAR wszDll[] = L"xmoe_chs.dll";
  
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	/*
	WCHAR wszCurrent[MAX_PATH] = {0};
	GetCurrentDirectoryW(MAX_PATH, wszCurrent);
	*/

	BOOL ret = CreateProcessWithDllW(NULL, wszPath, NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi, wszDll, NULL);
	if(ret == FALSE)
	{
		MessageBoxW(NULL, L"不能启动游戏！\n", L"WhiteEternityCHS", MB_OK);
		return 0;
	}

	return 0;
	//MSG msg;
	//HACCEL hAccelTable;

	// 初始化全局字符串
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_WHITEETERNITYCHS, szWindowClass, MAX_LOADSTRING);
	//MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	/*
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}
	*/

	//hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WHITEETERNITYCHS));

	// 主消息循环:

	/*
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
	*/
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//

/*
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WHITEETERNITYCHS));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WHITEETERNITYCHS);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}
*/

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//

/*
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

*/

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//


/*
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

*/

// “关于”框的消息处理程序。

/*
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
*/


