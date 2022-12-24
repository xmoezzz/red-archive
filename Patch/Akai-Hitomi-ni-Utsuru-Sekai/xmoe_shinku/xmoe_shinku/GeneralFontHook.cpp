#include "GeneralFontHook.h"

void SetNopCode(BYTE* pnop, size_t size)
{
	ULONG oldProtect;
	VirtualProtect((PVOID)pnop, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	for (size_t i = 0; i<size; i++)
	{
		pnop[i] = 0x90;
	}
}

ULONG  GetDataLen(LPVOID pBaseaddr, LPVOID pReadBuf)
{
	LPBYTE pBase = (LPBYTE)pBaseaddr;
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pReadBuf;
	ULONG uSize = 0;
	PIMAGE_SECTION_HEADER    pSec = (PIMAGE_SECTION_HEADER)(pBase + pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));
	for (int i = 0; i<PIMAGE_FILE_HEADER(pBase + pDosHeader->e_lfanew + 4)->NumberOfSections; ++i)
	{
		if (!strcmp((char*)pSec[i].Name, ".text"))
		{
			uSize += pSec[i].SizeOfRawData;
			break;
		}

	}
	return uSize;
}
//                           0    1     2     3     4     5     6
static BYTE ShellCode[] = { 0x83, 0xD8, 0xFF, 0x85, 0xC0, 0x75, 0x3F };

//return false only if exception arose
ULONG WINAPI FindShellCode(LPVOID lpParam)
{

	ULONG oldProtect;
	ULONG size = GetDataLen(GetModuleHandleW(NULL), GetModuleHandleW(NULL));
	if (size)
	{
		VirtualProtect((PVOID)GetModuleHandleW(NULL), size, PAGE_EXECUTE_READWRITE, &oldProtect);
		BYTE *start = (PBYTE)GetModuleHandleW(NULL);

		ULONG Strlen = strlen((const CHAR*)ShellCode);
		ULONG iPos = 0;
		ULONG zPos = 0;
		BOOL Found = FALSE;
		while (iPos < size)
		{
			if (zPos == Strlen - 1)
			{
				Found = TRUE;
				break;
			}
			if (start[iPos] == ShellCode[zPos])
			{
				iPos++;
				zPos++;
			}
			else
			{
				iPos++;
				zPos = 0;
			}
		}
		if (Found)
		{
			SetNopCode(start + iPos - 1, 2);
		}
	}
	return 1;
}

int SearchDataFromProcessByDllName(BYTE* pSearch, int size)
{
	int i, j;
	ULONG OldProtect;
	BYTE* pOrg;
	BYTE* pPare;
	MODULEINFO mMoudleInfo;
	HMODULE  hMoudle;

	hMoudle = GetModuleHandleW(NULL);

	pOrg = (BYTE*)hMoudle;

	VirtualProtectEx(GetCurrentProcess(), hMoudle, 1, PAGE_EXECUTE_READWRITE, &OldProtect);
	GetModuleInformation(GetCurrentProcess(), hMoudle, &mMoudleInfo, sizeof(mMoudleInfo));

	for (i = 0; i <(int)mMoudleInfo.SizeOfImage; i++)
	{
		pPare = pOrg + i;

		for (j = 0; j < size; j++)
		{
			if (pPare[j] != pSearch[j])
			{
				break;
			}
		}
		if (j == size)
		{
			VirtualProtectEx(GetCurrentProcess(), hMoudle, 1, OldProtect, NULL);

			return (int)(pPare);
		}
	}

	VirtualProtectEx(GetCurrentProcess(), hMoudle, 1, OldProtect, NULL);
	return 0;
}

void WINAPI SetNopCodeFont(BYTE* pnop)
{
	ULONG oldProtect;
	VirtualProtect((PVOID)pnop, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
	if (pnop[0] == ShellCode[5] && pnop[1] == ShellCode[6])
	{
		for (size_t i = 0; i < 2; i++)
		{
			pnop[i] = 0x90;
		}
	}
}

ULONG WINAPI GetFunAddr(LPVOID LpParam)
{
	int Addr;
	Addr = SearchDataFromProcessByDllName(ShellCode, sizeof(ShellCode));

	if (0 == Addr)
	{
		return false;
	}

	ULONG g_Addr_Function = Addr - 2 + 7;
	SetNopCodeFont((PBYTE)g_Addr_Function);
	return true;
}

BOOL WINAPI InstallFont()
{
	GetFunAddr(NULL);
	return TRUE;
}
