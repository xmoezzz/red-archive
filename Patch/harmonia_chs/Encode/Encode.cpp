// Encode.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "cxdec.h"
#include <string>

using std::wstring;

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2)
		return 0;

	FILE* file = _wfopen(argv[1], L"rb");
	fseek(file, 0, SEEK_END);
	ULONG Size = ftell(file);
	rewind(file);
	PBYTE Buffer = new BYTE[Size];
	fread(Buffer, 1, Size, file);
	fclose(file);

	CXDEC_INDO Info;
	Info.Buffer = Buffer;
	Info.BufferSize = Size;
	Info.FileHash = 114514;
	Info.Offset = 0;

	ArchiveExtractionFilter(&Info);

	wstring FileName(argv[1]);
	FileName += L".out";
	FILE* fout = _wfopen(FileName.c_str(), L"wb");
	fwrite(Buffer, 1, Size, fout);
	fclose(fout);

	return 0;
}

