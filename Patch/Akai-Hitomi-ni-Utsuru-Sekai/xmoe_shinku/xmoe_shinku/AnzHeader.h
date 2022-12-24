#ifndef _AnzHeader_
#define _AnzHeader_

#include <Windows.h>


typedef struct AnzScript
{
	BYTE Sign[16];
	DWORD OriginalSize;
	DWORD Key;
	DWORD BlockKey;
	DWORD Padding;
}AnzScript;

#endif
