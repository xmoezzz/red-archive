//#pragma comment(linker, "/entry:main") 
#pragma comment(linker, "/MERGE:.text=.Shinku /MERGE:.data=.xmoe")

#include <Windows.h>
#include "GenMD5Code.h"
#include "WinFile.h"

static const wchar_t* MD5Hash = L"814c5cd7eaeb0b661bd56cd025a7a2c4";

VOID WINAPI ShowInfo(BOOL Ok)
{
	if (Ok)
	{
		MessageBoxW(NULL, L"�ļ���������", L"ShinkuChecker", MB_OK);
	}
	else
	{
		// : L"�ļ��������" ? Ok
		MessageBoxW(NULL, L"�ļ��������", L"ShinkuChecker", MB_OK);
	}
}

int main(int argc, char* argv[])
{
	if (__argc == 1)
	{
		//���Һ��ļ�
		WinFile File;
		ULONG FileSize = 0;
		BYTE* FileBuffer = nullptr;
		if (File.Open(L"��ʰ�쵵�δ��.pdf", WinFile::FileRead) == S_OK)
		{
			FileSize = File.GetSize32();
			FileBuffer = (BYTE*)HeapAlloc(GetProcessHeap(), 0, FileSize);
			if (FileBuffer == nullptr)
			{
				MessageBoxW(NULL, L"�ڴ治�㣬�޷������֤", L"ShinkuChecker", MB_OK);
				File.Release();
				return 0;
			}
			File.Read(FileBuffer, FileSize);
			wstring OutHex;
			GenMD5CodeFile(FileBuffer, FileSize, OutHex);
			if (!wcscmp(MD5Hash, OutHex.c_str()))
			{
				ShowInfo(TRUE);
			}
			else
			{
				ShowInfo(FALSE);
			}

			HeapFree(GetProcessHeap(), 0, FileBuffer);
			File.Release();
		}
		else
		{
			WinFile File;
			ULONG FileSize = 0;
			BYTE* FileBuffer = nullptr;

			WCHAR strFile[MAX_PATH] = { 0 };
			//Select output file
			OPENFILENAMEW OutFileInfo = { 0 };
			OutFileInfo.lStructSize = sizeof(OPENFILENAMEW);
			OutFileInfo.lpstrFilter = L"PDF �ĵ�(*.pdf)\0*.pdf\0";
			OutFileInfo.lpstrFile = strFile;
			OutFileInfo.lpstrTitle = L"�ڵ�ǰĿ¼���Ҳ�������ʰ�쵵�δ��.pdf������ѡ���ļ�";
			OutFileInfo.nMaxFile = MAX_PATH;
			OutFileInfo.Flags = OFN_CREATEPROMPT;
			if (GetOpenFileNameW(&OutFileInfo))
			{
				if (File.Open(OutFileInfo.lpstrFile, WinFile::FileRead) == S_OK)
				{
					FileSize = File.GetSize32();
					FileBuffer = (BYTE*)HeapAlloc(GetProcessHeap(), 0, FileSize);
					if (FileBuffer == nullptr)
					{
						MessageBoxW(NULL, L"�ڴ治�㣬�޷������֤", L"ShinkuChecker", MB_OK);
						File.Release();
						return 0;
					}
					File.Read(FileBuffer, FileSize);
					wstring OutHex;
					GenMD5CodeFile(FileBuffer, FileSize, OutHex);
					if (!wcscmp(MD5Hash, OutHex.c_str()))
					{
						ShowInfo(TRUE);
					}
					else
					{
						ShowInfo(FALSE);
					}

					HeapFree(GetProcessHeap(), 0, FileBuffer);
					File.Release();
					return 0;
				}
				else
				{
					wstring ErrorLog = L"�޷����ļ�:\n";
					ErrorLog += OutFileInfo.lpstrFile;
					MessageBoxW(NULL, ErrorLog.c_str(), L"ShinkuChecker", MB_OK);
				}
			}
			else
			{
				MessageBoxW(NULL, L"���Ϸ����ļ�", L"ShinkuChecker", MB_OK);
				return 0;
			}
		}
	}
	else if (__argc == 2)
	{
		WinFile File;
		ULONG FileSize = 0;
		BYTE* FileBuffer = nullptr;
		if (File.Open(__wargv[1], WinFile::FileRead) == S_OK)
		{
			FileSize = File.GetSize32();
			FileBuffer = (BYTE*)HeapAlloc(GetProcessHeap(), 0, FileSize);
			if (FileBuffer == nullptr)
			{
				MessageBoxW(NULL, L"�ڴ治�㣬�޷������֤", L"ShinkuChecker", MB_OK);
				File.Release();
				return 0;
			}
			File.Read(FileBuffer, FileSize);
			wstring OutHex;
			GenMD5CodeFile(FileBuffer, FileSize, OutHex);
			if (!wcscmp(MD5Hash, OutHex.c_str()))
			{
				ShowInfo(TRUE);
			}
			else
			{
				ShowInfo(FALSE);
			}
			HeapFree(GetProcessHeap(), 0, FileBuffer);
			File.Release();
		}
		else
		{
			MessageBoxW(NULL, L"�ļ���ʧ��", L"ShinkuChecker", MB_OK);
		}
	}
	else
	{
		MessageBoxW(NULL, L"����ȷ��ʹ�÷�ʽorz", L"ShinkuChecker", MB_OK);
	}

	return 0;
}

