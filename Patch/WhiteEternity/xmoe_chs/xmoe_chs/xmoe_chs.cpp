// xmoe_chs.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include "Patch.h"

//Gdi32.lib
#pragma comment(lib,"Gdi32.lib")

BOOL StartHook(LPCSTR szDllName, PROC pfnOrg, PROC pfnNew)
{
	HMODULE hmod;
	LPCSTR szLibName;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	PIMAGE_THUNK_DATA pThunk;
	DWORD dwOldProtect, dwRVA;
	PBYTE pAddr;

	hmod = GetModuleHandle(NULL);
	pAddr= (PBYTE)hmod;
	pAddr += *((DWORD*) &pAddr[0x3C]);
	dwRVA = *((DWORD*) &pAddr[0x80]);

	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)hmod + dwRVA);

	for(; pImportDesc->Name; pImportDesc++)
	{
		szLibName = (LPCSTR)((DWORD)hmod + pImportDesc->Name);

		if(!_stricmp(szLibName, szDllName))
		{
			pThunk = (PIMAGE_THUNK_DATA)((DWORD)hmod + pImportDesc->FirstThunk);

			for(; pThunk->u1.Function; pThunk++)
			{
				if(pThunk->u1.Function == (DWORD)pfnOrg)
				{
					VirtualProtect((LPVOID)&pThunk->u1.Function, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);

					pThunk->u1.Function = (DWORD) pfnNew;

					VirtualProtect((LPVOID)&pThunk->u1.Function, 4, dwOldProtect, &dwOldProtect);

					return TRUE;
				}

			}
		}
	}
	return FALSE;
}


/*
0045C541  /$ 55             PUSH EBP
0045C542  |. 8BEC           MOV EBP,ESP
0045C544  |. 53             PUSH EBX
0045C545  |. 56             PUSH ESI
0045C546  |. 68 23C54500    PUSH WhiteEte.0045C523                   ; /pModule = "KERNEL32.dll"
0045C54B  |. FF15 34D14500  CALL DWORD PTR DS:[<&KERNEL32.GetModuleH>; \GetModuleHandleA
0045C551  |. 68 31C54500    PUSH WhiteEte.0045C531                   ; /ProcNameOrOrdinal = "CompareStringA"
0045C556  |. 50             PUSH EAX                                 ; |hModule
0045C557  |. FF15 2CD14500  CALL DWORD PTR DS:[<&KERNEL32.GetProcAdd>; \GetProcAddress
0045C55D  |. 6A FF          PUSH -1
0045C55F  |. FF75 0C        PUSH DWORD PTR SS:[EBP+C]
0045C562  |. 6A FF          PUSH -1
0045C564  |. FF75 08        PUSH DWORD PTR SS:[EBP+8]
0045C567  |. 6A 01          PUSH 1
0045C569  |. 68 11040000    PUSH 411
0045C56E  |. FFD0           CALL EAX
0045C570  |. 83C0 FE        ADD EAX,-2
0045C573  |. 5E             POP ESI
0045C574  |. 5B             POP EBX
0045C575  |. 5D             POP EBP
0045C576  \. C2 0800        RETN 8
*/

//lstrcmpiA
int __stdcall lstrcmpiEX(LPCSTR lpString1, LPCSTR lpString2)
{
	int ret = CompareStringA(0x411, 1, lpString1, -1, lpString2, -1);
	return ret - 2;
}

HWND __stdcall MyCreateWindowExA(
	DWORD dwExStyle,//窗口的扩展风格
	LPCSTR lpClassName,//指向注册类名的指针
	LPCSTR lpWindowName,//指向窗口名称的指针
	DWORD dwStyle,//窗口风格
	int x,//窗口的水平位置
	int y,//窗口的垂直位置
	int nWidth,//窗口的宽度
	int nHeight,//窗口的高度
	HWND hWndParent,//父窗口的句柄
	HMENU hMenu,//菜单的句柄或是子窗口的标识符
	HINSTANCE hInstance,//应用程序实例的句柄
	LPVOID lpParam//指向窗口的创建数据
)
{
	wchar_t*    szUniTitle;
	szUniTitle = new wchar_t[MAX_PATH];
	memset(szUniTitle, 0, MAX_PATH*2);

	wchar_t*    szUniClassName;
	szUniClassName = new wchar_t[MAX_PATH];
	memset(szUniClassName, 0, MAX_PATH*2);

	char  szName[MAX_PATH] = {0};
	/*
	LANGID lgID = GetSystemDefaultLangID();
	if(lgID != 0x0804)//0x804 CHS
	{
		char szBIG5[MAX_PATH] = {0};
		WORD wLanguageID = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
		LCID Locale = MAKELCID(wLanguageID, SORT_CHINESE_PRCP); 
		int iRet = LCMapStringA(Locale,LCMAP_TRADITIONAL_CHINESE,lpWindowName, -1, szBIG5, MAX_PATH);
		if(iRet != 0)
		{
			strcpy(szName, szBIG5);
		}
		else
		{
			strcpy(szName, lpWindowName);
		}
	}
	else
	{
		strcpy(szName, lpWindowName);
	}
	*/
	CovtASCIIToUni((const char*)lpWindowName, szUniTitle, MAX_PATH);
	CovtASCIIToUni((const char*)lpClassName,  szUniClassName, MAX_PATH);

	wchar_t wName[] = L"[X'moe汉化组]星辰恋曲的白色永恒V2.0";

	HWND hwnd = CreateWindowExW(dwExStyle, szUniClassName, szUniTitle, dwStyle, x,y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	
	/*
	if(hwnd == NULL)
	{
		wchar_t szInfo[1024] = {0};
		wsprintf(szInfo, L"Failed to CreateWindow [%08x]", GetLastError());
		MessageBoxW(NULL, szInfo, L"Error", MB_OK);
	}
	*/
	return hwnd;
}


bool CovtASCIIToUni(const char* GBKStr, wchar_t *wbuf, int nSize) 
{ 
    int nLen; 

    nLen = strlen(GBKStr)+1; 
    //if (wbuf==NULL) return false; 
    nLen = MultiByteToWideChar(936, 0, GBKStr, 
        nLen, wbuf, nLen); 
    return nLen!=0; 
}

DWORD __stdcall MYGetGlyphOutlineA(
     HDC hdc,
     UINT uChar,
     UINT uFormat,
     LPGLYPHMETRICS lpgm,
     DWORD cbBuffer,
     LPVOID lpvBuffer,
     const MAT2 *lpmat2
)
{
	if(uChar != 0xA1A1)
		return GetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	else
	{
		wchar_t UniChar;
		char GBKChar[2];
		GBKChar[0] = uChar | 0xFF;
		GBKChar[1] = (uChar>>8)| 0xFF;
		CovtASCIIToUni(GBKChar, &UniChar, 2);
		return GetGlyphOutlineW(hdc, UniChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}
}

bool CovtSJISToUni(const char* GBKStr, wchar_t *wbuf, int nSize) 
{ 
    int nLen; 

    nLen = strlen(GBKStr)+1; 
    //if (wbuf==NULL) return false; 
    nLen = MultiByteToWideChar(932, 0, GBKStr, 
        nLen, wbuf, nLen); 
    return nLen!=0; 
}

int WINAPI MyMessageBoxA(
    HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
	UINT uType
)
{
	wchar_t uniStr[1024] = {0};
	wchar_t uniTitle[512] = {0};
	CovtSJISToUni(lpText, uniStr, 1024);
	CovtASCIIToUni(lpCaption, uniTitle, 512);

	return MessageBoxW(hWnd, uniStr, uniTitle, uType);
}


HANDLE WINAPI MyCreateFileA(
     LPCSTR lpFileName,
     DWORD dwDesiredAccess,
     DWORD dwShareMode,
     LPSECURITY_ATTRIBUTES lpSecurityAttributes,
     DWORD dwCreationDisposition,
     DWORD dwFlagsAndAttributes,
     HANDLE hTemplateFile
)
{
	DWORD ACP   = GetACP();
	DWORD OEMCP = GetOEMCP();

	wchar_t RltPath[MAX_PATH * 2] = {0};

	if(ACP != OEMCP)
	{
		MultiByteToWideChar(CP_ACP, NULL, lpFileName, strlen(lpFileName), RltPath, MAX_PATH);
	}
	else
	{
		MultiByteToWideChar(CP_OEMCP, NULL, lpFileName, strlen(lpFileName), RltPath, MAX_PATH);
	}

	return CreateFileW(RltPath, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
                       dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}
