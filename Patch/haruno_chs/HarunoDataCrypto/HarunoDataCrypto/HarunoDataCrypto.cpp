// HarunoDataCrypto.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>

using std::wstring;

int _tmain(int argc, _TCHAR* argv[])
{
	unsigned long filelong = 0;
	if (argc != 2)
		return 0;

	FILE* fin = _wfopen(argv[1], L"rb");
	fseek(fin, 0, SEEK_END);
	filelong = ftell(fin);
	rewind(fin);
	char *buffer = new char[filelong];
	fread(buffer, 1, filelong, fin);
	fclose(fin);
	unsigned long iPos = 0;
	while (iPos < filelong)
	{
		buffer[iPos] = (~buffer[iPos] & 0xFF);
		buffer[iPos] ^= 0x7F;
		iPos++;
	}

	FILE *out = _wfopen(wstring(wstring(argv[1]) + L".out").c_str(), L"wb");

	fwrite(buffer, 1, filelong, out);
	fclose(out);
	return 0;
	return 0;
}

