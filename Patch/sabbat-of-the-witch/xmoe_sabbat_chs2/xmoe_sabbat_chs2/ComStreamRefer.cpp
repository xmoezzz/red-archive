#include "ComStreamRefer.h"


wstring GetFileName(const WCHAR* Name)
{
	wstring Info(Name);
	wstring tmp;
	if (Info.find_last_of(L">") != wstring::npos)
	{
		tmp = Info.substr(Info.find_last_of(L">") + 1, wstring::npos);
	}
	else
	{
		//int Pos = Info.find_last_of(L"/") - 1;
		//if (Info[Pos] == L'3' && Info[Pos - 1] == L'p' && Info[Pos - 2] == L'x')
		//{
			tmp = Info.substr(Info.find_last_of(L"/") + 1, wstring::npos);
		//}
	}
	return tmp;
}


wstring PathPrefix(L"TempPath\\");

IStream* WINAPI XmoeCreateStream(const ttstr & _name, tjs_uint32 flags)
{
	ULONG FileSize   = 0;
	PBYTE FileBuffer = nullptr;
#if 0
	WCHAR CurrentPath[MAX_PATH] = { 0 };
	GetCurrentDirectoryW(MAX_PATH, CurrentPath);
	wstring FileName = GetFileName(_name.c_str());
	wstring FullFileName(CurrentPath);
	FullFileName += L"\\";
	FullFileName += PathPrefix;
	FullFileName += FileName;
	WinFile File;
	if (File.Open(FullFileName.c_str(), WinFile::FileRead) != S_OK)
	{
		DebugInfo((wstring(L"Cannot find : ") + FullFileName.c_str()).c_str());
		DWORD ErrorCode = GetLastError();
		WCHAR szErrorString[20] = { 0 };
		wsprintfW(szErrorString, L"%08x", ErrorCode);
		DebugInfo(szErrorString);
		return nullptr;
	}
	FileSize   = File.GetSize32();
	FileBuffer = (PBYTE)CMem::Alloc(FileSize);
	File.Read(FileBuffer, FileSize);
	if (FileBuffer == nullptr)
	{
		File.Release();
		return nullptr;
	}
	File.Release();
#else
	DebugInfo(L"Try");
	DebugInfo(_name.c_str());
	wstring FileName = GetFileName(_name.c_str());
	FileBuffer = FileSystem::GetFileSystem()->QueryFile(FileName, FileSize);
	if (FileBuffer == nullptr)
	{
		return nullptr;
	}
	DebugInfo(FileName.c_str());

#endif

	StreamHolder* Holder = new StreamHolder(FileBuffer, FileSize);
	IStreamAdapter* st = new IStreamAdapter(Holder);
	return st;
}


VOID WINAPI InitPsbDllArea(ULONG S, ULONG L)
{
	DllStart = S;
	DllLength = L;
}

BOOL WINAPI IsVaildArea(ULONG Pos)
{
	if (Pos >= DllStart && Pos < (DllStart + DllLength))
	{
		return TRUE;
	}
	return FALSE;
}