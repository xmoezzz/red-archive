#ifndef __Common_H__
#define __Common_H__

#include <Windows.h>


typedef struct xHeader
{
	DWORD OriginalLength;
	DWORD CompressedLength;
};

typedef struct FilePair
{
	DWORD Offset;
	DWORD Length;
	DWORD OriginalLength;
	DWORD OriginalOffset;
	DWORD Key;
};

void __stdcall EncodeHeader(char *cHeader);
void __stdcall CalculateXORTable();

#endif
