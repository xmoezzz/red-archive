#include <Windows.h>
#include "my.h"

#pragma comment(lib, "XmoeCompiler.lib")

class ITextCompiler
{
public:
	virtual BOOL NTAPI LoadTextFromBuffer(PBYTE Buffer, ULONG Size) = 0;
	virtual BOOL NTAPI LoadText(LPCWSTR lpPath) = 0;
	virtual BOOL NTAPI ReCompile() = 0;
	virtual BOOL NTAPI QueryText(ULONG Index, LPSTR lpStr) = 0;
	virtual BOOL NTAPI Release() = 0;
};

EXTERN_C BOOL NTAPI CreateCompiler(ITextCompiler** Server);
EXTERN_C BOOL NTAPI DeleteCompiler();
