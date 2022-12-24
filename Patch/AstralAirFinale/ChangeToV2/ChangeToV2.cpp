// ChangeToV2.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <Windows.h>

using std::fstream;
using std::string;
using std::wstring;
using std::vector;
using std::map;


BOOL ProcessLineXmoe(ULONG& LineNum, string& Input)
{
	BOOL   Result;
	CHAR   NumStr[12];

	do
	{
		Result = FALSE;

		if (Input.length() <= 13)
			break;

		if (Input[12] != ']')
			break;

		RtlZeroMemory(NumStr, sizeof(NumStr));
		RtlCopyMemory(NumStr, &Input[2], 10);

		sscanf(NumStr, "%x", &LineNum);
		Input = Input.substr(13, string::npos);

		Result = TRUE;
	} while (0);
	return Result;
}


BOOL ProcessLineXmoeOther(ULONG& LineNum, string& Input)
{
	BOOL   Result;
	CHAR   NumStr[12];

	do
	{
		Result = FALSE;

		if (Input[12] != ']')
			break;

		RtlZeroMemory(NumStr, sizeof(NumStr));
		RtlCopyMemory(NumStr, &Input[2], 10);

		sscanf(NumStr, "%x", &LineNum);

		Result = TRUE;
	} while (0);
	return Result;
}

wstring GetOutName(wstring& FileName)
{
	wstring Result = FileName;

	FileName = FileName.substr(0, FileName.find_last_of(L'.'));
	FileName += L"_New.txt";

	return FileName;
}


int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 3)
		return 0;

	vector<ULONG>  NewReadLineNum;

	map<ULONG, string> StringPool;

	string ReadLine;

	fstream OldStream(argv[1]);
	fstream NewStream(argv[2]);

	if (!OldStream.is_open() || !NewStream.is_open())
		return 0;

	while (getline(OldStream, ReadLine))
	{
		if (ReadLine.length() == 0)
			continue;

		if (ReadLine[0] == ';')
		{
			ULONG Line = 0;
			BOOL Result = ProcessLineXmoe(Line, ReadLine);

			if (Result)
			{
				StringPool.insert(std::make_pair(Line, ReadLine));
			}
			else
			{
				printf("Failed : %s\n", ReadLine.c_str());
				getchar();
			}
		}
		ReadLine.clear();
	}

	OldStream.close();

	while (getline(NewStream, ReadLine))
	{
		if (ReadLine.length() == 0)
			continue;

		ULONG Line = 0;
		BOOL Result = ProcessLineXmoeOther(Line, ReadLine);

		if (Result)
		{
			NewReadLineNum.push_back(Line);
		}

		ReadLine.clear();
	}
	NewStream.close();

	if (NewReadLineNum.size() != StringPool.size())
	{
		printf("Line count diff\n");
		getchar();
		return 0;
	}

	wstring NewFileName = GetOutName(wstring(argv[1]));
	auto file = _wfopen(NewFileName.c_str(), L"wb");

	ULONG iPos = 0;
	for (auto& it : StringPool)
	{
		fprintf(file, ";[0x%08x]%s\r\n", NewReadLineNum[iPos], it.second.c_str());
		iPos++;
	}
	fclose(file);

	return 0;
}

