// InstallerWriting.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HarunoInstaller.h"
#include "InstallerWriting.h"
#include "afxdialogex.h"



// InstallerWriting �Ի���

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
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
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


// InstallerWriting ��Ϣ�������


void InstallerWriting::OnCustomdrawProgress1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
}


int InstallerWriting::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������
	PathSaver* p = PathSaver::GetHandle();
	if(p == NULL)
	{
		AfxMessageBox(L"��װ�����ڲ�����\n���򼴽��˳�");
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
		MessageBoxW(L"��װ�����ڲ�����", L"HarunoInstaller", MB_OK);
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
	// TODO: �ڴ˴������Ϣ����������
}


void InstallerWriting::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

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
		CPaintDC dc(this); // device context for painting
		// TODO:  �ڴ˴������Ϣ����������
		// ��Ϊ��ͼ��Ϣ���� CDialogEx::OnPaint()

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
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ

	//return CDialogEx::OnQueryDragIcon();
	return static_cast<HCURSOR>(m_hIcon);
}
