#include <Windows.h>
#include <string>
#include <fstream>
#include <cstdio>
#include <vector>
#include <clocale>

using std::string;
using std::wstring;
using std::fstream;
using std::vector;

const char TextStart[] = { 0xE3, 0x80, 0x8C };
const char TextEnd[] = { 0xE3, 0x80, 0x8D };


typedef struct TextPair
{
	string JPText;
	string CNText;
	ULONG  Index;

	TextPair& operator = (const TextPair& o)
	{
		JPText = o.JPText;
		CNText = o.CNText;
		Index  = o.Index;

		return *this;
	}
};


int ParseText(LPWSTR lpFileName, vector<TextPair>& TextPool)
{
	fstream fin(lpFileName);
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

	ULONG Index = 0;

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

		WCHAR IndexString[12] = { 0 };
		memcpy(IndexString, WideJPString + 1, 20);

		swscanf(IndexString, L"%x", &Index);

		//fwprintf(stdout, L"%s\n", WideJPString);
		//fwprintf(stdout, L"%s\n", WideCNString);
		fflush(stdout);

		if (JPString[0] != '[' || JPString[11] != ']')
		{
			WCHAR Info[512] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, JPString.c_str(), JPString.length(), Info, 512);
			MessageBoxW(NULL, Info, L"Error - TextAuth", MB_OK);
			return 0;
		}
		else
		{
			SubJPString = JPString.substr(12, string::npos);

			if (!memcmp(SubJPString.c_str(), TextStart, 3))
			{
				IsTalkLine = true;
			}
		}

		if ((CNString[0] != ';' || CNString[1] != '[' || CNString[12] != ']'))
		{
			WCHAR Info[512] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, CNString.c_str(), CNString.length(), Info, 512);
			MessageBoxW(NULL, Info, L"Error - TextAuth", MB_OK);
			return 0;
		}
		else
		{
			SubCNString = CNString.substr(13, string::npos);

			if (SubCNString.length() == 0)
				SubCNString = SubJPString;

			string TmpString = CNString.substr(13, string::npos);
			string FixedString;
			if (IsTalkLine)
			{
				if (!memcmp(TmpString.c_str(), TextStart, 3))
				{
					FixedString = TmpString;
				}
				else
				{
					FixedString = TextStart;
					FixedString += TmpString;
				}

				int Strlen = TmpString.length();
				Strlen--;
				if (TmpString[Strlen] == TextEnd[2] &&
					TmpString[Strlen - 1] == TextEnd[1] &&
					TmpString[Strlen - 2] == TextEnd[0])
				{
				}
				else
				{
					FixedString += TextEnd;
				}

				SubCNString = FixedString;
			}
			else
			{
				SubCNString = CNString.substr(13, string::npos);
			}
		}
		TextPair Pair;
		Pair.JPText = SubJPString;
		Pair.CNText = SubCNString;
		if (Pair.CNText.length() == 0)
			Pair.CNText = Pair.JPText;
		Pair.Index = Index;

		TextPool.push_back(Pair);
	}
	return 0;
}

int wmain(int argc, WCHAR* argv[])
{
	_wsetlocale(LC_ALL, L"chs");

	if (argc != 2)
		return 0;

	vector<TextPair> TextPool;

	ParseText(argv[1], TextPool);

	FILE* file = _wfopen(argv[1], L"wb");
	if (!file)
		return 0;

	for (auto& it : TextPool)
	{
		fprintf(file, "[0x%08x]%s\r\n", it.Index, it.JPText.c_str());
		fprintf(file, ";[0x%08x]%s\r\n\r\n", it.Index, it.CNText.c_str());
	}

	fclose(file);

	return 0;
}

