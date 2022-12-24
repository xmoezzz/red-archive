// Compiler.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "PsbText.h"
#include <string>

using std::wstring;

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2)
		return 0;

	FILE* File = _wfopen(argv[1], L"rb");
	if (!File)
		return 0;

	PBYTE Buffer;
	ULONG Size;
	if (ExtractPsbText(File, Buffer, Size, argv[1]) == 0)
	{
		FILE* Fout = _wfopen((wstring(argv[1]) + L".out").c_str(), L"wb");
		fwrite(Buffer, 1, Size, Fout);
		fclose(Fout);
	}
	return 0;
}

