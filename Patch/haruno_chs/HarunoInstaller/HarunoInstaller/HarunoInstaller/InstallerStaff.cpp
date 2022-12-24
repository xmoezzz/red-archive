// InstallerStaff.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HarunoInstaller.h"
#include "InstallerStaff.h"
#include "afxdialogex.h"


// InstallerStaff �Ի���


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
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
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


// InstallerStaff ��Ϣ�������


void InstallerStaff::OnBnClickedCancel()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	//CDialogEx::OnCancel();
	DWORD ret;
	ret = MessageBoxW(L"���ȷ��Ҫ�˳���װ����QAQ��", L"HarunoInstaller", MB_OKCANCEL);
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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

	// TODO: �ڴ˴������Ϣ����������
}


void InstallerStaff::OnInitMenu(CMenu* pMenu)
{
	CDialogEx::OnInitMenu(pMenu);

	// TODO: �ڴ˴������Ϣ����������
}


void InstallerStaff::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialogEx::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// TODO: �ڴ˴������Ϣ����������
}


void InstallerStaff::OnPaint()
{

	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		/*
		CPaintDC dc(this); // device context for painting
		// TODO: �ڴ˴������Ϣ����������
		// ��Ϊ��ͼ��Ϣ���� CDialogEx::OnPaint()
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
		bmpBackground.LoadBitmap(IDB_BITMAP3);   //IDB_BITMAP��Ӧ��ID                    
		BITMAP   bitmap;
		bmpBackground.GetBitmap(&bitmap);
		CBitmap   *pbmpOld = dcMem.SelectObject(&bmpBackground);
		dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
		//InvalidateRect(rect);
	}
}




void InstallerStaff::OnEnChangeEdit1()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
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
	// TODO:  �ڴ˴������Ϣ����������
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
	// TODO:  �ڴ˴������Ϣ����������
}


//BOOL InstallerStaff::OnEraseBkgnd(CDC* pDC)
//{
//	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
//
//	return CDialogEx::OnEraseBkgnd(pDC);
//}


BOOL InstallerStaff::OnEraseBkgnd(CDC* pDC)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ

	return CDialogEx::OnEraseBkgnd(pDC);
	return 0;
}


void InstallerStaff::OnNcPaint()
{
	// TODO:  �ڴ˴������Ϣ����������
	// ��Ϊ��ͼ��Ϣ���� 
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
	
	// TODO:  �ڴ˸��� DC ���κ�����

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
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
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ

	//return CDialogEx::OnQueryDragIcon();
	return static_cast<HCURSOR>(m_hIcon);
}
