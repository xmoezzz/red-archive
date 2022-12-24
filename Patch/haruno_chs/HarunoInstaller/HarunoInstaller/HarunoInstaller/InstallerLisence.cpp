// InstallerLisence.cpp : 实现文件
//

#include "stdafx.h"
#include "HarunoInstaller.h"
#include "InstallerLisence.h"
#include "afxdialogex.h"
#include "InstallerProcessing.h"


// InstallerLisence 对话框

IMPLEMENT_DYNAMIC(InstallerLisence, CDialogEx)

BOOL BGPFromIDResourceLis(CCImage* image)
{
	UINT nID = IDB_PNG9;
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

InstallerLisence::InstallerLisence(CWnd* pParent /*=NULL*/)
	: CDialogEx(InstallerLisence::IDD, pParent),
	bg(NULL),
	bOK(false)
{
	//bg = new CCImage;
	//BGPFromIDResourceLis(bg);

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);
}

InstallerLisence::~InstallerLisence()
{
	if (bg)
	{
		//delete bg;
		//bg = NULL;
	}
}

void InstallerLisence::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(InstallerLisence, CDialogEx)
	ON_BN_CLICKED(IDOK, &InstallerLisence::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &InstallerLisence::OnBnClickedCancel)
	ON_WM_PAINT()
	ON_WM_MOVE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_RADIO1, &InstallerLisence::OnBnClickedRadio1)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// InstallerLisence 消息处理程序


void InstallerLisence::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	if (bOK)
	{
		this->DestroyWindow();
		InstallerProcessing step_sel;
		step_sel.DoModal();
	}
	else
	{
		MessageBox(L"请阅读许可=。=", L"HarunoInstaller", MB_OK);
	}
}


void InstallerLisence::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnCancel();
	DWORD ret;
	ret = MessageBoxW(L"真的确定要退出安装程序QAQ？", L"HarunoInstaller", MB_OKCANCEL);
	if (ret == 1)
	{
		PostQuitMessage(0);
		//ExitProcess(0);
	}
}


void InstallerLisence::OnPaint()
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
		// TODO:  在此处添加消息处理程序代码
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
		bmpBackground.LoadBitmap(IDB_BITMAP4);   //IDB_BITMAP对应的ID                    
		BITMAP   bitmap;
		bmpBackground.GetBitmap(&bitmap);
		CBitmap   *pbmpOld = dcMem.SelectObject(&bmpBackground);
		dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
	}
}


void InstallerLisence::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);
	/*
	CPaintDC   dc(this);
	CRect      rect;
	GetClientRect(&rect);
	InvalidateRect(rect);
	*/
	// TODO:  在此处添加消息处理程序代码
}


HCURSOR InstallerLisence::OnQueryDragIcon()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	//return CDialogEx::OnQueryDragIcon();
	return static_cast<HCURSOR>(m_hIcon);
}


void InstallerLisence::OnBnClickedRadio1()
{
	// TODO:  在此添加控件通知处理程序代码
	bOK = true;
}


HBRUSH InstallerLisence::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		//pDC->SetBkMode(TRANSPARENT);
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)::GetStockObject(NULL_BRUSH);
	}
	if (nCtlColor == CTLCOLOR_BTN)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)::GetStockObject(NULL_BRUSH);
	}
	if (nCtlColor == IDC_RADIO1)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)::GetStockObject(NULL_BRUSH);
	}


	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
