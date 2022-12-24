// TestFont.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <locale>

typedef struct
{
	BYTE  Flag1;
	BYTE  Flag2;
	BYTE  Padding[2];
	DWORD Data1;
	DWORD Data2;
}FONT_INFO;

int __stdcall Proc(const LOGFONTW *a1, const TEXTMETRICW *a2, DWORD a3, FONT_INFO *a4)
{
	int v4; // eax@5
	int result; // eax@11

	if (a4->Flag1 && !(a3 & 4))
		return 1;
	if (a4->Flag2 && (a1->lfPitchAndFamily & 3) == 2)
		return 1;
	v4 = a4->Data2;
	if (v4 == 1 && a1->lfFaceName[0] == 64)
		return 1;
	if (v4 == 2 && a1->lfFaceName[0] != 64)
		return 1;
	if (v4 != 3)
	{
		wprintf(L"[1] %s\n", a1->lfFaceName);
		return 1;
	}
	if (a1->lfFaceName[0] == 64)
	{
		wprintf(L"[2] %s\n", a1->lfFaceName);
		result = 1;
	}
	else
	{
		wprintf(L"[3] %s\n", a1->lfFaceName);
		result = 1;
	}
	return 1;
}


signed int __stdcall Proc2(const LOGFONTW *a1, const TEXTMETRICW *a2, DWORD a3, FONT_INFO *a4)
{
	int v4; // eax@5

	if ((!a4->Flag1 || a3 & 4) && (!a4->Flag2 || (a1->lfPitchAndFamily & 3) != 2))
	{
		v4 = a4->Data2;
		if ((v4 != 1 || a1->lfFaceName[0] != 64) && (v4 != 2 || a1->lfFaceName[0] == 64))
		{
			if (v4 == 3)
			{
				wprintf(L"[1] %s\n", a1->lfFaceName);
				return 1;
			}
			wprintf(L"[2] %s\n", a1->lfFaceName);
		}
	}
	return 1;
}


#define CSET_GBK	"936"
#define CSET_UTF8	"65001"
#define LC_NAME_zh_CN	"Chinese_People's Republic of China"


#define LC_NAME_zh_CN_GBK		LC_NAME_zh_CN "." CSET_GBK
#define LC_NAME_zh_CN_UTF8		LC_NAME_zh_CN "." CSET_UTF8
#define LC_NAME_zh_CN_DEFAULT	LC_NAME_zh_CN_GBK

int _tmain(int argc, _TCHAR* argv[])
{
	tagLOGFONTW Logfont;

	//setlocale(LC_ALL, LC_NAME_zh_CN_DEFAULT);
	setlocale(0, "Japanese");

	Logfont.lfHeight = 0;
	memset(&Logfont.lfWidth, 0, 0x58u);
	Logfont.lfCharSet = GB2312_CHARSET;

	FONT_INFO v6;
	v6.Flag1 = 0;
	v6.Flag2 = 0;
	v6.Data1 = 128;
	v6.Data2 = 2;

	EnumFontFamiliesExW(CreateCompatibleDC(NULL), &Logfont, (FONTENUMPROCW)Proc2, (LPARAM)&v6, 0);
	getchar();
	return 0;
}

