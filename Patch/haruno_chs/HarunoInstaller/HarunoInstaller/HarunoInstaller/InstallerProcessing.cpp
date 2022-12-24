// InstallerProcessing.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HarunoInstaller.h"
#include "InstallerProcessing.h"
#include "afxdialogex.h"
#include "InstallerWriting.h"


// InstallerProcessing �Ի���


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
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
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


// InstallerProcessing ��Ϣ�������


void InstallerProcessing::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
    TCHAR szPath[MAX_PATH];     //���ѡ���Ŀ¼·��    
    CString str;  
    ZeroMemory(szPath, sizeof(szPath));     
      
    BROWSEINFO bi;
    bi.hwndOwner = m_hWnd;     
    bi.pidlRoot = NULL;     
    bi.pszDisplayName = szPath;     
    bi.lpszTitle = L"��ѡ��װĿ¼";     
    bi.ulFlags = 0;     
    bi.lpfn = NULL;     
    bi.lParam = 0;     
    bi.iImage = 0;     
    //����ѡ��Ŀ¼�Ի���   
    LPITEMIDLIST lp = SHBrowseForFolder(&bi);     
      
    if(lp && SHGetPathFromIDList(lp, szPath))
    {  
		//Debugʱ�ڴ���
        //str.Format(L"ѡ���Ŀ¼Ϊ %s",  szPath);
        //AfxMessageBox(str);      
		isInvaild = true;
    }  
    else 
	{
		isInvaild = true;
        AfxMessageBox(L"��Ч��Ŀ¼!");
	}

	lpDic = szPath;
	UpdateData(FALSE);
}


void InstallerProcessing::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
			AfxMessageBox(L"��װ�����ڲ�����\n���򼴽��˳�");
			PostQuitMessage(-1);
		}
		p->SetPath(std::wstring(lpDic.GetBuffer()));
		this->DestroyWindow();
		w.DoModal();
	}
	else
	{
		AfxMessageBox(L"�Ƿ��򲻴��ڵ�·��");
	}
}


void InstallerProcessing::OnPaintClipboard(CWnd* pClipAppWnd, HGLOBAL hPaintStruct)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialogEx::OnPaintClipboard(pClipAppWnd, hPaintStruct);
}


void InstallerProcessing::OnPaint()
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
		bmpBackground.LoadBitmap(IDB_BITMAP5);   //IDB_BITMAP��Ӧ��ID                    
		BITMAP   bitmap;
		bmpBackground.GetBitmap(&bitmap);
		CBitmap   *pbmpOld = dcMem.SelectObject(&bmpBackground);
		dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
	}
}


void InstallerProcessing::OnBnClickedCancel()
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


HCURSOR InstallerProcessing::OnQueryDragIcon()
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ

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
	// TODO:  �ڴ˸��� DC ���κ�����

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}
