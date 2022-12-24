#include "WinFile.h"
#include "lz4.h"

#pragma comment(lib, "lz4.lib")

#define _TAG4(s) ( \
                (((s) >> 24) & 0xFF)       | \
                (((s) >> 8 ) & 0xFF00)     | \
                (((s) << 24) & 0xFF000000) | \
                (((s) << 8 ) & 0x00FF0000) \
                )

#define TAG4(s) _TAG4((ULONG32)(s))

typedef struct XmoeImage
{
	ULONG Maigc;
	ULONG RawSize;
}XmoeImage;


int wmain(int argc, WCHAR* argv[])
{
	if (argc != 2)
		return 0;

	WCHAR   lpFileName[MAX_PATH] = { 0 };
	WinFile File, OutFile;
	PBYTE   InBuffer = nullptr, OutBuffer = nullptr;
	ULONG   InSize, OutSize;

	do
	{
		if (FAILED(File.Open(argv[1], WinFile::FileRead)))
			break;

		InSize = File.GetSize32();
		InBuffer = (PBYTE)GlobalAlloc(0, InSize);
		OutBuffer = (PBYTE)GlobalAlloc(0, InSize * 2);
		if (!InBuffer || !OutBuffer)
			break;

		File.Read(InBuffer, InSize);

		OutSize = LZ4_compress((char*)InBuffer, (char*)OutBuffer, InSize);
		
		wsprintfW(lpFileName, L"%s.comp", argv[1]);
		XmoeImage Header;
		Header.Maigc = TAG4('XMOE');
		Header.RawSize = InSize;
		if (FAILED(OutFile.Open(lpFileName, WinFile::FileWrite)))
			break;

		OutFile.Write((PBYTE)&Header, sizeof(Header));
		OutFile.Write(OutBuffer, OutSize);
		OutFile.Release();

	} while (0);

	if (InBuffer)
		GlobalFree(InBuffer);

	if (OutBuffer)
		GlobalFree(OutBuffer);

	File.Release();
	OutFile.Release();

	return 0;
}

