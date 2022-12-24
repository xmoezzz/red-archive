#include "stdafx.h"
#include "DebugLoad.h"

#include <string>

using std::wstring;

char* DebugLoad(const wchar_t* name, size_t& size_out)
{
	FILE* fin = nullptr;
	wchar_t FileName[256] = { 0 };
	wcscat(FileName, L"Scr\\");
	wcscat(FileName, name);
	_wfopen_s(&fin, FileName, L"rb");
	if (fin == nullptr)
	{
		MessageBoxW(NULL, wstring(L"无法打开脚本 ： " + wstring(name)).c_str(), L"Error", MB_OK);
		//ExitProcess(-1);
	}
	fseek(fin, 0, SEEK_END);
	size_t size = ftell(fin);
	rewind(fin);
	char sign[2];
	fread(sign, 1, 2, fin);
	char* tBuffer = new char[size+2];
	memset(tBuffer, 0, size + 2);
	if ((unsigned char)sign[0] == 0xFF && (unsigned char)sign[1] == 0xFE)
	{
		//UTF16-le
		fseek(fin, 2, SEEK_SET);
		fread(tBuffer, 1, size - 2, fin);
	}
	else
	{
		rewind(fin);
		fread(tBuffer, 1, size, fin);
	}
	
	fclose(fin);

	size_out = size;
	return tBuffer;
}
