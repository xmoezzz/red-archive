
// Harmonia启动配置Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Harmonia启动配置.h"
#include "Harmonia启动配置Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHarmonia启动配置Dlg 对话框



CHarmonia启动配置Dlg::CHarmonia启动配置Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CHarmonia启动配置Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHarmonia启动配置Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListBox);
}

BEGIN_MESSAGE_MAP(CHarmonia启动配置Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHS_BUTTON, &CHarmonia启动配置Dlg::OnBnClickedChsButton)
	ON_BN_CLICKED(IDC_ORI_BUTTON, &CHarmonia启动配置Dlg::OnBnClickedOriButton)
	ON_BN_CLICKED(IDC_FONT_BUTTON, &CHarmonia启动配置Dlg::OnBnClickedFontButton)
END_MESSAGE_MAP()




typedef struct
{
	BYTE  Flag1;
	BYTE  Flag2;
	BYTE  Padding[2];
	DWORD Data1;
	DWORD Data2;
	CHarmonia启动配置Dlg* This;
}FONT_INFO;


int WINAPI Proc(const LOGFONTW *a1, const TEXTMETRICW *a2, DWORD a3, FONT_INFO *a4)
{
	if (a1->lfCharSet == GB2312_CHARSET && a1->lfFaceName[0] != L'@')
		a4->This->FontList.push_back(a1->lfFaceName);
	return 1;
}


// CHarmonia启动配置Dlg 消息处理程序

BOOL CHarmonia启动配置Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	tagLOGFONTW Logfont;

	Logfont.lfHeight = 0;
	memset(&Logfont.lfWidth, 0, 0x58u);
	Logfont.lfCharSet = GB2312_CHARSET;

	FONT_INFO v6;
	v6.Flag1 = 0;
	v6.Flag2 = 0;
	v6.Data1 = 128;
	v6.Data2 = 2;
	v6.This = this;

	EnumFontFamiliesExW(CreateCompatibleDC(NULL), &Logfont, (FONTENUMPROCW)Proc, (LPARAM)&v6, 0);


	for (auto& Item : FontList)
	{
		m_ListBox.AddString(Item.c_str());
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。



void CHarmonia启动配置Dlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CHarmonia启动配置Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CHarmonia启动配置Dlg::OnBnClickedChsButton()
{
	STARTUPINFOW si = { 0 };
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = { 0 };
	BOOL bRet = CreateProcessW(
		L"StellarTheaterEncore汉化版.exe",//不在此指定可执行文件的文件名
		NULL,//命令行参数
		NULL,//默认进程安全性
		NULL,//默认进程安全性
		FALSE,//指定当前进程内句柄不可以被子进程继承
		NULL,
		NULL,//使用本进程的环境变量
		NULL,//使用本进程的驱动器和目录
		&si,
		&pi);
	if (bRet)
	{
		//不使用的句柄最好关掉
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		PostQuitMessage(0);
	}
	else
	{
		MessageBoxW(L"游戏启动失败！", L"Stellar☆Theater　Encore启动配置", MB_OK | MB_ICONERROR);
	}
}


void CHarmonia启动配置Dlg::OnBnClickedOriButton()
{
}


const UINT32 table[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

UINT32 GetCRC(BYTE* buf, int nLength)
{
	if (nLength < 1)
		return 0xffffffff;

	UINT32 crc = 0;

	for (int i = 0; i != nLength; ++i)
	{
		crc = table[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
	}

	crc = crc ^ 0xffffffff;
	return crc;
}

void CHarmonia启动配置Dlg::OnBnClickedFontButton()
{
	CHAR FontNameUTF8[MAX_PATH];
	int Index = m_ListBox.GetCurSel();
	if (Index == LB_ERR)
	{
		MessageBoxW(L"你没有选择任何一个字体", L"Stellar☆Theater　Encore启动配置", MB_OK | MB_ICONERROR);
	}
	else
	{
		CString FontName;
		m_ListBox.GetText(Index, FontName);
		CFile File;
		File.Open(L"Anz.ini", CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);


		RtlZeroMemory(FontNameUTF8, sizeof(FontNameUTF8));
		if (FontName[0] == L'@')
			WideCharToMultiByte(CP_UTF8, 0, FontName.GetBuffer() + 1, FontName.GetLength() - 1, FontNameUTF8, _countof(FontNameUTF8),0,0);
		else
			WideCharToMultiByte(CP_UTF8, 0, FontName.GetBuffer(), FontName.GetLength(), FontNameUTF8, _countof(FontNameUTF8), 0, 0);

		DWORD Crc32 = GetCRC((PBYTE)FontNameUTF8, lstrlenA(FontNameUTF8));
		File.Write(&Crc32, 4);
		File.Write(FontNameUTF8, lstrlenA(FontNameUTF8));
		File.Close();

		MessageBoxW(L"配置保存成功", L"Stellar☆Theater　Encore启动配置", MB_OK);
	}
}
