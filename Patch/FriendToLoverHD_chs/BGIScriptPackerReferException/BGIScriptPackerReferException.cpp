#include <Windows.h>
#include <WinFile.h>
#include <fstream>
#include <cstdio>
#include <vector>
#include <string>
#include "Instruction.h"
#include "ScriptHeader.h"
#include <clocale>

using std::string;
using std::wstring;
using std::fstream;
using std::vector;

BOOL JPMode = TRUE;


BYTE Sign[0x1C] =
{
	0x42, 0x75, 0x72, 0x69, 0x6B,
	0x6F, 0x43, 0x6F, 0x6D, 0x70,
	0x69, 0x6C, 0x65, 0x64, 0x53,
	0x63, 0x72, 0x69, 0x70, 0x74,
	0x56, 0x65, 0x72, 0x31, 0x2E,
	0x30, 0x30, 0x00
};


typedef struct PackInfo
{
	ULONG  StringOffset;
	string StringInfo;     //GBK
	ULONG  ByteCodeOffset; //rewrite the next offset
	ULONG  NewStrOffset;
	PackInfo() : StringOffset(0), NewStrOffset(0){}

	PackInfo& operator=(PackInfo& lhs)
	{
		this->StringOffset = lhs.StringOffset;
		this->StringInfo = lhs.StringInfo;
		this->ByteCodeOffset = lhs.ByteCodeOffset;
		this->NewStrOffset = lhs.NewStrOffset;
		return *this;
	}
}PackInfo;

//「
static BYTE TalkName[] = { 0xE3, 0x80, 0x8C };

//Return string count
ULONG DisasmProc(PBYTE Buffer, ULONG Size, vector<PackInfo>& PackPool)
{
	ULONG Count = 0;
	ScriptHeader Header = { 0 };
	ULONG iPos = 0;
	ULONG StreamSize = Size;

	memcpy(&Header, Buffer, sizeof(ScriptHeader));
	iPos = sizeof(ScriptHeader) - sizeof(ULONG);
	iPos += Header.HeaderSize;

	ULONG Finder = iPos;
	ULONG LastOffset = Size;
	while (Finder < Size)
	{
		if (*(PULONG)(Buffer + Finder) == 0x0000001B)
		{
			LastOffset = Finder + 4;
		}
		Finder += 4;
	}

	//强制提取异常处理回调函数的文本
	//异常处理回调的文本是不需要提取的

	ULONG MinAddr = 0x7FFFFFFFUL;
	string OpString;
	while (iPos < Size)
	{
		ULONG Ins = *(PULONG)(Buffer + iPos);
		if (Ins & 0xFFF00000)
		{
			break;
		}
		GetInstructionInfo(Buffer, iPos, OpString, MinAddr);
	}

	ULONG HeaderSize = Header.HeaderSize - sizeof(ULONG) + sizeof(ScriptHeader);

	//StreamSize = LastOffset;
	StreamSize = MinAddr + HeaderSize;
	iPos = HeaderSize;
	while (iPos < StreamSize)
	{
		string OpString;
		if (GetInstructionInfo(Buffer, iPos, OpString) == S_OK)
		{
			PackInfo info;
			info.ByteCodeOffset = iPos - 8;
			info.StringOffset = *(PDWORD)(Buffer + iPos - 4);
			info.StringInfo = "";

			PackPool.push_back(info);
			Count++;
		}
	}
	return Count++;
}

//Instruction Order
int wmain(int argc, WCHAR* argv[])
{
	if (argc != 2)
		return 0;

	setlocale(LC_ALL, "chs");
	vector<PackInfo> PackPool;

	char szPath[260] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, argv[1], lstrlenW(argv[1]), szPath, 260, nullptr, nullptr);
	lstrcatA(szPath, ".txt");

	fstream fin(szPath);
	if (!fin)
	{
		MessageBoxW(NULL, L"不能打开输入", L"Error", MB_OK);
		return -1;
	}

	printf("Input ok\n");
	vector<string> RawText;
	string ReadLine;
	while (getline(fin, ReadLine))
	{
		if (ReadLine.length())
		{
			RawText.push_back(ReadLine);
			ReadLine.clear();
		}
	}
	fin.close();


	printf("GetText ok\n");
	if (RawText.size() % 2 != 0)
	{
		bool FoundError = false;
		bool init = true;
		int line = 0;
		for (int i = 0; i < RawText.size(); i++)
		{
			string tmp = RawText[i];
			if ((tmp[0] != '[' && tmp[0] != ';') || tmp.length() < lstrlenA("[0x0000004c]"))
			{
				WCHAR ErrorInfo[512] = { 0 };
				MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), tmp.length(), ErrorInfo, 512);
				WCHAR ErrorInfo2[512] = { 0 };
				if (i != 0)
				{
					tmp = RawText[i - 1];
					MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), tmp.length(), ErrorInfo2, 512);
				}

				wstring ErrorOutput;
				ErrorOutput += ErrorInfo2;
				ErrorOutput += L"\n";
				ErrorOutput += ErrorInfo;
				MessageBoxW(NULL, ErrorOutput.c_str(), L"%2", MB_OK);
				FoundError = true;
				break;
			}
			else
			{
				if (tmp[0] == '[')
				{
					if (!init)
					{
						if (line != 1)
						{
							WCHAR ErrorInfo[512] = { 0 };
							MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), tmp.length(), ErrorInfo, 512);
							WCHAR ErrorInfo2[512] = { 0 };
							if (i != 0)
							{
								tmp = RawText[i - 1];
								MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), tmp.length(), ErrorInfo2, 512);
							}

							wstring ErrorOutput;
							ErrorOutput += ErrorInfo2;
							ErrorOutput += L"\n";
							ErrorOutput += ErrorInfo;
							MessageBoxW(NULL, ErrorOutput.c_str(), L"%2", MB_OK);
							FoundError = true;
						}
						else
						{
							line = 0;
						}
					}
				}
				else if (tmp[0] == ';')
				{
					if (line != 0)
					{
						WCHAR ErrorInfo[512] = { 0 };
						MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), tmp.length(), ErrorInfo, 512);
						WCHAR ErrorInfo2[512] = { 0 };
						if (i != 0)
						{
							tmp = RawText[i - 1];
							MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), tmp.length(), ErrorInfo2, 512);
						}

						wstring ErrorOutput;
						ErrorOutput += ErrorInfo2;
						ErrorOutput += L"\n";
						ErrorOutput += ErrorInfo;
						MessageBoxW(NULL, ErrorOutput.c_str(), L"%2", MB_OK);
						FoundError = true;
					}
					else
					{
						line = 1;
					}
				}

				if (init)
				{
					init = false;
				}
			}
		}

		if (!FoundError)
		{
			MessageBoxW(NULL, L"UnHandle Error", L"Error", MB_OK);
		}
		return 0;
	}


	printf("Modify text\n");
	vector<string> TextPool;


	for (int i = 0; i < RawText.size(); i += 2)
	{
		string JPString = RawText[i + 0];
		string CNString = RawText[i + 1];
		string SubJPString, SubCNString;

		WCHAR WideJPString[1024] = { 0 };
		WCHAR WideCNString[1024] = { 0 };
		MultiByteToWideChar(CP_UTF8, 0, JPString.c_str(), JPString.length(), WideJPString, 1024);
		MultiByteToWideChar(CP_UTF8, 0, CNString.c_str(), CNString.length(), WideCNString, 1024);

		//fwprintf(stdout, L"%s\n", WideJPString);
		//fwprintf(stdout, L"%s\n", WideCNString);
		fflush(stdout);

		if ((JPString[0] != '[' || JPString[11] != ']') && strnicmp(JPString.c_str(), "[addtext]", lstrlenA("[addtext]")))
		{
			WCHAR Info[512] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, JPString.c_str(), JPString.length(), Info, 512);
			MessageBoxW(NULL, Info, L"Error - TextAuth", MB_OK);
			return 0;
		}
		else
		{
			if (!strnicmp(CNString.c_str(), "[addtext]", lstrlenA("[addtext]")))
			{
				SubJPString = JPString.substr(9, CNString.length());
			}
			else
			{
				SubJPString = JPString.substr(12, CNString.length());
			}
		}

		if ((CNString[0] != ';' || CNString[1] != '[' || CNString[12] != ']') &&
			strnicmp(CNString.c_str(), ";[addtext]", lstrlenA(";[addtext]")))
		{
			WCHAR Info[512] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, CNString.c_str(), CNString.length(), Info, 512);
			MessageBoxW(NULL, Info, L"Error - TextAuth", MB_OK);
			return 0;
		}
		else
		{
			if (!strnicmp(CNString.c_str(), ";[addtext]", lstrlenA(";[addtext]")))
			{
				SubCNString = CNString.substr(10, CNString.length());
				if (SubCNString.length() == 0 && SubJPString.length() != 0)
				{
					WCHAR WideName[40] = { 0 };
					MultiByteToWideChar(CP_UTF8, 0, JPString.c_str(), JPString.length(),
						WideName, 40);
					wstring Info = L"找不到缺少翻译:\n";
					Info += WideName;
					MessageBoxW(NULL, Info.c_str(), L"Error", MB_OK);
				}
				else
				{
					TextPool.push_back(CNString.substr(10, CNString.length()));
				}
			}
			else
			{
				SubCNString = CNString.substr(13, CNString.length());
				if (SubCNString.length() == 0 && SubJPString.length() != 0)
				{
					WCHAR WideName[40] = { 0 };
					MultiByteToWideChar(CP_UTF8, 0, JPString.c_str(), JPString.length(),
						WideName, 40);
					wstring Info = L"找不到翻译:\n";
					Info += WideName;
					MessageBoxW(NULL, Info.c_str(), L"Error", MB_OK);
				}
				else
				{
					TextPool.push_back(CNString.substr(13, CNString.length()));
				}
			}
		}
	}

	/*******************************************/
	FILE* BinFile = nullptr;
	ULONG BinSize = 0;
	PBYTE BinBuffer = nullptr;

	BinFile = _wfopen(argv[1], L"rb");
	if (BinFile == nullptr)
	{
		MessageBoxW(NULL, L"无法打开脚本文件", nullptr, MB_OK);
		return 0;
	}
	fseek(BinFile, 0, SEEK_END);
	BinSize = ftell(BinFile);
	rewind(BinFile);
	BinBuffer = new BYTE[BinSize];
	fread(BinBuffer, 1, BinSize, BinFile);
	fclose(BinFile);

	ScriptHeader* Header = (ScriptHeader*)BinBuffer;
	ULONG HeaderSize =  sizeof(ScriptHeader) - sizeof(ULONG) + Header->HeaderSize;

	ULONG OriCount = DisasmProc(BinBuffer, BinSize, PackPool);

	if (OriCount != (ULONG)TextPool.size())
	{
		MessageBoxW(NULL, L"The count of text is different!!", 0, 0);
		return 0;
	}

	//vector<string> TextPool;

	for (int i = 0; i < TextPool.size(); i++)
	{
		string UTF8Str = TextPool[i];

		WCHAR WideStr[2000] = { 0 };
		CHAR  GBKString[2000] = { 0 };

		BOOL Ret;
		MultiByteToWideChar(CP_UTF8, 0, UTF8Str.c_str(), UTF8Str.length(), WideStr, 2000);

		ULONG CodePage = 936;

		WideCharToMultiByte(CodePage, 0, WideStr, lstrlenW(WideStr), GBKString, 2000, nullptr, &Ret);

		PackPool[i].StringInfo = GBKString;

		//printf("Packing : %s\n", PackPool[i].StringInfo.c_str());
		if (Ret)
		{
			wprintf(L"Lost Char : %s\n", WideStr);
			if (i > 0)
			{
				WCHAR WideStr2[2000] = { 0 };
				MultiByteToWideChar(CP_UTF8, 0, TextPool[i - 1].c_str(),
					TextPool[i - 1].length(), WideStr2, 2000);
				wprintf(L"Lost Char : %s\n", WideStr2);
			}
			getchar();
		}
	}

	//开始计算新的Offset
	ULONG iPos = HeaderSize;

	ULONG Finder = iPos;
	ULONG LastOffset = BinSize;
	while (Finder < BinSize)
	{
		if (*(PULONG)(BinBuffer + Finder) == 0x0000001B)
		{
			LastOffset = Finder + 4;
		}
		Finder += 4;
	}

	ULONG MinAddr = 0x7FFFFFFFUL;
	string OpString;
	iPos = HeaderSize;
	while (iPos < BinSize)
	{
		ULONG Ins = *(PULONG)(BinBuffer + iPos);
		if (Ins & 0xFFF00000)
		{
			break;
		}
		GetInstructionInfo(BinBuffer, iPos, OpString, MinAddr);
	}

	iPos = HeaderSize;
	ULONG StreamSize = MinAddr;
	vector<BYTE> StringPool;
	ULONG StrOffset = StreamSize;

	for (ULONG i = 0; i < PackPool.size(); i++)
	{
		for (ULONG j = 0; j < PackPool[i].StringInfo.length(); j++)
		{
			StringPool.push_back((BYTE)PackPool[i].StringInfo[j]);
		}
		StringPool.push_back(0);

		PackPool[i].NewStrOffset = StrOffset;
		StrOffset += PackPool[i].StringInfo.length() + 1;
	}

	iPos = HeaderSize;
	//开始重新写入Offset
	ULONG Index = 0;
	ULONG Dummy;
	while (iPos < StreamSize + HeaderSize)
	{
		ULONG Ins = *(PULONG)(BinBuffer + iPos);
		//Push String
		if (Ins == 0x00000003UL)
		{
			ULONG Offset = *(PULONG)(BinBuffer + iPos + 4);
			memcpy((BinBuffer + iPos + 4), &(PackPool[Index].NewStrOffset), sizeof(DWORD));
			Index++;
		}
		GetInstructionInfo(BinBuffer, iPos, OpString, Dummy);
	}


	FILE* BinOut = _wfopen((wstring(argv[1]) + L".out").c_str(), L"wb");
	fwrite(BinBuffer, 1, StreamSize + HeaderSize, BinOut);
	fwrite(&(StringPool[0]), 1, StringPool.size(), BinOut);

	fclose(BinOut);
	delete[] BinBuffer;
	return 0;
}

