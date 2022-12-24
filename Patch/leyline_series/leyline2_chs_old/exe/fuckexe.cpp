#include <windows.h>
#include <cstdio>
#include <string>

using std::wstring;

int wmain(int argc, WCHAR** argv) 
{
	if(argc != 2)
	{
		return 0;
	}
	
	FILE* fin = _wfopen(argv[1], L"rb");
	fseek(fin, 0, 2);
	unsigned FileSize = ftell(fin);
	rewind(fin);
	unsigned char* Buff = new unsigned char[FileSize];
	fread(Buff, 1, FileSize, fin);
	fclose(fin);
	
	unsigned iPos = 0xB2C00;
	while(iPos <= 0xB5EE3)
	{
		Buff[iPos] = 0;
		iPos++;
	}
	
	FILE* out = fopen("GameCore.dll", "wb");
	fwrite(Buff, 1, FileSize, out);
	fclose(out);
	delete[] Buff;
	return 0;
}
