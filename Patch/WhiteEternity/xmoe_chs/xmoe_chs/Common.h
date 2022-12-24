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
DWORD dwExStyle,//���ڵ���չ���
LPCSTR lpClassName,//ָ��ע��������ָ��
LPCSTR lpWindowName,//ָ�򴰿����Ƶ�ָ��
DWORD dwStyle,//���ڷ��
int x,//���ڵ�ˮƽλ��
int y,//���ڵĴ�ֱλ��
int nWidth,//���ڵĿ��
int nHeight,//���ڵĸ߶�
HWND hWndParent,//�����ڵľ��
HMENU hMenu,//�˵��ľ�������Ӵ��ڵı�ʶ��
HINSTANCE hInstance,//Ӧ�ó���ʵ���ľ��
LPVOID lpParam//ָ�򴰿ڵĴ�������
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
