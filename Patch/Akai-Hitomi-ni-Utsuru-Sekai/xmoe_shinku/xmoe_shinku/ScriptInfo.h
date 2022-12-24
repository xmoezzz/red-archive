#ifndef _ScriptInfo_
#define _ScriptInfo_

#include <Windows.h>
#include "AnzHeader.h"

#pragma pack(push, 4)
struct tTVPXP3ExtractionFilterInfo2
{
	DWORD SizeOfSelf; // structure size of tTVPXP3ExtractionFilterInfo itself
	DWORD Offset; // offset of the buffer data in uncompressed stream position
	void * Buffer; // target data buffer
	DWORD BufferSize; // buffer size in bytes pointed by "Buffer"
	DWORD FileHash; // hash value of the file (since inteface v2)
};
#pragma pack(pop)

static char* ScriptSign = "AnzuShinkuVer1.0";

void WINAPI BufferDecoder(unsigned char* pBuffer, DWORD Size);
void WINAPI ExtractionFilter(tTVPXP3ExtractionFilterInfo2 *Info);

#endif
