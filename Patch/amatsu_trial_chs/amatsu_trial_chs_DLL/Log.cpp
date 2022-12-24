#include "Log.h"

DWORD NTAPI OutputInfo(LPCSTR lpString)
{
	DWORD    nRet;
	HANDLE   HandleOfStd = GetStdHandle(STD_OUTPUT_HANDLE);

	WriteConsoleA(HandleOfStd, lpString, lstrlenA(lpString), &nRet, NULL);
	WriteConsoleA(HandleOfStd, "\n", 1, &nRet, NULL);
	return   nRet;
}

DWORD NTAPI OutputInfo(LPCSTR lpString, INT CodePage)
{
	DWORD    nRet;
	HANDLE   HandleOfStd = GetStdHandle(STD_OUTPUT_HANDLE);
	WCHAR    StringInfo[0x1000];
	CHAR     DebugInfo[0x1000];
	CHAR     SingleChar[10];

	RtlZeroMemory(StringInfo, 0x2000);
	RtlZeroMemory(DebugInfo, sizeof(DebugInfo));
	MultiByteToWideChar(CodePage, 0, lpString, lstrlenA(lpString), StringInfo, 0x1000);

	lstrcatA(DebugInfo, "[");
	for (ULONG_PTR i = 0; i < lstrlenA(lpString); i++)
	{
		RtlZeroMemory(SingleChar, 10);

		if (i != lstrlenA(lpString) - 1)
			wsprintfA(SingleChar, "%02x, ", (BYTE)lpString[i]);
		else
			wsprintfA(SingleChar, "%02x]", (BYTE)lpString[i]);

		lstrcatA(DebugInfo, SingleChar);
	}

	WriteConsoleW(HandleOfStd, StringInfo, lstrlenW(StringInfo), &nRet, NULL);
	WriteConsoleA(HandleOfStd, DebugInfo, lstrlenA(DebugInfo), &nRet, NULL);
	WriteConsoleA(HandleOfStd, "\n", 1, &nRet, NULL);
	return   nRet;
}

DWORD NTAPI OutputInfo(LPCWSTR lpString)
{
	DWORD    nRet;
	HANDLE   HandleOfStd = GetStdHandle(STD_OUTPUT_HANDLE);

	WriteConsoleW(HandleOfStd, lpString, lstrlenW(lpString), &nRet, NULL);
	WriteConsoleW(HandleOfStd, L"\n",    1,                  &nRet, NULL);
	return   nRet;
}
