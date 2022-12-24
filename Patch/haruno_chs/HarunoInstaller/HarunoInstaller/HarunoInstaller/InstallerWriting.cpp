// InstallerWriting.cpp : 实现文件
//

#include "stdafx.h"
#include "HarunoInstaller.h"
#include "InstallerWriting.h"
#include "afxdialogex.h"



// InstallerWriting 对话框

IMPLEMENT_DYNAMIC(InstallerWriting, CDialogEx)


BOOL BGFromIDResourceWriting(CCImage* image)
{
	UINT nID = IDB_PNG7;
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

InstallerWriting::InstallerWriting(CWnd* pParent /*=NULL*/)
	: CDialogEx(InstallerWriting::IDD, pParent),
	pack(NULL),
	bg(NULL)
{
	bg = new CCImage;
	BGFromIDResourceWriting(bg);

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);

	m_ctrlProgress.Create(WS_CHILD | WS_VISIBLE, CRect(50, 73, 450, 30), pParent,
		IDC_PROGRESS1);
}

InstallerWriting::~InstallerWriting()
{
	if (bg)
	{
		delete bg;
		bg = NULL;
	}
}

void InstallerWriting::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_ctrlProgress);
}


BEGIN_MESSAGE_MAP(InstallerWriting, CDialogEx)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PROGRESS1, &InstallerWriting::OnCustomdrawProgress1)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// InstallerWriting 消息处理程序


void InstallerWriting::OnCustomdrawProgress1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


int InstallerWriting::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	PathSaver* p = PathSaver::GetHandle();
	if(p == NULL)
	{
		AfxMessageBox(L"安装程序内部故障\n程序即将退出");
		PostQuitMessage(-1);
	}
	m_ctrlProgress.SetRange(0,100);
	m_ctrlProgress.SetStep(1);
	m_ctrlProgress.SetPos(0);
	SetTimer(2, 20, NULL);
	pack = new Package;
	PathSaver* t = PathSaver::GetHandle();
	if (t == NULL)
	{
		MessageBoxW(L"安装程序内部错误！", L"HarunoInstaller", MB_OK);
		PostQuitMessage(-1);
	}
	pack->SetPath(t->tPath);
	Package::RunProc(pack);
	return 0;
}


void InstallerWriting::OnDestroy()
{
	CDialogEx::OnDestroy();
	KillTimer(2);
	if (bg != NULL)
	{
		delete bg;
		bg = NULL;
	}
	// TODO: 在此处添加消息处理程序代码
}


void InstallerWriting::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if(nIDEvent == 2)
	{
		if(pack)
		{
			if(!pack->isDone())
			{
				m_ctrlProgress.SetPos((int)ceil(pack->GetProcess()) * 100);
			}
			else
			{
				delete pack;
				KillTimer(2);
				this->DestroyWindow();
				InstallerFinal fin;
				fin.DoModal();
			}
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void InstallerWriting::OnPaint()
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
		CPaintDC dc(this); // device context for painting
		// TODO:  在此处添加消息处理程序代码
		// 不为绘图消息调用 CDialogEx::OnPaint()

		CRect      rect;
		GetClientRect(&rect);
		CDC        dcMem;
		dcMem.CreateCompatibleDC(&dc);
		dcMem.SetBkMode(TRANSPARENT);

		CDC* pDC = GetDC();

		bg->AlphaBlend(pDC->GetSafeHdc(), -20, 0);
	}
}


HCURSOR InstallerWriting::OnQueryDragIcon()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	//return CDialogEx::OnQueryDragIcon();
	return static_cast<HCURSOR>(m_hIcon);
}
