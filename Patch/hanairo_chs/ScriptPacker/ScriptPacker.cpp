#include <Windows.h>
#include <WinFile.h>
#include <fstream>
#include <cstdio>
#include <vector>
#include <string>
#include "ScriptHeader.h"
#include "Instruction.h"
#include <clocale>

using std::string;
using std::wstring;
using std::fstream;
using std::vector;

BYTE Sign[0x1C] =
{
	0x42, 0x75, 0x72, 0x69, 0x6B,
	0x6F, 0x43, 0x6F, 0x6D, 0x70,
	0x69, 0x6C, 0x65, 0x64, 0x53,
	0x63, 0x72, 0x69, 0x70, 0x74,
	0x56, 0x65, 0x72, 0x31, 0x2E,
	0x30, 0x30, 0x00
};


BOOL JPMode = TRUE;

typedef struct PackInfo
{
	ULONG  StringOffset;
	string StringInfo;     //GBK
	vector<ULONG>  ByteCodeOffset; //rewrite the next offset
	ULONG  NewStrOffset;
	BOOL   OffsetUpdated;
	PackInfo() : StringOffset(0), NewStrOffset(0), OffsetUpdated(FALSE){}

	PackInfo& operator=(PackInfo& lhs)
	{
		this->StringOffset = lhs.StringOffset;
		this->StringInfo = lhs.StringInfo;
		this->ByteCodeOffset = lhs.ByteCodeOffset;
		this->NewStrOffset = lhs.NewStrOffset;
		this->OffsetUpdated = lhs.OffsetUpdated;
		return *this;
	}
}PackInfo;






//Binary Order
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

		fwprintf(stdout, L"%s\n", WideJPString);
		fwprintf(stdout, L"%s\n", WideCNString);
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

	ScriptHeader Header = { 0 };
	ULONG iPos = 0;
	ULONG StreamSize = BinSize;

	memcpy(&Header, BinBuffer, sizeof(ScriptHeader));
	iPos = sizeof(ScriptHeader) - sizeof(ULONG);
	iPos += Header.HeaderSize;

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

	ULONG FinderAll = iPos;
	ULONG MinAddr = 0x7FFFFFFFUL;
	string OpString;
	while (iPos < BinSize)
	{
		ULONG Ins = *(PULONG)(BinBuffer + iPos);
		if (Ins & 0xFFF00000)
		{
			break;
		}
		GetInstructionInfo(BinBuffer, iPos, OpString, MinAddr);
	}

	ScriptHeader* pHeader = (ScriptHeader*)BinBuffer;
	ULONG HeaderSize = pHeader->HeaderSize - sizeof(ULONG) + sizeof(ScriptHeader);

	iPos = HeaderSize;
	//StreamSize = max(LastOffset, MinAddr);

	StreamSize = MinAddr;

	iPos += StreamSize;
	PBYTE BlockPtr = (BinBuffer + iPos);

	//Pack Info
	//假设翻译过程中没人手贱 删了几行之类的
	for (auto it : TextPool)
	{
		WCHAR WideString[2000] = { 0 };
		MultiByteToWideChar(CP_UTF8, 0, it.c_str(), it.length(), WideString, 1024);
		wstring FixedWideString;
		LONG CharIndex = 0;
		while(CharIndex < lstrlenW(WideString))
		{
			if (WideString[CharIndex] == L'\\' && WideString[CharIndex] == L'n')
			{
				FixedWideString += 0x000A;
				CharIndex += 2;
			}
			else
			{
				FixedWideString += WideString[CharIndex];
				CharIndex++;
			}
		}

		CHAR  GBKString[2000] = { 0 };
		BOOL Ret;
		WideCharToMultiByte(936, 0, FixedWideString.c_str(), FixedWideString.length(), GBKString, 2000, nullptr, &Ret);

		if (Ret)
		{
			wprintf(L"Lost Char : %s\n", WideString);
			getchar();
		}

		PackInfo Atom;
		Atom.StringInfo = GBKString;
		Atom.StringOffset = 0;

		PackPool.push_back(Atom);
	}

	auto itr = PackPool.begin();
	ULONG DebugCount = 0;
	while (iPos < BinSize)
	{
		itr->StringOffset = iPos;
		ULONG Len = lstrlenA((PCHAR)BlockPtr) + 1;
		BlockPtr += Len;
		iPos += Len;
		itr++;
		DebugCount++;
	}

	if (DebugCount != PackPool.size())
		printf("%d vs %d\n", PackPool.size(), DebugCount);

	//itr = PackPool.begin();
	iPos = HeaderSize;

	ULONG Dummy;
	while (iPos < StreamSize + HeaderSize)
	{
		ULONG Ins = *(PULONG)(BinBuffer + iPos);
		//Push String
		if (Ins == 0x00000003UL)
		{
			ULONG Offset = *(PULONG)(BinBuffer + iPos + 4);
			printf("Offset : %08x\n", Offset + HeaderSize);

			//auto it : Class 必须重载=运算
			for (ULONG i = 0; i < PackPool.size(); i++)
			{
				if (PackPool[i].StringOffset == (Offset + HeaderSize))
				{
					printf("Offset Found\n");
					PackPool[i].ByteCodeOffset.push_back(iPos + 4);
					PackPool[i].OffsetUpdated = TRUE;
					break;
				}
			}
		}
		GetInstructionInfo(BinBuffer, iPos, OpString, Dummy);
	}
	//getchar();

	//Rebuild String Table and Offset Information
	LONG64 DeltaOffset = 0;

	iPos = HeaderSize;
	StreamSize = max(LastOffset, MinAddr);
	iPos += StreamSize;

	BOOL HasError = FALSE;
	DebugCount = 0;
	for (ULONG i = 0; i < PackPool.size(); i++)
	{
		if (PackPool[i].OffsetUpdated == FALSE)
		{
			HasError = TRUE;
			printf("Offset Error: %s -- %08x\n", PackPool[i].StringInfo.c_str(), PackPool[i].ByteCodeOffset);
			DebugCount++;
		}
	}
	if (HasError)
	{
		printf("Offset Count : %d\n", DebugCount);
		getchar();
	}


	FILE* BinOut = _wfopen((wstring(argv[1]) + L".out").c_str(), L"wb");
	fwrite(BinBuffer, 1, iPos, BinOut);

	for (ULONG i = 0; i < PackPool.size(); i++)
	{
		PackPool[i].NewStrOffset = iPos - HeaderSize;
		fwrite(PackPool[i].StringInfo.c_str(), 1, PackPool[i].StringInfo.length() + 1, BinOut);
		iPos += PackPool[i].StringInfo.length() + 1;
	}

	for (ULONG i = 0; i < PackPool.size(); i++)
	{
		for (ULONG j = 0; j < PackPool[i].ByteCodeOffset.size(); j++)
		{
			fseek(BinOut, PackPool[i].ByteCodeOffset[j], SEEK_SET);
			fwrite(&(PackPool[i].NewStrOffset), 1, 4, BinOut);
		}
	}

	fclose(BinOut);
	delete[] BinBuffer;
	return 0;
}

