#ifndef CXDEC_H
#define CXDEC_H

#include <Windows.h>


typedef struct
{
	DWORD FileHash;
	DWORD Offset;
	PBYTE Buffer;
	DWORD BufferSize;
}CXDEC_INDO, *PCXDEC_INFO;

void ArchiveExtractionFilter(PCXDEC_INFO info);

#endif
