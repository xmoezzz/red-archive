
// HarunoInstaller.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "HarunoInstaller.h"
#include "HarunoInstallerDlg.h"
#include "CCImage.h"

#include <mmsystem.h>
#pragma comment(lib, "WINMM.LIB")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef BOOL(*UPDATELAYEREDWINDOWFUNCTION)(HWND,HDC,POINT*,SIZE*,HDC,POINT*,COLORREF,BLENDFUNCTION*,DWORD);

// CHarunoInstallerApp

BEGIN_MESSAGE_MAP(CHarunoInstallerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CHarunoInstallerApp ����

CHarunoInstallerApp::CHarunoInstallerApp()
{
	// TODO:  �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CHarunoInstallerApp ����

CHarunoInstallerApp theApp;


IStream* ImageFromIDResource2();
void FreeStream(IStream* pstm);


ULONG_PTR gdiplusStartupToken;  
CCImage image;
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
    //StretchBlt(hdcMEM, 0, 0, nWidth, nHeight, hdcImage, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    //StretchBlt(hDC, 0, 0, nWidth, nHeight, hdcMEM, 0, 0, nWidth, nHeight, SRCCOPY);
	RECT rect;
	rect.top = 0;
	rect.left= 0;
	rect.bottom = image.GetHeight();
	rect.right  = image.GetWidth();
	image.Draw(hDC, rect);

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

/*
		BLENDFUNCTION blendFunction;
		blendFunction.AlphaFormat = AC_SRC_ALPHA;
		blendFunction.BlendFlags = 0;
		blendFunction.BlendOp = AC_SRC_OVER;
		blendFunction.SourceConstantAlpha = 255;
		
		UpdateLayeredWindow(hwnd,screenDC,&ptSrc,&wndSize,memDC,&ptSrc,0,&blendFunction,2);
		*/

int CALLBACK TimerProc2()
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
		{
			DestroyWindow(ghWnd);
		}
		IStream* st = ImageFromIDResource2();
		Gdiplus::Image pimage(st);
		FreeStream(st);

		RECT wndRect;
		::GetWindowRect(ghWnd,&wndRect);
		SIZE wndSize = {wndRect.right-wndRect.left,wndRect.bottom-wndRect.top};
		HDC hdc = ::GetDC(ghWnd);
		HDC memDC = ::CreateCompatibleDC(hdc);
		HBITMAP memBitmap = ::CreateCompatibleBitmap(hdc,wndSize.cx,wndSize.cy);
		::SelectObject(memDC,memBitmap);

		Gdiplus::Graphics graphics(memDC);
		graphics.DrawImage(&pimage,0,0,wndSize.cx,wndSize.cy);

		HDC screenDC = GetDC(NULL);
		POINT ptSrc = {0,0};

		BLENDFUNCTION blendFunction;
		blendFunction.AlphaFormat = AC_SRC_ALPHA;
		blendFunction.BlendFlags = 0;
		blendFunction.BlendOp = AC_SRC_OVER;
		blendFunction.SourceConstantAlpha = 255;
		UpdateLayeredWindow(ghWnd,screenDC,&ptSrc,&wndSize,memDC,&ptSrc,0,&blendFunction,2);
        //SetLayeredWindowAttributes(ghWnd,-1,wndAlp,LWA_ALPHA);
    }
    else
    {
        wndAlp+=5;
        if(wndAlp==255)
		{
            flag = 1;
		}

		IStream* st = ImageFromIDResource2();
		Gdiplus::Image pimage(st);
		FreeStream(st);

		RECT wndRect;
		::GetWindowRect(ghWnd,&wndRect);
		SIZE wndSize = {wndRect.right-wndRect.left,wndRect.bottom-wndRect.top};
		HDC hdc = ::GetDC(ghWnd);
		HDC memDC = ::CreateCompatibleDC(hdc);
		HBITMAP memBitmap = ::CreateCompatibleBitmap(hdc,wndSize.cx,wndSize.cy);
		::SelectObject(memDC,memBitmap);

		Gdiplus::Graphics graphics(memDC);
		graphics.DrawImage(&pimage,0,0,wndSize.cx,wndSize.cy);

		HDC screenDC = GetDC(NULL);
		POINT ptSrc = {0,0};

		BLENDFUNCTION blendFunction;
		blendFunction.AlphaFormat = AC_SRC_ALPHA;
		blendFunction.BlendFlags = 0;
		blendFunction.BlendOp = AC_SRC_OVER;
		blendFunction.SourceConstantAlpha = 255;
		UpdateLayeredWindow(ghWnd,screenDC,&ptSrc,&wndSize,memDC,&ptSrc,0,&blendFunction,2);
        //SetLayeredWindowAttributes(ghWnd,-1,wndAlp,LWA_ALPHA);
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
	if(uMsg == WM_CTLCOLOR)
	{
		if(HIWORD(lParam) == CTLCOLOR_DLG)
		{
			HDC dc = GetDC(hwnd);
			COLORREF bkColor = RGB(0,0,0);
			SetBkColor(dc, bkColor);
			SetBkMode(dc, TRANSPARENT);
			//return FALSE;
		}
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

BOOL ImageFromIDResource()
{
 UINT nID = IDB_PNG1;
 LPCTSTR sTR = _T("PNG");

 //UINT nID = IDB_BITMAP1;
 //LPCTSTR sTR = RT_BITMAP;

 HINSTANCE hInst = AfxGetResourceHandle();
 HRSRC hRsrc = ::FindResource (hInst,MAKEINTRESOURCE(nID),sTR); // type
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
 memcpy(pmem,lpRsrc,len);
 IStream* pstm;
 CreateStreamOnHGlobal(m_hMem,FALSE,&pstm);

 // load from stream
 image.Load(pstm, true);
 // free/release stuff
 GlobalUnlock(m_hMem);
 pstm->Release();
 FreeResource(lpRsrc);
 return TRUE;
}


IStream* ImageFromIDResource2()
{
 UINT nID = IDB_PNG1;
 LPCTSTR sTR = _T("PNG");

 //UINT nID = IDB_BITMAP1;
 //LPCTSTR sTR = RT_BITMAP;

 HINSTANCE hInst = AfxGetResourceHandle();
 HRSRC hRsrc = ::FindResource (hInst,MAKEINTRESOURCE(nID),sTR); // type
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
 memcpy(pmem,lpRsrc,len);
 IStream* pstm;
 CreateStreamOnHGlobal(m_hMem,FALSE,&pstm);

 GlobalUnlock(m_hMem);
 FreeResource(lpRsrc);

 return pstm;
 // free/release stuff
}

void FreeStream(IStream* pstm)
{
	pstm->Release();
}

int CALLBACK TimerProc3()
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
		{
			DestroyWindow(ghWnd);
		};
        //SetLayeredWindowAttributes(ghWnd,-1,wndAlp,LWA_ALPHA);
    }
    else
    {
        wndAlp+=5;
        if(wndAlp==255)
		{
            flag = 1;
		}
        //SetLayeredWindowAttributes(ghWnd,-1,wndAlp,LWA_ALPHA);
    }
    return 0;
}



LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ghWnd = hwnd;
	static RECT rect;

    if(uMsg == WM_CREATE)
    {
        ghWnd = hwnd;
        //SetLayeredWindowAttributes(hwnd,-1,0,LWA_ALPHA);
        SetTimer(hwnd,1003,1,(TIMERPROC)TimerProc3);

		/*
        int scrWidth,scrHeight;
		
        scrWidth = GetSystemMetrics(SM_CXSCREEN);
        scrHeight = GetSystemMetrics(SM_CYSCREEN);
        GetWindowRect(hwnd,&rect);
        rect.left = (scrWidth-rect.right)/2;
        rect.top = (scrHeight-rect.bottom)/2;
        SetWindowPos(hwnd,HWND_TOP,rect.left,rect.top,rect.right,rect.bottom,SWP_SHOWWINDOW);
		*/
    }
	if(uMsg == WM_CTLCOLOR)
	{
		;
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



DWORD WINAPI Init(LPVOID lpParam)
{
	Gdiplus::GdiplusStartupInput gdiInput;  
    Gdiplus::GdiplusStartup(&gdiplusStartupToken,&gdiInput,NULL);  
    DWORD routeAddr;
    DWORD oldProtect;

	BOOL LayerSupport = TRUE;
	//XP��֧��Layer
	HMODULE hUser32 = ::LoadLibrary(L"User32.dll");  
	if(!hUser32)  
	{  
		LayerSupport = FALSE;  
	}  
	UPDATELAYEREDWINDOWFUNCTION UpdateLayeredWindow = (UPDATELAYEREDWINDOWFUNCTION)::GetProcAddress(hUser32,"UpdateLayeredWindow");  
	if(!UpdateLayeredWindow)  
	{  
		LayerSupport = FALSE;  
	}  

    if(true/*LayerSupport*/)
    {
        //if(tree)
		//BITMAP            bm;
		//hBitmap=::LoadBitmapW(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_BITMAP2));
		//GetObject(hBitmap, sizeof(bm),(LPSTR)&bm);
		//hBitmap = (HBITMAP)LoadImageA(NULL, "X'moe_eyecatch_loge.bmp",IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_DEFAULTSIZE|LR_LOADFROMFILE);
        ImageFromIDResource();
		//image.AlphaBlend();
		WNDCLASSEXA wcex;
        memset(&wcex,0,sizeof(wcex));
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW ;
        wcex.lpfnWndProc = WindowProc;
		wcex.hInstance = AfxGetInstanceHandle();
        wcex.hbrBackground    = (HBRUSH)(COLOR_BTNFACE+1);
        wcex.lpszClassName    = "XmoeLogo";
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.cbWndExtra = DLGWINDOWEXTRA;
		wcex.hbrBackground = (HBRUSH)NULL /*COLOR_BACKGROUND*/;
        rgb = 0xFFFFFFFF;
        RegisterClassExA(&wcex);
		HWND hWnd = CreateWindowExA(WS_EX_LAYERED|WS_EX_TOPMOST,
			                        "XmoeLogo","XmoeLogo",
									WS_POPUP /*| WS_SYSMENU | WS_SIZEBOX*/ ,
									0,
									0,
									image.GetWidth(),
									image.GetHeight(),
									NULL,
									NULL,
									AfxGetInstanceHandle(),
									NULL);

        ::SetWindowLong(hWnd,GWL_EXSTYLE,GetWindowLong(hWnd,GWL_EXSTYLE)|WS_EX_LAYERED);
		SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0),255,LWA_ALPHA | LWA_COLORKEY);
		ShowWindow(hWnd,SW_SHOW);

		/**************************************/
        UpdateWindow(hWnd);
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    }
	else
	{
		WCHAR szClassName[ ] = L"XmoeLogo";

		Gdiplus::GdiplusStartupInput gdiInput;
		Gdiplus::GdiplusStartup(&gdiplusStartupToken,&gdiInput,NULL);
		/**/
		HWND hwnd;               /* This is the handle for our window */
		MSG messages;            /* Here messages to the application are saved */
		WNDCLASSEX wincl;        /* Data structure for the windowclass */

		/* The Window structure */
		wincl.hInstance = AfxGetInstanceHandle();
		wincl.lpszClassName = szClassName;//+-69+
		wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
		wincl.style = /*CS_DBLCLKS |*/ DS_CENTER | CS_HREDRAW | CS_VREDRAW;                 /* Catch double-clicks */
		wincl.cbSize = sizeof (WNDCLASSEX);

		/* Use default icon and mouse-pointer */
		wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
		wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
		wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
		wincl.lpszMenuName = NULL;                 /* No menu */
		wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
		wincl.cbWndExtra = 0;                      /* structure or the window instance */
		/* Use Windows's default colour as the background of the window */
		wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

		/* Register the window class, and if it fails quit the program */
		if (!RegisterClassEx (&wincl))
			return 0;

		IStream* st = ImageFromIDResource2();
		Gdiplus::Image pimage(st);
		FreeStream(st);

		ULONG x = GetSystemMetrics(SM_CXSCREEN);
		ULONG y = GetSystemMetrics(SM_CYSCREEN);

		ULONG WinX = x / 2 - pimage.GetWidth()/2;
		ULONG WinY = y / 2 - pimage.GetHeight()/2;

		/* The class is registered, let's create the program*/
		hwnd = CreateWindowEx (
           WS_EX_LAYERED|WS_EX_TOPMOST|WS_EX_TOOLWINDOW,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           L"XmoeLogo",       /* Title Text */
           WS_POPUP/*WS_OVERLAPPEDWINDOW*/, /* default window */
           x/2 /*WinX*/,       /* Windows decides the position */
           y/2 /*WinY*/,       /* where the window ends up on the screen */
		   pimage.GetWidth(),                 /* The programs width */
		   pimage.GetHeight(),                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
		   AfxGetInstanceHandle(),       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

		/* Make the window visible on the screen */
		ShowWindow (hwnd, SW_SHOW);
		LONG style = ::GetWindowLong(hwnd,GWL_STYLE);
		if(style&WS_CAPTION)
			style^=WS_CAPTION;
		if(style&WS_THICKFRAME)
			style^=WS_THICKFRAME;
		if(style&WS_SYSMENU)
			style^=WS_SYSMENU;
		::SetWindowLong(hwnd,GWL_STYLE,style);

		style = ::GetWindowLong(hwnd,GWL_EXSTYLE);
		if(style&WS_EX_APPWINDOW)
			style^=WS_EX_APPWINDOW;
		::SetWindowLong(hwnd,GWL_EXSTYLE,style);

		RECT wndRect;
		::GetWindowRect(hwnd,&wndRect);
		SIZE wndSize = {wndRect.right-wndRect.left,wndRect.bottom-wndRect.top};
		HDC hdc = ::GetDC(hwnd);
		HDC memDC = ::CreateCompatibleDC(hdc);
		HBITMAP memBitmap = ::CreateCompatibleBitmap(hdc,wndSize.cx,wndSize.cy);
		::SelectObject(memDC,memBitmap);

		Gdiplus::Graphics graphics(memDC);
		graphics.DrawImage(&pimage,0,0,wndSize.cx,wndSize.cy);

		HDC screenDC = GetDC(NULL);
		POINT ptSrc = {0,0};

		BLENDFUNCTION blendFunction;
		blendFunction.AlphaFormat = AC_SRC_ALPHA;
		blendFunction.BlendFlags = 0;
		blendFunction.BlendOp = AC_SRC_OVER;
		blendFunction.SourceConstantAlpha = 255;
		UpdateLayeredWindow(hwnd,screenDC,&ptSrc,&wndSize,memDC,&ptSrc,0,&blendFunction,2);

		::DeleteDC(memDC);
		::DeleteObject(memBitmap);

		/* Run the message loop. It will run until GetMessage() returns 0 */
		while (GetMessage (&messages, NULL, 0, 0))
		{
			/* Translate virtual-key messages into character messages */
			TranslateMessage(&messages);
			/* Send message to WindowProcedure */
			DispatchMessage(&messages);
		}
		Gdiplus::GdiplusShutdown(gdiplusStartupToken);
		/* The program return-value is 0 - The value that PostQuitMessage() gave */
		//return messages.wParam;
	}
	return 0;
}

BOOL CHarunoInstallerApp::InitInstance()
{

	//PlaySoundW(L"bgm.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	//PlaySoundW(L"IDR_WAVE1", AfxGetResourceHandle(), SND_RESOURCE | SND_NODEFAULT | SND_ASYNC | SND_LOOP);

	HINSTANCE h = AfxGetInstanceHandle();
	HRSRC hr = FindResourceW(h, MAKEINTRESOURCE(IDR_WAVE1), L"WAVE");
	HGLOBAL hg = LoadResource(h, hr);
	LPSTR lp = (LPSTR)LockResource(hg);
	sndPlaySoundA(lp, SND_MEMORY | SND_ASYNC | SND_LOOP);
	/*
	MCI_OPEN_PARMS m_mciOpen;  //�򿪲���
	MCI_PLAY_PARMS m_mciPlay;  //���Ų���

	m_mciOpen.lpstrDeviceType = L"mpegvideo"; //Ҫ�������ļ�����
	m_mciOpen.lpstrElementName = L"bgm.mp3"; //Ҫ�������ļ�·��
	MCIERROR mcierror = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&m_mciOpen)
	*/
	/*
	DWORD id;
	HANDLE hHandle = ::CreateThread(NULL,//defaultsecurityattributes
	0,//usedefaultstacksize
	Init,//threadfunction
	NULL,//argumenttothreadfunction
	0,//usedefaultcreationflags
	&id);

	WaitForSingleObject(hHandle, INFINITE);
	*/
	Init(NULL);
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO:  Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("X'moe Installer V1.0"));

	CHarunoInstallerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "��װ�����ʼ��ʧ��\n");
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

