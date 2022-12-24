#include <Windows.h>


#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker, "/SECTION:.Xmoe,ERW /MERGE:.text=.Xmoe")
#pragma comment(linker, "/ENTRY:DllMainEntry")

#pragma comment(linker, "/EXPORT:GetInfo=_GetInfo@4,PRIVATE")


#define _TAG4(s) ( \
                (((s) >> 24) & 0xFF)       | \
                (((s) >> 8 ) & 0xFF00)     | \
                (((s) << 24) & 0xFF000000) | \
                (((s) << 8 ) & 0x00FF0000) \
                )
#define TAG4(s) _TAG4((DWORD)(s))

EXTERN_C __declspec(dllexport) void NTAPI GetInfo(PBYTE Buffer)
{
	*(PDWORD)(Buffer + 0) = TAG4('xMoE');
	*(PDWORD)(Buffer + 4) = TAG4('lNik');
}

BOOL APIENTRY DllMainEntry(HMODULE hModule, DWORD Reason, LPVOID lpReserved )
{
	return TRUE;
}

