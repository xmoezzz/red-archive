#ifndef _MinNtdll_
#define _MinNtdll_

#include <Windows.h>

typedef struct _UNICODE_STRING 
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

NTSYSAPI void      WINAPI RtlInitUnicodeString(PUNICODE_STRING, PCWSTR);
NTSYSAPI NTSTATUS WINAPI RtlUnicodeToMultiByteN(LPSTR, DWORD, LPDWORD, LPCWSTR, DWORD);
NTSYSAPI VOID RtlInitEmptyUnicodeString(
IN OUT PUNICODE_STRING  DestinationString,
IN PCWSTR  Buffer,
IN USHORT  BufferSize
);


#endif
