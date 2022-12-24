// PackText.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <vector>
#include <string>
#include <fstream>
#include <Windows.h>

using std::string;
using std::wstring;
using std::vector;
using std::fstream;


BYTE WhiteSpace[3] = { 0xE3, 0x80, 0x80 };
BYTE TextStart[] = { 0xE3, 0x80, 0x8C };
BYTE TextEnd[] = { 0xE3, 0x80, 0x8D };

typedef struct NameList
{
	WCHAR* JPName;
	WCHAR* CHName;
}NameList;


NameList NamePairList[] =
{
	{ nullptr, nullptr }
};

ULONG LineCount = 0;

vector<string> TextPool;
vector<string> ScriptPool;

int GetText(LPCWSTR TextName)
{
	_wsetlocale(LC_ALL, L"chs");

	printf("Begin\n");
	fstream fin(TextName);
	if (!fin.is_open())
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

	for (int i = 0; i < RawText.size(); i += 2)
	{
		bool IsTalkLine = false;

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
			SubJPString = JPString.substr(12, std::string::npos);
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
			SubCNString = CNString.substr(13, std::string::npos);
			if (SubCNString.length() == 0)
			{
				SubCNString = SubJPString;
				TextPool.push_back(SubCNString);
			}
			else
			{
				string TempString = CNString.substr(13, std::string::npos);
				if (!memcmp(SubJPString.c_str(), WhiteSpace, 3))
				{
					if (memcmp(TempString.c_str(), WhiteSpace, 3))
						TempString = (char*)WhiteSpace + TempString;

					TextPool.push_back(TempString);
				}
				else if (!memcmp(SubJPString.c_str(), TextStart, 3))
				{
					string FixedString;
					if (!memcmp(TempString.c_str(), TextStart, 3))
					{
						FixedString = TempString;
					}
					else
					{
						FixedString = (char*)TextStart;
						FixedString += TempString;
					}

					int Strlen = TempString.length();
					Strlen--;
					if (TempString[Strlen] == TextEnd[2] &&
						TempString[Strlen - 1] == TextEnd[1] &&
						TempString[Strlen - 2] == TextEnd[0])
					{

					}
					else
					{
						FixedString += (char*)TextEnd;
					}
					TextPool.push_back(FixedString);
				}
				else
				{
					TextPool.push_back(CNString.substr(13, std::string::npos));
				}
			}
		}
	}
	return 0;
}

int GetScript(LPCWSTR FileName)
{
	fstream fin(FileName);

	string ReadLine;
	while (getline(fin, ReadLine))
	{
		if (ReadLine[0] != ';' && ReadLine[0] != '[')
		{
			LineCount++;
		}
		ScriptPool.push_back(ReadLine);
	}
	fin.close();
	return 0;
}


std::wstring ReplaceFileNameExtension(std::wstring& Path, PCWSTR NewExtensionName)
{
	ULONG_PTR Ptr;

	Ptr = Path.find_last_of(L".");
	if (Ptr == std::wstring::npos)
		return Path + NewExtensionName;

	return Path.substr(0, Ptr) + NewExtensionName;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2)
		return 0;


	SetConsoleTitleW(ReplaceFileNameExtension(wstring(argv[1]), L"").c_str());

	GetText(argv[1]);
	GetScript(ReplaceFileNameExtension(wstring(argv[1]), L"").c_str());

	if (LineCount != TextPool.size())
	{
		printf("Different Line Count %d vs %d\n", LineCount, TextPool.size());
		return 0;
	}


	ULONG CurPos = 0;
	for (auto& Line : ScriptPool)
	{
		if (Line[0] != ';' && Line[0] != '[')
		{
			Line = TextPool[CurPos];
			CurPos++;
		}
	}

	FILE *file = _wfopen(ReplaceFileNameExtension(wstring(argv[1]), L".ini").c_str(), L"wb");
	for (auto& Line : ScriptPool)
	{
		fprintf(file, "%s\r\n", Line.c_str());
	}
	fclose(file);

	return 0;
}

