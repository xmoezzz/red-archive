#include "DummyAPI.h"


/*
Copyright 2014 - 2015 X'moe
The Haruno Translation Project

DummyAPI :
2nd hook to make process read xm3 package from memory buffer
*/

BOOL WINAPI DummyMemory::DummyReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	SDL_RWread(file, lpBuffer, 1, nNumberOfBytesToRead);
	*lpNumberOfBytesRead = nNumberOfBytesToRead;

	//wchar_t szInfo[40] = { 0 };
	//wsprintfW(szInfo, L"Count : %08x", nNumberOfBytesToRead);
	//MessageBox(NULL, szInfo, L"", MB_OK);

	return TRUE;
	/*
	if (nNumberOfBytesToRead + offset >= size)
	{
		*lpNumberOfBytesRead = 0;
		return FALSE;
	}
	else
	{
		memcpy((unsigned char*)lpBuffer, (unsigned char*)(Buffer + offset), nNumberOfBytesToRead);
		wchar_t szInfo[40] = { 0 };
		wsprintfW(szInfo, L"Count : %08x", nNumberOfBytesToRead);
		MessageBox(NULL, szInfo, L"", MB_OK);
		offset += nNumberOfBytesToRead;
		*lpNumberOfBytesRead = nNumberOfBytesToRead;
		return TRUE;
	}
	*/
}

DWORD WINAPI DummyMemory::SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
	return(DWORD) SDL_RWseek(file, lDistanceToMove, dwMoveMethod);

	/*
	if (dwMoveMethod == FILE_BEGIN)
	{
		wchar_t szInfo[40] = { 0 };
		wsprintfW(szInfo, L"Set : %08x", lDistanceToMove);
		MessageBox(NULL, szInfo, L"", MB_OK);
		offset = lDistanceToMove;
	}
	else if (dwMoveMethod == FILE_CURRENT)
	{
		offset += lDistanceToMove;
	}
	else if (dwMoveMethod == FILE_END)
	{
		offset = (size - lDistanceToMove) - 1;
	}
	return offset;
	*/
}

