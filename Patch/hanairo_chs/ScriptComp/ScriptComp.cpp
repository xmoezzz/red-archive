#include <WinFile.h>
#include "Compressor.h"

#define MY_BURIKO_SCRIPT_MAGIC "STmoeSTmoeChu>_<" //@16

int wmain(int argc, WCHAR* argv[])
{
	if (argc != 2)
		return 0;

	WinFile InFile, OutFile;
	PBYTE InBuffer = nullptr, OutBuffer = nullptr;
	ULONG InSize = 0, OutSize = 0;
	WCHAR szFileName[MAX_PATH] = { 0 };
	BOOL  bResult;
	do
	{
		if (InFile.Open(argv[1], WinFile::FileRead) != S_OK)
			break;

		InSize = InFile.GetSize32();
		InBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), 0, InSize);
		OutBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), 0, InSize * 2);

		InFile.Read(InBuffer, InSize);
		if (!InBuffer)
			break;

		wsprintfW(szFileName, L"%s_out", argv[1]);
		if (OutFile.Open(szFileName, WinFile::FileWrite) != S_OK)
			break;

		bResult = UCL_NRV2E_Compress(InBuffer, InSize, OutBuffer, &OutSize, 10);
		if (!bResult)
		{
			MessageBoxW(NULL, L"Failed to compress", 0, 0);
			break;
		}

		OutFile.Write((PBYTE)MY_BURIKO_SCRIPT_MAGIC, sizeof(MY_BURIKO_SCRIPT_MAGIC));
		OutFile.Write(OutBuffer, OutSize);

	} while (0);

	if (InBuffer)
		HeapFree(GetProcessHeap(), 0, InBuffer);
	if (OutBuffer)
		HeapFree(GetProcessHeap(), 0, OutBuffer);

	InFile.Release();
	OutFile.Release();
	return 0;
}

