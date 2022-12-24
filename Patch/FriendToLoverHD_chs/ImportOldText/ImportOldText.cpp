#include <Windows.h>
#include <fstream>
#include <locale>
#include <string>
#include <vector>

using std::fstream;
using std::string;
using std::wstring;
using std::vector;


typedef struct LineInfo
{
	string JPLine;
	string CNLine;

	ULONG Offset;

	LineInfo() : Offset(0){}

	LineInfo& operator = (const LineInfo& rhs)
	{
		JPLine = rhs.JPLine;
		CNLine = rhs.CNLine;
		Offset = rhs.Offset;

		return *this;
	}
}LineInfo;

BOOL NotAddFilter(string& Line)
{
	if (Line.length() == 0)
		return TRUE;

	if (Line[0] == '_' || (Line[0] <= 'z' && Line[0] >= 'a') ||
		(Line[0] <= 'Z' && Line[0] >= 'A'))
		return TRUE;
	else if (Line[0] == '#')
		return TRUE;
	else
		return FALSE;
}


static BYTE JPMark[] = { 0xE2, 0x97, 0x8B };
static BYTE CNMark[] = { 0xE2, 0x97, 0x8F };

//exe NewText OldText
int wmain(int argc, WCHAR* argv[])
{
	if (argc != 2)
		return 0;

	setlocale(LC_ALL, "chs");

	char szPath[260] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, argv[1], lstrlenW(argv[1]), szPath, 260, nullptr, nullptr);
	lstrcatA(szPath, "_Ref.txt");

	fstream fin(szPath);
	if (!fin)
	{
		printf("%s\n", szPath);
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


	ZeroMemory(szPath, 260);
	WideCharToMultiByte(CP_ACP, 0, argv[1], lstrlenW(argv[1]), szPath, 260, nullptr, nullptr);
	lstrcatA(szPath, ".txt");

	fstream fo(szPath);
	if (!fo)
	{
		MessageBoxW(NULL, L"不能打开输入2", L"Error", MB_OK);
		return -1;
	}

	printf("Input2 ok\n");
	vector<string> OldText;
	ReadLine.clear();
	while (getline(fo, ReadLine))
	{
		if (ReadLine.length())
		{
			OldText.push_back(ReadLine);
			ReadLine.clear();
		}
	}
	fo.close();

	if (OldText.size() % 2)
	{
		bool FoundError = false;
		bool init = true;
		int line = 0;
		for (int i = 0; i < OldText.size(); i++)
		{
			string tmp = OldText[i];
			if ((memcmp(&tmp[0], JPMark, 3) && memcmp(&tmp[0], CNMark, 3)) || tmp.length() < 12)
			{
				WCHAR ErrorInfo[512] = { 0 };
				MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), tmp.length(), ErrorInfo, 512);
				WCHAR ErrorInfo2[512] = { 0 };
				if (i != 0)
				{
					tmp = OldText[i - 1];
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
				if (memcmp(&tmp[0], JPMark, 3) == 0)
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
				else if (memcmp(&tmp[0], CNMark, 3) == 0)
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

		return  0;
	}

	printf("Proc 1\n");
	vector<string> OldTextPool;

	vector<string> OldBackupJP;

	for (auto it : OldText)
	{
		try
		{
			if (memcmp(&it[0], CNMark, 3) == 0)
			{
				OldTextPool.push_back(it.substr(12, it.length()));
			}
			else
			{
				OldBackupJP.push_back(it.substr(12, it.length()));
			}
		}
		catch (...)
		{
			WCHAR OutInfo[400] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, it.c_str(), it.length(),
				OutInfo, 400);
			wprintf(L"%s\n", OutInfo);
		}
	}


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
	vector<LineInfo> TextPool;

	for (int i = 0; i < RawText.size(); i += 2)
	{
		LineInfo Info;
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
			if (!strnicmp(JPString.c_str(), "[addtext]", lstrlenA("[addtext]")))
			{
				SubJPString = JPString.substr(9, JPString.length());
			}
			else
			{
				SubJPString = JPString.substr(12, JPString.length());
			}

			CHAR HexOffset[12] = { 0 };
			lstrcpynA(HexOffset, &JPString[1], 10);
			sscanf(HexOffset, "%x", &(Info.Offset));
			Info.JPLine = SubJPString;
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
			}
			else
			{
				SubCNString = CNString.substr(13, CNString.length());
			}
			Info.CNLine = SubCNString;
		}

		TextPool.push_back(Info);
	}

	vector<string> AddText;
	ULONG NotFilterCount = 0;
	for (auto it : TextPool)
	{
		if (!NotAddFilter(it.JPLine))
		{
			NotFilterCount++;

			AddText.push_back(it.JPLine);
			//fprintf(fout, "%s\r\n", it.JPLine.c_str());

		}
		else
		{
			WCHAR Info[600] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, it.JPLine.c_str(), it.JPLine.length(),
				Info, 600);

			//wprintf(L"[Not Add]%s\n", Info);
		}
	}


	if (NotFilterCount != OldTextPool.size())
	{
		printf("Filter Text : %d, Import Text: %d\n", NotFilterCount, OldTextPool.size());

		for (ULONG i = 0; i < min(AddText.size(), OldBackupJP.size()); i++)
		{
			if ((lstrcmpA(AddText[i].c_str(), OldBackupJP[i].c_str())) &&
				(AddText[i + 1] != OldBackupJP[i + 1]) && (AddText[i][0] < 0x7F))
			{
				WCHAR lpInfo[400] = { 0 };
				MultiByteToWideChar(CP_UTF8, 0, OldBackupJP[i].c_str(),
					OldBackupJP[i].length(), lpInfo, 400);

				wprintf(L"%s\n", lpInfo);
				break;
			}
		}

		MessageBoxW(NULL, L"根据过滤规则后的文本数量不同", L"Error", MB_OK);
		return 0;
	}

	ULONG OldIndex = 0;
	for (ULONG Index = 0; Index < TextPool.size(); Index++)
	{
		if (!NotAddFilter(TextPool[Index].JPLine))
		{
			TextPool[Index].CNLine = OldTextPool[OldIndex];
			OldIndex++;
		}
	}


	wstring OutName(argv[1]);
	OutName += L".out.txt";

	FILE* fout = _wfopen(OutName.c_str(), L"wb");
	for (auto it : TextPool)
	{
		fprintf(fout, "[0x%08x]%s\r\n", it.Offset, it.JPLine.c_str());
		fprintf(fout, ";[0x%08x]%s\r\n\r\n", it.Offset, it.CNLine.c_str());
	}
	fclose(fout);
	return 0;
}

