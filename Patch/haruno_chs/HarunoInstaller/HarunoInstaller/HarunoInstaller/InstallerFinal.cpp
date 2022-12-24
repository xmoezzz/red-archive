// InstallerFinal.cpp : 实现文件
//

#include "stdafx.h"
#include "HarunoInstaller.h"
#include "InstallerFinal.h"
#include "afxdialogex.h"


BOOL BGFromIDResourceFin(CCImage* image)
{
	UINT nID = IDB_PNG6;
	LPCTSTR sTR = _T("PNG");

	//UINT nID = IDB_BITMAP1;
	//LPCTSTR sTR = RT_BITMAP;

	HINSTANCE hInst = AfxGetResourceHandle();
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
	image->Load(pstm, true);
	// free/release stuff
	GlobalUnlock(m_hMem);
	pstm->Release();
	FreeResource(lpRsrc);
	return TRUE;
}


// InstallerFinal 对话框

IMPLEMENT_DYNAMIC(InstallerFinal, CDialogEx)


void FreeMemLast(InstallerFinal* o)
{
	if(o->bg)
	{
		delete o->bg;
		o->bg = NULL;
	}
}

InstallerFinal::InstallerFinal(CWnd* pParent /*=NULL*/)
	: CDialogEx(InstallerFinal::IDD, pParent),
	bg(NULL)
{
	//bg = new CCImage;
	//BGFromIDResourceFin(bg);

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);
}

InstallerFinal::~InstallerFinal()
{
	if(bg)
	{
		//delete bg;
		//bg = NULL;
	}
}

void InstallerFinal::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(InstallerFinal, CDialogEx)
	ON_BN_CLICKED(IDOK, &InstallerFinal::OnBnClickedOk)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// InstallerFinal 消息处理程序


void InstallerFinal::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	PostQuitMessage(0);
}


void InstallerFinal::OnPaint()
{

	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		/*
		CPaintDC dc(this); // device context for painting
		// TODO: 在此处添加消息处理程序代码
		// 不为绘图消息调用 CDialogEx::OnPaint()

		CRect      rect;
		GetClientRect(&rect);
		CDC        dcMem;
		dcMem.CreateCompatibleDC(&dc);
		dcMem.SetBkMode(TRANSPARENT);
		CDC* pDC = GetDC();

		bg->AlphaBlend(pDC->GetSafeHdc(), 0, 0);
		//InvalidateRect(rect);
		*/
		CPaintDC   dc(this);
		CRect      rect;
		GetClientRect(&rect);
		CDC        dcMem;
		dcMem.CreateCompatibleDC(&dc);
		CBitmap   bmpBackground;
		bmpBackground.LoadBitmap(IDB_BITMAP6);   //IDB_BITMAP对应的ID                    
		BITMAP   bitmap;
		bmpBackground.GetBitmap(&bitmap);
		CBitmap   *pbmpOld = dcMem.SelectObject(&bmpBackground);
		dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
	}
}


HCURSOR InstallerFinal::OnQueryDragIcon()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	//return CDialogEx::OnQueryDragIcon();
	return static_cast<HCURSOR>(m_hIcon);
}
