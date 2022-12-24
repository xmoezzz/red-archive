#include "DebugTool.h"

VOID WINAPI DebugInfo(const WCHAR* Name)
{
#ifdef DebugMode
	DWORD nRet = 0;
	HANDLE hOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsole(hOutputHandle, Name, lstrlenW(Name), &nRet, NULL);
	WriteConsole(hOutputHandle, L"\n", 1, &nRet, NULL);
#endif
}
