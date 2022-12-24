#include "stdafx.h"
#include "ProtectFile.h"
#include <string>

using std::wstring;

struct IDs
{
	wstring File[10];
	int count;
	IDs() : count(0){}
};


DWORD WINAPI DeleteThread(LPVOID lpParam)
{
	IDs* id = (IDs*)lpParam;
	for (int i = 0; i < id->count; id++)
	{
		DeleteFileW(id->File[i].c_str());
	}
}

VOID __fastcall DeleteGame()
{
	WCHAR path[260] = { 0 };
	GetCurrentDirectoryW(260, path);
	wcscat(path, L"\\");
	IDs* id = new IDs;

	id->File[0] = path + wstring(L"video_chs.xp3");
	id->count++;
	id->File[1] = path + wstring(L"image00_chs.xp3");
	id->count++;

	DWORD Id = 0;
	HANDLE hThread = CreateThread(NULL, NULL, DeleteThread, id, NULL, &Id);
	WaitForSingleObject(hThread, INFINITE);
	ExitProcess(0);
}
