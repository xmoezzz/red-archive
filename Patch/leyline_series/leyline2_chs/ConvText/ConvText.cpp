// ConvText.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <fstream>
#include <Windows.h>
#include <string>
#include <vector>

using std::fstream;
using std::wstring;
using std::string;
using std::vector;


struct LineInfo
{
	string JPLine;
	string CNLine;
};


vector<LineInfo> TextPool;

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 3)
		return 0;

	FILE* fin = _wfopen(argv[1], L"rb");
	fseek(fin, 0, SEEK_END);
	ULONG Size = ftell(fin);
	rewind(fin);
	PBYTE Buffer = new BYTE[Size];
	fread(Buffer, 1, Size, fin);
	fclose(fin);

	ULONG iPos = 0x10;

	while (iPos < Size)
	{
		iPos += 4;
		WCHAR WideLine[2000] = { 0 };
		CHAR  UTF8Line[3000] = { 0 };
		MultiByteToWideChar(932, 0, (LPSTR)&Buffer[iPos], lstrlenA((LPSTR)&Buffer[iPos]), WideLine, 2000);
		WideCharToMultiByte(CP_UTF8, 0, WideLine, lstrlenW(WideLine), UTF8Line, 3000, 0, 0);

		LineInfo info;
		info.JPLine = UTF8Line;
		TextPool.push_back(info);
		iPos += lstrlenA((LPSTR)&Buffer[iPos]) + 1;
	}

	fstream File(argv[2]);
	if (!File)
		return 0;

	iPos = 0;
	string Line;
	while (getline(File, Line))
	{
		TextPool[iPos].CNLine = Line;
		iPos++;
	}
	File.close();

	wstring FileName(L"Out.txt");
	FILE* OutFile = _wfopen(FileName.c_str(), L"wb");
	for (ULONG i = 0; i < TextPool.size(); i++)
	{
		fprintf(OutFile, "[0x%08x]%s\r\n", i, TextPool[i].JPLine.c_str());
		fprintf(OutFile, ";[0x%08x]%s\r\n\r\n", i, TextPool[i].CNLine.c_str());
	}
	fclose(OutFile);
	return 0;
}

