
// HarunoInstallerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HarunoInstaller.h"
#include "HarunoInstallerDlg.h"
#include "afxdialogex.h"

#include "InstallerStaff.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CHarunoInstallerDlg::~CHarunoInstallerDlg()
{
	/*
	if(image)
		delete image;
	if(Back)
		delete Back;
	if(Logo)
		delete Logo;
	*/
	//KillTimer(10);
	//KillTimer(5);
}

BOOL LogoFromIDResource(CCImage* image)
{
 UINT nID = IDB_PNG4;
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


BOOL ImageFromIDResource(CCImage* image)
{
 UINT nID = IDB_PNG2;
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


BOOL BGFromIDResource(CCImage* image)
{
 UINT nID = IDB_PNG3;
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

// CHarunoInstallerDlg �Ի���

CHarunoInstallerDlg::CHarunoInstallerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CHarunoInstallerDlg::IDD, pParent), 
	image(NULL),
	Back(NULL),
	alpha(0),
	bOK(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	/*
	if(image == NULL)
	{
		image = new CImage;
		image->Load(_T("A.png"));
	}
	*/

	/*
	image = new CCImage;
	ImageFromIDResource(image);
	Back  = new CCImage;
	BGFromIDResource(Back);
	Logo = new CCImage;
	LogoFromIDResource(Logo);
	*/
}

void CHarunoInstallerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHarunoInstallerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CHarunoInstallerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CHarunoInstallerDlg::OnBnClickedCancel)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
//	ON_WM_INITMENU()
ON_WM_DESTROY()
ON_WM_MOVE()
END_MESSAGE_MAP()


// CHarunoInstallerDlg ��Ϣ�������

BOOL CHarunoInstallerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	//SetTimer(10,15,NULL);
	//SetTimer(5, 100, NULL);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CHarunoInstallerDlg::OnPaint()
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
		//CDialogEx::OnPaint();
		CPaintDC   dc(this);
		CRect      rect;
		GetClientRect(&rect);
		CDC        dcMem;
		dcMem.CreateCompatibleDC(&dc);

		HDC hdc;
		hdc = dcMem.m_hDC;
		if(image)
		{
			RECT rect;
			rect.top = 0;
			rect.left= 0;
			rect.right = image->GetWidth();
			rect.bottom= image->GetHeight();
			image->Draw(hdc, rect);
		}
		*/
		/*
		CPaintDC   dc(this);
		CRect      rect;
		GetClientRect(&rect);
		CDC        dcMem;
		dcMem.CreateCompatibleDC(&dc);
		dcMem.SetBkMode(TRANSPARENT);

		DWORD DrawWidth = (float)image->GetWidth() / ((float)image->GetHeight()/ (float)rect.Height()) + 1;
		//dc.StretchBlt(0,0,DrawWidth, rect.Height(), &dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,PATCOPY);
		BLENDFUNCTION alphaFunc;
		alphaFunc.BlendOp = AC_SRC_OVER;
		alphaFunc.BlendFlags = 0;
		alphaFunc.SourceConstantAlpha = 255;
		alphaFunc.AlphaFormat = AC_SRC_ALPHA;
		//dc.AlphaBlend(0,0,DrawWidth, rect.Height(), &dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,alphaFunc);
		CDC* pDC = GetDC();
		
		DWORD bgDrawHeight = (float)Back->GetHeight()/ ((float)Back->GetWidth()/ (float)rect.Width()) + 1;

		bOK = true;
		if(!bOK)
		{
			Back->AlphaBlend(pDC->GetSafeHdc(), 0,0, 0);
			//Back->AlphaBlend(pDC->GetSafeHdc(), 0, 0, rect.Width(), bgDrawHeight, 0, 0, image->GetWidth(), image->GetHeight());
			image->AlphaBlend(pDC->GetSafeHdc(), 0, 0, DrawWidth, rect.Height(), 0, 0, image->GetWidth(), image->GetHeight(), 0);
		}
		else
		{
			Back->AlphaBlend(pDC->GetSafeHdc(), 0,0);
			Logo->AlphaBlend(pDC->GetSafeHdc(), rect.Width() - Logo->GetWidth(), 0);	
			//Back->AlphaBlend(pDC->GetSafeHdc(), 0, 0, rect.Width(), bgDrawHeight, 0, 0, image->GetWidth(), image->GetHeight());
			image->AlphaBlend(pDC->GetSafeHdc(), 0, 0, DrawWidth, rect.Height(), 0, 0, image->GetWidth(), image->GetHeight());
		}
		*/
		//image->AlphaBlend(pDC->GetSafeHdc(),0,0,DrawWidth, rect.Height(), &dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight);
		/*
		HBITMAP hbitmap = img.Detach();
		
		CBitmap   bmpBackground;
		bmpBackground.Attach(hbitmap);
		dcMem.SelectObject(&bmpBackground);
		
		dc.StretchBlt(0, 0, img.GetWidth(), img.GetHeight(), &dcMem, 0, 0, img.GetWidth(), img.GetHeight(), SRCCOPY);
		*/
		//this->UpdateWindow();
		//this->RedrawWindow();

		CPaintDC   dc(this);
		CRect      rect;
		GetClientRect(&rect);
		CDC        dcMem;
		dcMem.CreateCompatibleDC(&dc);
		CBitmap   bmpBackground;
		bmpBackground.LoadBitmap(IDB_BITMAP7);   //IDB_BITMAP��Ӧ��ID                    
		BITMAP   bitmap;
		bmpBackground.GetBitmap(&bitmap);
		CBitmap   *pbmpOld = dcMem.SelectObject(&bmpBackground);
		dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CHarunoInstallerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CHarunoInstallerDlg::OnBnClickedOk()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnOK();
	InstallerStaff staff;
	//FreeMem();
	staff.DoModal();
	//DestroyWindow();
}


void CHarunoInstallerDlg::OnBnClickedCancel()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	DWORD ret;
	ret = MessageBoxW(L"���ȷ��Ҫ�˳���װ����QAQ��", L"HarunoInstaller", MB_OKCANCEL);
	if (ret == 1)
	{
		PostQuitMessage(0);
		//ExitProcess(0);
	}
	//CDialogEx::OnCancel();
}


HBRUSH CHarunoInstallerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    switch (nCtlColor)
    {
        case CTLCOLOR_BTN:
        case CTLCOLOR_STATIC:
        {
			if (pWnd->GetDlgCtrlID() == IDC_STATIC)
			{
				//pDC->SetBkColor(RGB(0,0,0));
				//pDC->SetTextColor(RGB(255, 255, 255));
			}

        }
        case CTLCOLOR_DLG:
        {
			COLORREF rgb = 0xFFFFFFFF;
			pDC->SetBkColor(rgb);
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetDCBrushColor(rgb);
            CBrush*     back_brush;
            COLORREF    color;
            color = (COLORREF) GetSysColor(COLOR_BTNFACE);
            back_brush = new CBrush(color);
            return (HBRUSH) (back_brush->m_hObject);
        }
    }
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}


void CHarunoInstallerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if(nIDEvent == 10)
	{
		if(alpha < 0xFF)
		{			
			alpha += 5;
			UpdataAnimation();
		}
		else if(alpha >= 0xFF)
		{
			alpha = 0xFF;
			KillTimer(10);
			bOK = true;
			UpdataAnimation2();
		}
	}
	if (nIDEvent == 5)
	{
		//RedrawWindow();
	}
	CDialogEx::OnTimer(nIDEvent);
}


//void CHarunoInstallerDlg::OnInitMenu(CMenu* pMenu)
//{
//	CDialogEx::OnInitMenu(pMenu);
//
//	// TODO: �ڴ˴������Ϣ����������
//}

void CHarunoInstallerDlg::UpdataAnimation()
{
		CPaintDC   dc(this);
		CRect      rect;
		GetClientRect(&rect);
		CDC        dcMem;
		dcMem.CreateCompatibleDC(&dc);
		dcMem.SetBkMode(TRANSPARENT);
		CBitmap   bmpBackground;
		bmpBackground.LoadBitmap(IDB_BITMAP1);   //IDB_BITMAP��Ӧ��ID                    
		BITMAP   bitmap;
		bmpBackground.GetBitmap(&bitmap);
		CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);
		DWORD DrawWidth = (float)image->GetWidth() / ((float)image->GetHeight()/ (float)rect.Height()) + 1;
		//dc.StretchBlt(0,0,DrawWidth, rect.Height(), &dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,PATCOPY);
		BLENDFUNCTION alphaFunc;
		alphaFunc.BlendOp = AC_SRC_OVER;
		alphaFunc.BlendFlags = 0;
		alphaFunc.SourceConstantAlpha = 255;
		alphaFunc.AlphaFormat = AC_SRC_ALPHA;
		//dc.AlphaBlend(0,0,DrawWidth, rect.Height(), &dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,alphaFunc);
		CDC* pDC = GetDC();
		
		DWORD bgDrawHeight = (float)Back->GetHeight()/ ((float)Back->GetWidth()/ (float)rect.Width()) + 1;

		Back->AlphaBlend(pDC->GetSafeHdc(), 0,0, alpha);
		//Back->AlphaBlend(pDC->GetSafeHdc(), 0, 0, rect.Width(), bgDrawHeight, 0, 0, image->GetWidth(), image->GetHeight());
		image->AlphaBlend(pDC->GetSafeHdc(), 0, 0, DrawWidth, rect.Height(), 0, 0, image->GetWidth(), image->GetHeight(), 0);
}


void CHarunoInstallerDlg::UpdataAnimation2()
{
		CPaintDC   dc(this);
		CRect      rect;
		GetClientRect(&rect);
		CDC        dcMem;
		dcMem.CreateCompatibleDC(&dc);
		dcMem.SetBkMode(TRANSPARENT);
		CBitmap   bmpBackground;
		bmpBackground.LoadBitmap(IDB_BITMAP1);   //IDB_BITMAP��Ӧ��ID                    
		BITMAP   bitmap;
		bmpBackground.GetBitmap(&bitmap);
		CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);
		DWORD DrawWidth = (float)image->GetWidth() / ((float)image->GetHeight()/ (float)rect.Height()) + 1;
		//dc.StretchBlt(0,0,DrawWidth, rect.Height(), &dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,PATCOPY);
		BLENDFUNCTION alphaFunc;
		alphaFunc.BlendOp = AC_SRC_OVER;
		alphaFunc.BlendFlags = 0;
		alphaFunc.SourceConstantAlpha = 255;
		alphaFunc.AlphaFormat = AC_SRC_ALPHA;
		//dc.AlphaBlend(0,0,DrawWidth, rect.Height(), &dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,alphaFunc);
		CDC* pDC = GetDC();
		
		DWORD bgDrawHeight = (float)Back->GetHeight()/ ((float)Back->GetWidth()/ (float)rect.Width()) + 1;

		Back->AlphaBlend(pDC->GetSafeHdc(), 0,0);
		//Back->AlphaBlend(pDC->GetSafeHdc(), 0, 0, rect.Width(), bgDrawHeight, 0, 0, image->GetWidth(), image->GetHeight());
		image->AlphaBlend(pDC->GetSafeHdc(), 0, 0, DrawWidth, rect.Height(), 0, 0, image->GetWidth(), image->GetHeight());
		//InvalidateRect(rect);
}

void CHarunoInstallerDlg::FreeMem()
{
	if(image)
	{
		delete image;
		image = NULL;
	}
	if(Back)
	{
		delete Back;
		Back = NULL;
	}
	if(Logo)
	{
		delete Logo;
		Logo = NULL;
	}
}

void CHarunoInstallerDlg::OnDestroy()
{
	/*
	if(image)
	{
		delete image;
		image = NULL;
	}
	if(Back)
	{
		delete Back;
		Back = NULL;
	}
	if(Logo)
	{
		delete Logo;
		Logo = NULL;
	}
	*/
	CDialogEx::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������
}


void CHarunoInstallerDlg::OnMove(int x, int y)
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
