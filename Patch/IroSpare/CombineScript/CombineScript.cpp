#include <Windows.h>
#include <string>
#include <fstream>
#include <vector>

using std::string;
using std::wstring;
using std::fstream;
using std::vector;


const char TextStart[] = { 0xE3, 0x80, 0x8C, 0 };
const char TextEnd[] = { 0xE3, 0x80, 0x8D, 0 };
const char TextStart_Q[] = { 0xE3, 0x80, 0x8E, 0 };
const char TextEnd_Q[] = { 0xE3, 0x80, 0x8F, 0};

typedef struct TransInfo
{
	string JPLine;
	string CNLine;

	TransInfo& operator = (const TransInfo& o)
	{
		JPLine = o.JPLine;
		CNLine = o.CNLine;

		return *this;
	}
};



void PrintUTF8(LPCSTR lpStr)
{
	WCHAR Info[2000] = { 0 };
	DWORD  Bytes;
	MultiByteToWideChar(CP_UTF8, 0, lpStr, lstrlenA(lpStr), Info, 2000);

	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), Info, lstrlenW(Info), &Bytes, NULL);
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), L"\n", 1, &Bytes, NULL);
}

vector<string> ScriptPool;

int wmain(int argc, WCHAR* argv[])
{
	if (argc != 3)
		return 0;

	_wsetlocale(LC_ALL, L"chs");

	fstream ScriptFile(argv[1]);
	if (!ScriptFile)
		return 0;

	string ReadLine;
	while (getline(ScriptFile, ReadLine))
	{
		ScriptPool.push_back(ReadLine);
	}

	ScriptFile.close();

	wstring OutFileName(argv[2]);
	OutFileName += L".ini";

	fstream fin(argv[2]);
	if (!fin)
	{
		MessageBoxW(NULL, L"不能打开输入", L"Error", MB_OK);
		return -1;
	}

	printf("Input ok\n");
	vector<string> RawText;
	//string ReadLine;
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
		enum
		{
			NONE_TEXT,
			TALK_TEXT,
			TALK_TEXT_Q
		}IsTalkLine;

		IsTalkLine = NONE_TEXT;

		string JPString = RawText[i + 0];
		string CNString = RawText[i + 1];
		string SubJPString, SubCNString, StringCode;

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
				SubJPString = JPString.substr(9, string::npos);
			}
			else
			{
				SubJPString = JPString.substr(12, string::npos);
			}

			if (SubJPString[SubJPString.length() - 1] == ']')
			{
				auto StringPos = SubJPString.find_last_of('[');
				StringCode = SubJPString.substr(StringPos, string::npos);
			}
			IsTalkLine = NONE_TEXT;

			if (!memcmp(SubJPString.c_str(), TextStart, 3))
			{
				IsTalkLine = TALK_TEXT;
			}
			else if (!memcmp(SubJPString.c_str(), TextStart_Q, 3))
			{
				IsTalkLine = TALK_TEXT_Q;
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
				SubCNString = CNString.substr(10, string::npos);
				if (SubCNString.length() == 0)
				{
					SubCNString = SubJPString;
					TextPool.push_back(SubCNString);
				}
				else if (SubCNString == " " && SubJPString != " ")
				{
					SubCNString = "[np]";
					TextPool.push_back(SubCNString);
				}
				else
				{
					string TmpString = CNString.substr(10, string::npos);
					string FixedString;
					if (IsTalkLine != NONE_TEXT)
					{
						if (IsTalkLine == TALK_TEXT)
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
						}
						else if (IsTalkLine == TALK_TEXT_Q)
						{
							if (!memcmp(TmpString.c_str(), TextStart_Q, 3))
							{
								FixedString = TmpString;
							}
							else
							{
								FixedString = TextStart_Q;
								FixedString += TmpString;
							}

							int Strlen = TmpString.length();
							Strlen--;
							if (TmpString[Strlen] == TextEnd_Q[2] &&
								TmpString[Strlen - 1] == TextEnd_Q[1] &&
								TmpString[Strlen - 2] == TextEnd_Q[0])
							{

							}
							else
							{
								FixedString += TextEnd_Q;
							}
						}
						TextPool.push_back(FixedString + StringCode);
					}
					else
					{
						TextPool.push_back(CNString.substr(10, string::npos) + StringCode);
					}

				}
			}
			else
			{
				SubCNString = CNString.substr(13, string::npos);
				if (SubCNString.length() == 0)
				{
					SubCNString = SubJPString;
					TextPool.push_back(SubCNString);
				}
				else if (SubCNString == " " && SubJPString != " ")
				{
					SubCNString = "[np]";
					TextPool.push_back(SubCNString);
				}
				else
				{
					string TmpString = CNString.substr(13, CNString.length());
					string FixedString;
					if (IsTalkLine != NONE_TEXT)
					{
						if (IsTalkLine == TALK_TEXT)
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
						}
						else if (IsTalkLine == TALK_TEXT_Q)
						{
							if (!memcmp(TmpString.c_str(), TextStart_Q, 3))
							{
								FixedString = TmpString;
							}
							else
							{
								FixedString = TextStart_Q;
								FixedString += TmpString;
							}

							int Strlen = TmpString.length();
							Strlen--;
							if (TmpString[Strlen] == TextEnd_Q[2] &&
								TmpString[Strlen - 1] == TextEnd_Q[1] &&
								TmpString[Strlen - 2] == TextEnd_Q[0])
							{

							}
							else
							{
								FixedString += TextEnd_Q;
							}
						}
						TextPool.push_back(FixedString + StringCode);
					}
					else
					{
						TextPool.push_back(CNString.substr(13, string::npos) + StringCode);
					}
				}
			}
		}
	}

	FILE *out = _wfopen((wstring(argv[1]) + L".out").c_str(), L"wb");
	if (out == nullptr)
	{
		MessageBoxW(NULL, L"无法打开输出文本", L"Error", MB_OK);
	}

	printf("Write\n");
	ULONG IndexPos = 0;
	for (auto& it : ScriptPool)
	{

		switch (it[0])
		{
		case ';':
		case '*':
			fprintf(out, "%s\r\n", it.c_str());
			break;

		case '@':
			PrintUTF8(TextPool[IndexPos].c_str());
			if (!memcmp(&TextPool[IndexPos][0], "@nm", 3))
				IndexPos++; //skip script in text

			if (IndexPos >= TextPool.size())
			{
				printf("out of range..\n");
				PrintUTF8(it.c_str());
				return 0;
			}
			
			fprintf(out, "%s\r\n", it.c_str());
			break;

		case '[':
			if (it[1] >= 0x80)
			{
				//ruby text, copy the translated line
				fprintf(out, "%s\r\n", TextPool[IndexPos].c_str());
				IndexPos++;

				if (IndexPos >= TextPool.size())
				{
					printf("out of range..\n");
					PrintUTF8(it.c_str());
					return 0;
				}
			}
			else
			{
				fprintf(out, "%s\r\n", it.c_str());
				IndexPos++;

				if (IndexPos >= TextPool.size())
				{
					printf("out of range..\n");
					PrintUTF8(it.c_str());
					return 0;
				}
			}
			break;

		default:
			fprintf(out, "%s\r\n", TextPool[IndexPos].c_str());
			IndexPos++;

			if (IndexPos >= TextPool.size())
			{
				printf("out of range..\n");
				PrintUTF8(it.c_str());
				return 0;
			}
			break;
		}

	}
	fclose(out);

	return 0;
}

