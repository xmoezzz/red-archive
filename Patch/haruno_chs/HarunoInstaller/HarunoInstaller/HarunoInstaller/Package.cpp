#include "stdafx.h"
#include "Package.h"

ULONG  GetFileLen(LPVOID pBaseaddr, LPVOID pReadBuf)
{
	LPBYTE pBase = (LPBYTE)pBaseaddr;
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pReadBuf;
	ULONG uSize = PIMAGE_OPTIONAL_HEADER((pBase + pDosHeader->e_lfanew + 4 + 20))->SizeOfHeaders;
	PIMAGE_SECTION_HEADER    pSec = (PIMAGE_SECTION_HEADER)(pBase + pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));
	for (int i = 0; i<PIMAGE_FILE_HEADER(pBase + pDosHeader->e_lfanew + 4)->NumberOfSections; ++i)
	{
		uSize += pSec[i].SizeOfRawData;
	}
	return uSize;
}

Package::Package() :
	hFile(INVALID_HANDLE_VALUE),
	ProcessBar(0.0f),
	isStarted(false),
	isFinal(false)
{
}

Package::~Package()
{
	if(hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
	}
}

//
bool Package::TestPackage()
{
	TCHAR exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);
	hFile = CreateFileW(exePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
								FILE_ATTRIBUTE_NORMAL, NULL);
	//hFile = CreateFileW(L"xmoe.pak", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
	//							FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		ULONG exeSize = GetFileLen(GetModuleHandle(NULL), GetModuleHandle(NULL));
		ULONG szGet = GetFileSize(hFile, NULL);

		if (szGet - 4 <= exeSize)
		{
			MessageBoxW(NULL, L"安装包数据文件损毁", L"HarunoInstaller", MB_OK);
			CloseHandle(hFile);
			return false;
		}
		DWORD Readed;
		char Sign[4];

		SetFilePointer(hFile, exeSize, NULL, SEEK_SET);
		//SetFilePointer(hFile, 0, NULL, SEEK_SET);
		ReadFile(hFile, Sign, 4, &Readed, NULL);
		if(memcmp(Sign, "XMOE", 4) != 0)
		{
			MessageBoxW(NULL, L"损坏的安装包", L"HarunoInstaller", MB_OK);
			CloseHandle(hFile);
			return false;
		}
		return true;
	}
	else
	{
		MessageBoxW(NULL, L"安装包数据文件损毁", L"HarunoInstaller", MB_OK);
		return false;
	}
}


double Package::GetProcess()
{
	if(isStarted)
		return ProcessBar;
	else
		return 0.0f;
}


void Package::Extract()
{
	ULONG exeSize = GetFileLen(GetModuleHandle(NULL), GetModuleHandle(NULL));
	std::wstring tPath = Path + L"\\";
	isStarted = true;
////////////////////////////////////
	SetFilePointer(hFile, exeSize + 4, NULL, FILE_BEGIN);
	//SetFilePointer(hFile, 4, NULL, FILE_BEGIN);
	DWORD countOffile = 0;
	DWORD  Readed = 0;
	ReadFile(hFile, &countOffile, 4, &Readed, NULL);
	DWORD DataOffset = 0;

	SetFilePointer(hFile, exeSize + 8, NULL, FILE_BEGIN);
	//SetFilePointer(hFile, 8, NULL, FILE_BEGIN);
	ReadFile(hFile, &DataOffset, 4, &Readed, NULL);
	char *FileBuffer = new char[DataOffset - sizeof(Header)];

	//SetFilePointer(hFile, sizeof(Header), NULL, FILE_BEGIN);
	SetFilePointer(hFile, sizeof(Header) + exeSize, NULL, FILE_BEGIN);
	ReadFile(hFile, FileBuffer, DataOffset - sizeof(Header), &Readed, NULL);
	DWORD iPos = 0;
	for(DWORD i = 0; i < countOffile; ++i)
	{
		wchar_t* name = (wchar_t*)(FileBuffer + iPos);
		iPos += (wcslen(name)+1)*2;
		FileBuf buf;
		buf.name_ = name;
		PackageEntry *entry = (PackageEntry*)(FileBuffer + iPos);
		buf.checksum_ = entry->checksum_;
		buf.offset_   = entry->offset_;
		buf.size_     = entry->size_;

		FileList.push_back(buf);
		iPos += sizeof(PackageEntry);
	}

	delete[] FileBuffer;
	for(DWORD i = 0; i < countOffile; ++i)
	{
		SetFilePointer(hFile, exeSize + FileList[i].offset_, NULL, FILE_BEGIN);
		char* mFile = new char[FileList[i].size_];
		ReadFile(hFile, mFile, FileList[i].size_, &Readed, NULL);
		HANDLE output = INVALID_HANDLE_VALUE;
		output = CreateFileW( std::wstring(tPath + FileList[i].name_).c_str() , GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
							CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(output != INVALID_HANDLE_VALUE)
		{
			FILE* debug = NULL;

			/*
			_wfopen_s(&debug, L"Log.txt", L"a");
			fwprintf(debug, L"%s\r\n", std::wstring(tPath + FileList[i].name_).c_str());
			fclose(debug);
			*/
			WriteFile(output, mFile, FileList[i].size_, &Readed, NULL);
			CloseHandle(output);
		}
		else
		{
			MessageBoxW(NULL, std::wstring(L"Couldn't write file : " + FileList[i].name_).c_str(), L"HarunoInstaller", MB_OK);
			delete[] mFile;
			continue;
		}
		delete[] mFile;
		ProcessBar = double(i + 1)/(double)countOffile;
	}
	CloseHandle(hFile);
	isFinal = true;
}

bool Package::isDone()
{
	return isFinal;
}



DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	Package* tPack = (Package*)lpParam;
	if(lpParam)
	{
		if(!tPack->TestPackage())
		{
			return -1;
		}
		else
		{
			tPack->Extract();
		}
	}
	return 0;
}


void Package::RunProc(Package* p)
{
	 DWORD threadID;
	 HANDLE hThread;

	 AfxBeginThread((AFX_THREADPROC)ThreadProc, p);
	 //hThread = CreateThread(NULL,0,ThreadProc,p,0,&threadID);
	 //WaitForSingleObject(hThread, INFINITE);
	 //CloseHandle(hThread);
}

