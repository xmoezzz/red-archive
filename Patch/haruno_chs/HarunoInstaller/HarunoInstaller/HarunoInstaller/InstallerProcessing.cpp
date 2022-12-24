// InstallerProcessing.cpp : 实现文件
//

#include "stdafx.h"
#include "HarunoInstaller.h"
#include "InstallerProcessing.h"
#include "afxdialogex.h"
#include "InstallerWriting.h"


// InstallerProcessing 对话框


BOOL BGPFromIDResource(CCImage* image)
{
 UINT nID = IDB_PNG8;
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



IMPLEMENT_DYNAMIC(InstallerProcessing, CDialogEx)

InstallerProcessing::InstallerProcessing(CWnd* pParent /*=NULL*/)
	: CDialogEx(InstallerProcessing::IDD, pParent),
	bg(0),
	isInvaild(false)
{
	//bg = new CCImage;
	//BGPFromIDResource(bg);

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);
}

InstallerProcessing::~InstallerProcessing()
{
	if(bg)
	{
		//delete bg;
		//bg = NULL;
	}
}

void InstallerProcessing::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT1, lpDic);
	DDV_MaxChars(pDX, lpDic, 2048);
}


BEGIN_MESSAGE_MAP(InstallerProcessing, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &InstallerProcessing::OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, &InstallerProcessing::OnBnClickedOk)
	ON_WM_PAINTCLIPBOARD()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDCANCEL, &InstallerProcessing::OnBnClickedCancel)
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// InstallerProcessing 消息处理程序


void InstallerProcessing::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
    TCHAR szPath[MAX_PATH];     //存放选择的目录路径    
    CString str;  
    ZeroMemory(szPath, sizeof(szPath));     
      
    BROWSEINFO bi;
    bi.hwndOwner = m_hWnd;     
    bi.pidlRoot = NULL;     
    bi.pszDisplayName = szPath;     
    bi.lpszTitle = L"请选择安装目录";     
    bi.ulFlags = 0;     
    bi.lpfn = NULL;     
    bi.lParam = 0;     
    bi.iImage = 0;     
    //弹出选择目录对话框   
    LPITEMIDLIST lp = SHBrowseForFolder(&bi);     
      
    if(lp && SHGetPathFromIDList(lp, szPath))
    {  
		//Debug时期处理
        //str.Format(L"选择的目录为 %s",  szPath);
        //AfxMessageBox(str);      
		isInvaild = true;
    }  
    else 
	{
		isInvaild = true;
        AfxMessageBox(L"无效的目录!");
	}

	lpDic = szPath;
	UpdateData(FALSE);
}


void InstallerProcessing::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	InstallerWriting w;
	BOOL ret = PathFileExistsW(lpDic.GetBuffer());
	if(ret == TRUE)
	{
		isInvaild = true;
	}
	if(ret && isInvaild)
	{
		PathSaver* p = PathSaver::GetHandle();
		if(p == NULL)
		{
			AfxMessageBox(L"安装程序内部故障\n程序即将退出");
			PostQuitMessage(-1);
		}
		p->SetPath(std::wstring(lpDic.GetBuffer()));
		this->DestroyWindow();
		w.DoModal();
	}
	else
	{
		AfxMessageBox(L"非法或不存在的路径");
	}
}


void InstallerProcessing::OnPaintClipboard(CWnd* pClipAppWnd, HGLOBAL hPaintStruct)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnPaintClipboard(pClipAppWnd, hPaintStruct);
}


void InstallerProcessing::OnPaint()
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

		bg->AlphaBlend(pDC->GetSafeHdc(), -20, 0);
		//InvalidateRect(rect);
		*/
		CPaintDC   dc(this);
		CRect      rect;
		GetClientRect(&rect);
		CDC        dcMem;
		dcMem.CreateCompatibleDC(&dc);
		CBitmap   bmpBackground;
		bmpBackground.LoadBitmap(IDB_BITMAP5);   //IDB_BITMAP对应的ID                    
		BITMAP   bitmap;
		bmpBackground.GetBitmap(&bitmap);
		CBitmap   *pbmpOld = dcMem.SelectObject(&bmpBackground);
		dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
	}
}


void InstallerProcessing::OnBnClickedCancel()
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


HCURSOR InstallerProcessing::OnQueryDragIcon()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	//return CDialogEx::OnQueryDragIcon();
	return static_cast<HCURSOR>(m_hIcon);
}


HBRUSH InstallerProcessing::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor == CTLCOLOR_STATIC)
	{
		//pDC->SetBkMode(TRANSPARENT);
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)::GetStockObject(NULL_BRUSH);
	}
	// TODO:  在此更改 DC 的任何特性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
