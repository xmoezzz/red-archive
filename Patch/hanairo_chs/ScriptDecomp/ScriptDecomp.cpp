#include <WinFile.h>

#define MY_BURIKO_SCRIPT_MAGIC "STmoeSTmoeChu>_<" //@16

#define FASTCALL __fastcall

#ifndef ASM
#define ASM __declspec(naked)
#endif /* ASM */

#ifndef NAKED
#define NAKED __declspec(naked)
#endif /* ASM */

#ifndef INLINE_ASM
#define INLINE_ASM __asm
#endif


ASM ULONG_PTR FASTCALL UCL_NRV2E_DecompressASMFast32(PVOID /* pvInput */, PVOID /* pvOutput */)
{
	INLINE_ASM
	{
		add     esp, -0x18;
		mov[esp + 0x00], ebx;
		mov[esp + 0x04], ebp;
		mov[esp + 0x08], esi;
		mov[esp + 0x0C], edi;
		mov[esp + 0x10], edx;
		cld;
		mov     esi, ecx;
		mov     edi, edx;
		or      ebp, 0xFFFFFFFF;
		xor     ecx, ecx;
		jmp L029;

		INLINE_ASM __emit 0x8D INLINE_ASM __emit 0xB4 INLINE_ASM __emit 0x26 INLINE_ASM __emit 0x00 INLINE_ASM __emit 0x00 INLINE_ASM __emit 0x00 INLINE_ASM __emit 0x00;   // lea esi, [esi]
		INLINE_ASM __emit 0x8D INLINE_ASM __emit 0xB4 INLINE_ASM __emit 0x26 INLINE_ASM __emit 0x00 INLINE_ASM __emit 0x00 INLINE_ASM __emit 0x00 INLINE_ASM __emit 0x00;   // lea esi, [esi]
	L022:
		mov     al, byte ptr[esi];
		inc     esi;
		mov     byte ptr[edi], al;
		inc     edi;
	L026:
		add     ebx, ebx;
		jnb L033;
		jnz L022;
	L029:
		mov     ebx, dword ptr[esi];
		sub     esi, -0x4;
		adc     ebx, ebx;
		jb L022;
	L033:
		mov     eax, 0x1;
	L034:
		add     ebx, ebx;
		jnz L039;
		mov     ebx, dword ptr[esi];
		sub     esi, -0x4;
		adc     ebx, ebx;
	L039:
		adc     eax, eax;
		add     ebx, ebx;
		jnb L047;
		jnz L055;
		mov     ebx, dword ptr[esi];
		sub     esi, -0x4;
		adc     ebx, ebx;
		jb L055;
	L047:
		dec     eax;
		add     ebx, ebx;
		jnz L053;
		mov     ebx, dword ptr[esi];
		sub     esi, -0x4;
		adc     ebx, ebx;
	L053:
		adc     eax, eax;
		jmp L034;
	L055:
		sub     eax, 0x3;
		jb L072;
		shl     eax, 0x8;
		mov     al, byte ptr[esi];
		inc     esi;
		xor     eax, 0xFFFFFFFF;
		je L120;
		sar     eax, 1;
		mov     ebp, eax;
		jnb L078;
	L065:
		add     ebx, ebx;
		jnz L070;
		mov     ebx, dword ptr[esi];
		sub     esi, -0x4;
		adc     ebx, ebx;
	L070:
		adc     ecx, ecx;
		jmp L099;
	L072:
		add     ebx, ebx;
		jnz L077;
		mov     ebx, dword ptr[esi];
		sub     esi, -0x4;
		adc     ebx, ebx;
	L077:
		jb L065;
	L078:
		inc     ecx;
		add     ebx, ebx;
		jnz L084;
		mov     ebx, dword ptr[esi];
		sub     esi, -0x4;
		adc     ebx, ebx;
	L084:
		jb L065;
	L085:
		add     ebx, ebx;
		jnz L090;
		mov     ebx, dword ptr[esi];
		sub     esi, -0x4;
		adc     ebx, ebx;
	L090:
		adc     ecx, ecx;
		add     ebx, ebx;
		jnb L085;
		jnz L098;
		mov     ebx, dword ptr[esi];
		sub     esi, -0x4;
		adc     ebx, ebx;
		jnb L085;
	L098:
		add     ecx, 0x2;
	L099:
		cmp     ebp, -0x500;
		adc     ecx, 0x2;
		lea     edx, dword ptr[edi + ebp];
		cmp     ebp, -0x4;
		jbe L111;
	L104:
		mov     al, byte ptr[edx];
		inc     edx;
		mov     byte ptr[edi], al;
		inc     edi;
		dec     ecx;
		jnz L104;
		jmp L026;
	L111:
		mov[esp + 0x14], ecx;
		and     ecx, ~3;
		jecxz   L111_END;
	L111_:
		mov     eax, dword ptr[edx];
		add     edx, 0x4;
		mov     dword ptr[edi], eax;
		add     edi, 0x4;
		sub     ecx, 0x4;
		ja L111_;

	L111_END:

		mov     ecx, [esp + 0x14];
		and     ecx, 3;
		jecxz   L111_LOOP_2_END;
		mov[esp + 0x14], ecx;

	L111_LOOP_2:
		mov     al, [edx];
		mov[edi], al;
		inc     edx;
		inc     edi;
		loop    L111_LOOP_2;

		mov     ecx, [esp + 0x14];
		sub     edx, ecx;
		add     edx, 4;

	L111_LOOP_2_END:

		//        add     edi, ecx;
		xor     ecx, ecx;
		jmp L026;
	L120:
		mov     eax, edi;
		mov     ebx, [esp + 0x00];
		mov     ebp, [esp + 0x04];
		mov     esi, [esp + 0x08];
		mov     edi, [esp + 0x0C];
		sub     eax, [esp + 0x10];
		add     esp, 0x18;
		ret;
	}
}


int wmain(int argc, WCHAR* argv[])
{
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
		OutBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), 0, 1024 * 1024 * 64);

		InFile.Read(InBuffer, InSize);
		if (!InBuffer)
			break;

		wsprintfW(szFileName, L"%s.in", argv[1]);
		if (OutFile.Open(szFileName, WinFile::FileWrite) != S_OK)
			break;

		if (InSize > sizeof(MY_BURIKO_SCRIPT_MAGIC) &&
			!memcmp(InBuffer, MY_BURIKO_SCRIPT_MAGIC, sizeof(MY_BURIKO_SCRIPT_MAGIC)))
		{
			OutSize = UCL_NRV2E_DecompressASMFast32((PBYTE)InBuffer + sizeof(MY_BURIKO_SCRIPT_MAGIC), OutBuffer);
		}
		else
		{
			MessageBoxW(NULL, L"Error Type", 0, 0);
			break;
		}
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

