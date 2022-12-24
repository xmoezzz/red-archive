#ifndef __Common_H__
#define __Common_H__

#include <windows.h>

BOOL StartHook(LPCSTR szProcName, PROC pfnOrg, PROC pfnNew);

int __stdcall lstrcmpiEX(LPCSTR lpString1, LPCSTR lpString2);

/*****************************/
//private method:

void __stdcall CalculateXORTable();
void __stdcall FirstDecode(char *in, unsigned int in_len);
void __stdcall QuitAndShowMess(const char *in);
void __stdcall DecodeHeader(char *cHeader);
void __stdcall EncodeText(DWORD in /*, const char *out*/);
void __stdcall CalculateTextXorTable() ;
void __stdcall GetAddr(DWORD p);

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
);

DWORD __stdcall MYGetGlyphOutlineA(
     HDC hdc,
     UINT uChar,
     UINT uFormat,
     LPGLYPHMETRICS lpgm,
     DWORD cbBuffer,
     LPVOID lpvBuffer,
     const MAT2 *lpmat2
);

//MyMessageBoxA
int WINAPI MyMessageBoxA(
    HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
	UINT uType
);

//OEM CP
HANDLE WINAPI MyCreateFileA(
     LPCSTR lpFileName,
     DWORD dwDesiredAccess,
     DWORD dwShareMode,
     LPSECURITY_ATTRIBUTES lpSecurityAttributes,
     DWORD dwCreationDisposition,
     DWORD dwFlagsAndAttributes,
     HANDLE hTemplateFile
);

#endif
