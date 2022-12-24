// InstallerStaff.cpp : 实现文件
//

#include "stdafx.h"
#include "HarunoInstaller.h"
#include "InstallerStaff.h"
#include "afxdialogex.h"


// InstallerStaff 对话框


BOOL BGFromIDResource2(CCImage* image)
{
 UINT nID = IDB_PNG5;
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
 image->Load(pstm, true);
 // free/release stuff
 GlobalUnlock(m_hMem);
 pstm->Release();
 FreeResource(lpRsrc);
 return TRUE;
}

IMPLEMENT_DYNAMIC(InstallerStaff, CDialogEx)

InstallerStaff::InstallerStaff(CWnd* pParent /*=NULL*/)
	: CDialogEx(InstallerStaff::IDD, pParent),
	bg(0)
	, mText(_T(""))
{
	//bg = new CCImage;
	//BGFromIDResource2(bg);

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);
}

InstallerStaff::~InstallerStaff()
{
	if(bg)
	{
		delete bg;
		bg = NULL;
	}
}

void InstallerStaff::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(InstallerStaff, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &InstallerStaff::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &InstallerStaff::OnBnClickedOk)
	ON_WM_DESTROY()
	ON_WM_INITMENU()
	ON_WM_INITMENUPOPUP()
	ON_WM_PAINT()
	ON_EN_CHANGE(IDC_EDIT1, &InstallerStaff::OnEnChangeEdit1)
	ON_WM_MOVE()
	ON_WM_MOVING()
//	ON_WM_ERASEBKGND()
ON_WM_ERASEBKGND()
ON_WM_NCPAINT()
ON_WM_CTLCOLOR()
ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// InstallerStaff 消息处理程序


void InstallerStaff::OnBnClickedCancel()
{
	// TODO:  在此添加控件通知处理程序代码
	//CDialogEx::OnCancel();
	DWORD ret;
	ret = MessageBoxW(L"真的确定要退出安装程序QAQ？", L"HarunoInstaller", MB_OKCANCEL);
	if (ret == 1)
	{
		PostQuitMessage(0);
		//ExitProcess(0);
	}
}


void FreeMem2(InstallerStaff *o)
{
	if(o->bg)
	{
		delete o->bg;
		o->bg = NULL;
	}
}

void InstallerStaff::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	//FreeMem2(this);
	this->DestroyWindow();
	InstallerLisence step3;
	step3.DoModal();
}


void InstallerStaff::OnDestroy()
{
	if(bg)
	{
		delete bg;
		bg = NULL;
	}
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}


void InstallerStaff::OnInitMenu(CMenu* pMenu)
{
	CDialogEx::OnInitMenu(pMenu);

	// TODO: 在此处添加消息处理程序代码
}


void InstallerStaff::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialogEx::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// TODO: 在此处添加消息处理程序代码
}


void InstallerStaff::OnPaint()
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
		CBitmap   bmpBackground;
		BITMAP   bitmap;
		bmpBackground.GetBitmap(&bitmap);
		CDC* pDC = GetDC();
		bg->AlphaBlend(pDC->GetSafeHdc(), 0, 0);
		*/


		CPaintDC   dc(this);
		CRect      rect;
		GetClientRect(&rect);
		CDC        dcMem;
		dcMem.CreateCompatibleDC(&dc);
		CBitmap   bmpBackground;
		bmpBackground.LoadBitmap(IDB_BITMAP3);   //IDB_BITMAP对应的ID                    
		BITMAP   bitmap;
		bmpBackground.GetBitmap(&bitmap);
		CBitmap   *pbmpOld = dcMem.SelectObject(&bmpBackground);
		dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
		//InvalidateRect(rect);
	}
}




void InstallerStaff::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void InstallerStaff::OnMove(int x, int y)
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


void InstallerStaff::OnMoving(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnMoving(fwSide, pRect);
	/*
	CPaintDC   dc(this);
	CRect      rect;
	GetClientRect(&rect);
	InvalidateRect(rect);
	*/
	// TODO:  在此处添加消息处理程序代码
}


//BOOL InstallerStaff::OnEraseBkgnd(CDC* pDC)
//{
//	// TODO:  在此添加消息处理程序代码和/或调用默认值
//
//	return CDialogEx::OnEraseBkgnd(pDC);
//}


BOOL InstallerStaff::OnEraseBkgnd(CDC* pDC)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	return CDialogEx::OnEraseBkgnd(pDC);
	return 0;
}


void InstallerStaff::OnNcPaint()
{
	// TODO:  在此处添加消息处理程序代码
	// 不为绘图消息调用 
	/*
	CPaintDC   dc(this);
	CRect      rect;
	GetClientRect(&rect);
	InvalidateRect(rect);
	*/
	//UpdateWindow();
	CDialogEx::OnNcPaint();
}


HBRUSH InstallerStaff::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	
	// TODO:  在此更改 DC 的任何特性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	/*
	CPaintDC   dc(this);
	CRect      rect;
	GetClientRect(&rect);

	switch (nCtlColor)
	{
	case CTLCOLOR_STATIC:
		//InvalidateRect(rect);
		return (HBRUSH) NULL;
		break;

	case CTLCOLOR_EDIT:
		//InvalidateRect(rect);
		return (HBRUSH)NULL;
		break;

	case CTLCOLOR_BTN:

		//InvalidateRect(rect);
		return (HBRUSH)NULL;
		break;
	}
	*/
	
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		//pDC->SetBkMode(TRANSPARENT);
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)::GetStockObject(NULL_BRUSH);
	}

	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}


HCURSOR InstallerStaff::OnQueryDragIcon()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	//return CDialogEx::OnQueryDragIcon();
	return static_cast<HCURSOR>(m_hIcon);
}
