// xmoe_haruno.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Common.h"
#include "CCImage.h"
#include "resource.h"

ULONG_PTR gdiplusStartupToken;

COLORREF rgb;
HWND ghWnd;
HBITMAP hBitmap;
int bitheight;
int bitwidth;
CCImage image;

BOOL ImageFromIDResource();

void DrawBmp(HDC hDC, HBITMAP bitmap, int nWidth, int nHeight)
{
	BITMAP            bm;
	HDC hdcImage;
	HDC hdcMEM;
	hdcMEM = CreateCompatibleDC(hDC);
	hdcImage = CreateCompatibleDC(hDC);
	HBITMAP bmp = ::CreateCompatibleBitmap(hDC, nWidth, nHeight);
	GetObject(bitmap, sizeof(bm), (LPSTR)&bm);
	SelectObject(hdcMEM, bmp);
	SelectObject(hdcImage, bitmap);
	//StretchBlt(hdcMEM, 0, 0, nWidth, nHeight, hdcImage, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	//StretchBlt(hDC, 0, 0, nWidth, nHeight, hdcMEM, 0, 0, nWidth, nHeight, SRCCOPY);
	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.bottom = image.GetHeight();
	rect.right = image.GetWidth();
	image.Draw(hDC, rect);

	DeleteObject(hdcImage);
	DeleteDC(hdcImage);
	DeleteDC(hdcMEM);
}

int CALLBACK TimerProc()
{
	static int wndAlp = 0;
	static int flag = 0;

	if (flag)
	{
		if (flag == 1)
		{
			Sleep(1000);
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

		DrawBmp(GetDC(hwnd), hBitmap, 703, 252);
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

BOOL ImageFromIDResource()
{
	UINT nID = DLL_IDB_PNG1;
	LPCTSTR sTR = _T("PNG");

	//UINT nID = IDB_BITMAP1;
	//LPCTSTR sTR = RT_BITMAP;

	HINSTANCE hInst = ::GetModuleHandleW(L"xmoe_haruno.dll");
	HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nID), sTR); // type
	if (!hRsrc)
	{
		MessageBoxW(NULL, L"无法加载", L"", MB_OK);
		return FALSE;
	}
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

BOOL ImageFromIDResource2()
{
	IStream* is;
	SHCreateStreamOnFileW(L"haruno.png", STGM_READ, &is);
	image.Load(is, true);
	is->Release();
	return TRUE;
}


void FreeStream(IStream* pstm)
{
	pstm->Release();
}


void WINAPI Init()
{
	if (true/*LayerSupport*/)
	{
		//if(tree)
		//BITMAP            bm;
		//hBitmap=::LoadBitmapW(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_BITMAP2));
		//GetObject(hBitmap, sizeof(bm),(LPSTR)&bm);
		//hBitmap = (HBITMAP)LoadImageA(NULL, "X'moe_eyecatch_loge.bmp",IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_DEFAULTSIZE|LR_LOADFROMFILE);
		if (ImageFromIDResource2() == FALSE)
		{
			//MessageBoxW(NULL, L"无法加载", L"", MB_OK);
		}
		//image.AlphaBlend();
		WNDCLASSEXA wcex;
		memset(&wcex, 0, sizeof(wcex));
		wcex.cbSize = sizeof(wcex);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WindowProc;
		wcex.hInstance = GetModuleHandle(NULL);
		wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcex.lpszClassName = "XmoeLogo";
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.cbWndExtra = DLGWINDOWEXTRA;
		wcex.hbrBackground = (HBRUSH)NULL /*COLOR_BACKGROUND*/;
		rgb = 0xFFFFFFFF;
		RegisterClassExA(&wcex);
		HWND hWnd = CreateWindowExA(WS_EX_LAYERED | WS_EX_TOPMOST,
			"XmoeLogo", "XmoeLogo",
			WS_POPUP /*| WS_SYSMENU | WS_SIZEBOX*/,
			0,
			0,
			image.GetWidth(),
			image.GetHeight(),
			NULL,
			NULL,
			GetModuleHandle(NULL),
			NULL);

		//::SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		//SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);
		ShowWindow(hWnd, SW_SHOW);

		/**************************************/
		UpdateWindow(hWnd);
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	}
}


extern "C" __declspec(dllexport) int dummy()
{
	return 0;
}

