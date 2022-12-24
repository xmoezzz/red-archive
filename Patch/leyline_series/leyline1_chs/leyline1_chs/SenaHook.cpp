#include "SenaHook.h"
#include "MyHook.h"
#include "my.h"
#include <new>
#include "FileManager.h"
#include <WinFile.h>

//#include "VMProtectSDK.h"

SenaHook* SenaHook::m_Inst = nullptr;

//ML_OVERLOAD_NEW;


VOID NTAPI OutputInfo(LPCWSTR Info)
{
	DWORD   nRet;
	HANDLE  hOutput;

	hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleW(hOutput, Info, StrLengthW(Info), &nRet, NULL);
	WriteConsoleW(hOutput, L"\n", 1, &nRet, NULL);
}


VOID NTAPI OutputInfo(LPCSTR Info)
{
	DWORD   nRet;
	HANDLE  hOutput;

	hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleA(hOutput, Info, StrLengthA(Info), &nRet, NULL);
	WriteConsoleW(hOutput, L"\n", 1, &nRet, NULL);
}


VOID NTAPI OutputInfo(LPCSTR Info, ULONG CodePage)
{
	DWORD nRet;
	WCHAR wInfo[2000] = { 0 };
	MultiByteToWideChar(CodePage, 0, Info, lstrlenA(Info), wInfo, 2000);
	HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleW(hStd, wInfo, lstrlenW(wInfo), &nRet, NULL);
	WriteConsoleW(hStd, L"\n", 1, &nRet, NULL);
}

SenaHook::SenaHook() :
hSelfModule(NULL),
hHostModule(NULL),
XmoeCompiler(NULL),
OldLoadLibraryExA(NULL),
OldLdrLoadDll(NULL),
OldGetGlyphOutlineA(NULL),
OldGetACP(NULL),
OldGetOEMCP(NULL),
FileSystemInit(FALSE),
DllPatchFlag(FALSE),
hWnd(NULL),
IsWindowInited(FALSE),
OldWindowProc(NULL)
{
	if (!CreateCompiler(&XmoeCompiler))
	{
		MessageBoxW(NULL, L"Failed to create X'moe compiler instance!",
			L"X'moe common lib", MB_OK | MB_ICONERROR);
		Ps::ExitProcess(-1);
	}
}

SenaHook::~SenaHook()
{
	if (!XmoeCompiler)
	{
		DeleteCompiler();
		XmoeCompiler = NULL;
	}
}

SenaHook* SenaHook::GetSenaHook()
{
	if (!m_Inst)
		m_Inst = new SenaHook;

	return m_Inst;
}

NTSTATUS NTAPI SenaHook::SetSelfModule(HMODULE  hModule)
{
	hSelfModule = hModule;
	return STATUS_SUCCESS;
}


NTSTATUS NTAPI SenaHook::GetSelfModule(HMODULE& hModule)
{
	hModule = hSelfModule;
	return IsValidImage(hModule) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

NTSTATUS NTAPI SenaHook::GetHostModule(HMODULE& hModule)
{
	if (!hHostModule)
		hHostModule = GetModuleHandleW(NULL);

	hModule = hHostModule;

	return STATUS_SUCCESS;
}

DWORD WINAPI HookGetGlyphOutlineA(
	_In_        HDC            hdc,
	_In_        UINT           uChar,
	_In_        UINT           uFormat,
	_Out_       LPGLYPHMETRICS lpgm,
	_In_        DWORD          cbBuffer,
	_Out_       LPVOID         lpvBuffer,
	_In_  const MAT2           *lpmat2
	)
{
	ULONG     len;
	CHAR      mbchs[2];
	HFONT     hFont, hOldFont;
	LOGFONTW* lplf;
	UINT      cp = 936;

	if (IsDBCSLeadByteEx(cp, uChar >> 8))
	{
		len = 2;
		mbchs[0] = (uChar & 0xff00) >> 8;
		mbchs[1] = (uChar & 0xff);
	}
	else
	{
		len = 1;
		mbchs[0] = (uChar & 0xff);
	}

	uChar = 0;
	MultiByteToWideChar(cp, 0, mbchs, len, (LPWSTR)&uChar, 1);

	DWORD Result = 0;

#if 0
	{
		LOGFONTW FontInfo[1];
		RtlZeroMemory(FontInfo, sizeof(FontInfo[0]));
		auto hOldFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
		GetObject(hOldFont, sizeof(LOGFONTW), FontInfo);
	}
#endif

	//A2 E1 
	if (LOWORD(uChar) == (WORD)0x2468)
	{
		lplf = (LOGFONTW*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LOGFONTW));

		hOldFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
		GetObject(hOldFont, sizeof(LOGFONTW), lplf);
		LOGFONTW Info = { 0 };

		Info.lfHeight = lplf->lfHeight;
		Info.lfWidth = lplf->lfWidth;
		Info.lfEscapement = lplf->lfEscapement;
		Info.lfOrientation = lplf->lfOrientation;
		Info.lfWeight = lplf->lfWeight;
		Info.lfItalic = lplf->lfItalic;
		Info.lfUnderline = lplf->lfUnderline;
		Info.lfStrikeOut = lplf->lfStrikeOut;
		Info.lfOutPrecision = lplf->lfOutPrecision;
		Info.lfClipPrecision = lplf->lfClipPrecision;
		Info.lfQuality = lplf->lfQuality;
		Info.lfPitchAndFamily = lplf->lfPitchAndFamily;
		lstrcpyW(Info.lfFaceName, L"MS Gothic");
		lplf->lfCharSet = SHIFTJIS_CHARSET;

		hFont = CreateFontIndirectW(&Info);

		hOldFont = (HFONT)SelectObject(hdc, hFont);
		//6A 26 
		Result = GetGlyphOutlineW(hdc, (UINT)0x266A, uFormat,
			lpgm, cbBuffer, lpvBuffer, lpmat2);

		SelectObject(hdc, hOldFont);
		DeleteObject(hFont);
		HeapFree(GetProcessHeap(), 0, lplf);
	}
	else if (LOWORD(uChar) == (WORD)0x2467)
	{
		lplf = (LOGFONTW*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LOGFONTW));

		hOldFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
		GetObject(hOldFont, sizeof(LOGFONTW), lplf);
		LOGFONTW Info = { 0 };

		Info.lfHeight = lplf->lfHeight;
		Info.lfWidth = lplf->lfWidth;
		Info.lfEscapement = lplf->lfEscapement;
		Info.lfOrientation = lplf->lfOrientation;
		Info.lfWeight = lplf->lfWeight;
		Info.lfItalic = lplf->lfItalic;
		Info.lfUnderline = lplf->lfUnderline;
		Info.lfStrikeOut = lplf->lfStrikeOut;
		Info.lfOutPrecision = lplf->lfOutPrecision;
		Info.lfClipPrecision = lplf->lfClipPrecision;
		Info.lfQuality = lplf->lfQuality;
		Info.lfPitchAndFamily = lplf->lfPitchAndFamily;
		lstrcpyW(Info.lfFaceName, L"MS Gothic");
		lplf->lfCharSet = SHIFTJIS_CHARSET;

		HFONT hFont = CreateFontIndirectW(&Info);
		hOldFont = (HFONT)SelectObject(hdc, hFont);
		Result = GetGlyphOutlineW(hdc, (UINT)0x2200, uFormat,
			lpgm, cbBuffer, lpvBuffer, lpmat2);

		SelectObject(hdc, hOldFont);
		DeleteObject(hFont);
		HeapFree(GetProcessHeap(), 0, lplf);
	}
	else
	{
		Result = GetGlyphOutlineW(hdc, uChar, uFormat,
			lpgm, cbBuffer, lpvBuffer, lpmat2);


#if 0
		if (lpvBuffer)
		{
			int nByteCount = ((lpgm->gmBlackBoxX + 31) >> 5) << 2;

			for (int i = 0; i < lpgm->gmBlackBoxY; i++)
			{
				for (int j = 0; j < nByteCount; j++)
				{
					BYTE btCode = ((PBYTE)lpvBuffer)[i* nByteCount + j];
					for (int k = 0; k < 8; k++)
					{
						if (btCode & (0x80 >> k))
							PrintConsoleW(_T("1"));
						else
							PrintConsoleW(_T("0"));
					}
				}
				PrintConsoleW(L"\n");
			}
		}
#endif
	}

	return Result;
}


int WINAPI HookMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	WCHAR  Text[400];
	WCHAR  Caption[200];

	RtlZeroMemory(Text, countof(Text) * sizeof(Text[0]));
	RtlZeroMemory(Caption, countof(Caption) * sizeof(Caption[0]));

	MultiByteToWideChar(CP_ACP, 0, lpCaption, StrLengthA(lpCaption), Caption, countof(Caption));
	MultiByteToWideChar(932,    0, lpText,    StrLengthA(lpText),    Text,    countof(Text));

	return MessageBoxW(hWnd, Text, Caption, uType);
}


BOOL WINAPI HookSetWindowTextA(HWND hWnd, LPCSTR lpString)
{
	if (hWnd == SenaHook::GetSenaHook()->hWnd && SenaHook::GetSenaHook()->IsWindowInited)
	{
		return SetWindowTextW(hWnd, L"时钟机关的Ley-line  -黄昏时的境界线-");
	}
	return SetWindowTextA(hWnd, lpString);
}

PVOID pfPalLoadSprite = NULL;
typedef DWORD(*PfunPalLoadSprite) (DWORD, CHAR* FileName, PBYTE DataBuffers, DWORD DataSize);
VOID GetBMPData(PBYTE BMPImage, PLONG pdwWidth, PLONG pdwHeight, PULONG pBitCount, PBYTE* OutputBits, DWORD* BitsSize)
{
	PBITMAPFILEHEADER    Header;
	PBITMAPINFOHEADER    HeaderInfo;
	PBYTE                SourceBits, TargetBits;
	LONG                 TargetAlignWidth, SourceAlignWidth, BitCount;
	ULONG                TargetBitsLength;
	PBYTE                SourceLines, TargetLines;
	LONG                 ProcessHeight, ProcessWidth;


	Header = (PBITMAPFILEHEADER)BMPImage;
	HeaderInfo = (PBITMAPINFOHEADER)(BMPImage + sizeof(BITMAPFILEHEADER));
	SourceBits = &BMPImage[Header->bfOffBits];
	BitCount = HeaderInfo->biBitCount;
	TargetAlignWidth = (HeaderInfo->biWidth * 32 + 31) / 32;
	SourceAlignWidth = (HeaderInfo->biWidth*BitCount + 31) / 32;
	TargetBitsLength = 4 * TargetAlignWidth * HeaderInfo->biHeight;

	TargetBits = (PBYTE)HeapAlloc(GetProcessHeap(), 0, TargetBitsLength);

	if (BitCount == 32)
	{
		RtlCopyMemory(TargetBits, SourceBits, TargetBitsLength);
	}
	else
	{
		for (ProcessHeight = 0; ProcessHeight < HeaderInfo->biHeight; ProcessHeight++)
		{
			SourceLines = &SourceBits[SourceAlignWidth * ProcessHeight * 4];
			TargetLines = &TargetBits[TargetAlignWidth * ProcessHeight * 4];

			for (ProcessWidth = 0; ProcessWidth < HeaderInfo->biWidth; ProcessWidth++)
			{
				TargetLines[0] = SourceLines[0];
				TargetLines[1] = SourceLines[1];
				TargetLines[2] = SourceLines[2];

				if (BitCount == 32)
					TargetLines[3] = SourceLines[3];
				else
					TargetLines[3] = 0xFF;

				SourceLines += (BitCount / 8);
				TargetLines += 4;
			}
		}
	}

	*pdwWidth = HeaderInfo->biWidth;
	*pdwHeight = HeaderInfo->biHeight;
	*pBitCount = BitCount;
	*OutputBits = TargetBits;
	*BitsSize = TargetBitsLength;
}

VOID ConvertBMPToDIB(PBYTE Data, ULONG Width, ULONG Height, ULONG BitCount)
{
	PBYTE   TempBuffer;
	ULONG   WidthLen, BufferSize, AlignWidth;

	AlignWidth = (Width * 32 + 31) / 32;
	BufferSize = 4 * AlignWidth * Height;
	TempBuffer = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BufferSize);

	WidthLen = Width * (BitCount / 8);
	for (ULONG i = 0; i < Height; i++)
	{
		RtlCopyMemory(&TempBuffer[(((Height - i) - 1) * WidthLen)], &Data[WidthLen * i], WidthLen);
	}
	RtlCopyMemory(Data, TempBuffer, BufferSize);

	HeapFree(GetProcessHeap(), 0, TempBuffer);
}


BYTE* g_ImageBits = NULL;
ULONG g_ImageBitsLength = 0;
LONG  g_ImageWidth = 0;
LONG  g_ImageHeight = 0;
ULONG g_ImageBitCount = 32;

DWORD PalLoadSprEx(DWORD unk, CHAR* FileName, PBYTE DataBuffers, DWORD DataSize)
{
	DWORD    Result;
	CHAR     ConvertName[MAX_PATH];
	ULONG    NameLength;

	RtlZeroMemory(ConvertName, MAX_PATH);
	StrCopyA(ConvertName, FileName);
	NameLength = StrLengthA(ConvertName);

	ConvertName[NameLength - 3] = 'b';
	ConvertName[NameLength - 2] = 'm';
	ConvertName[NameLength - 1] = 'p';

	ULONG FileSize = 0;
	PBYTE FileBuffer = 0;

	//OutputInfo(ConvertName);
	if (FileManager::GetFileManager()->QueryFile(ConvertName, FileBuffer, FileSize))
	{
		//OutputInfo("Load image");
		GetBMPData(FileBuffer, &g_ImageWidth, &g_ImageHeight, &g_ImageBitCount, &g_ImageBits, &g_ImageBitsLength);
		ConvertBMPToDIB(g_ImageBits, g_ImageWidth, g_ImageHeight, 32);
	}

	Result = (PfunPalLoadSprite(pfPalLoadSprite))(unk, FileName, DataBuffers, DataSize);

	if (FileBuffer)
	{
		HeapFree(GetProcessHeap(), 0, FileBuffer);
		FileBuffer = NULL;
	}

	if (g_ImageBits)
	{
		HeapFree(GetProcessHeap(), 0, g_ImageBits);
		g_ImageBits = NULL;
	}

	return Result;
}

VOID FixWidth(PBYTE SourceDIB, ULONG Width, ULONG Height, ULONG BitCount, ULONG NewWidth, PBYTE* Output, PDWORD OutputLength)
{
	PBYTE      TmpBuffer, SourceLines, TargetLines;
	ULONG      nAlignWidth, nNewAlignWidth;

	nAlignWidth = (Width * 32 + 31) / 32;
	nNewAlignWidth = (NewWidth * 32 + 31) / 32;
	*OutputLength = 4 * nNewAlignWidth * Height;
	*Output = (PBYTE)HeapAlloc(GetProcessHeap(), 0, *OutputLength);
	TmpBuffer = *Output;

	RtlZeroMemory(TmpBuffer, *OutputLength);

	for (ULONG i = 0; i<Height; i++)
	{
		SourceLines = &SourceDIB[nAlignWidth * i * 4];
		TargetLines = &TmpBuffer[nNewAlignWidth * i * 4];

		for (ULONG j = 0; j<Width; j++)
		{
			TargetLines[0] = SourceLines[0];
			TargetLines[1] = SourceLines[1];
			TargetLines[2] = SourceLines[2];
			TargetLines[3] = SourceLines[3];
			SourceLines += (BitCount / 8);
			TargetLines += 4;
		}
	}
}

ULONG FixedList[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
#define FixedListLength 13

void PALCopyImage(PBYTE RenderBuffer, ULONG NewWidth)
{
	PBYTE    Output = NULL;
	ULONG    OutputLength;
	ULONG    FixedWidth = NewWidth;
	BOOL     Found = FALSE;

	for (ULONG i = 1; i < FixedListLength; i++)
	{
		if (FixedList[i - 1] == FixedWidth || FixedList[i] == FixedWidth)
		{
			Found = TRUE;
			break;
		}
		else if (FixedList[i - 1] < FixedWidth && FixedList[i] > FixedWidth)
		{
			FixedWidth = FixedList[i];
			Found = TRUE;
			break;
		}
	}
	if (g_ImageBits && Found)
	{

		FixWidth(g_ImageBits, g_ImageWidth, g_ImageHeight, 32, FixedWidth, &Output, &OutputLength);


#if 0
		if (g_ImageBitCount != 32)
		{
			RtlCopyMemory(draw_buf, g_ImageBits, OutputLength);
		}
		else
		{
			nNewAlignWidth = (new_width * 32 + 31) / 32;

			for (i = 0; i<g_ImageHeight; i++)
			{
				SourceLines = &Output[nNewAlignWidth * i * 4];
				TargetLines = &draw_buf[nNewAlignWidth * i * 4];
				for (j = 0; j<new_width; j++)
				{
					TargetLines[0] = SourceLines[0];
					TargetLines[1] = SourceLines[1];
					TargetLines[2] = SourceLines[2];
					TargetLines[3] = SourceLines[3];

					SourceLines += 4;
					TargetLines += 4;
				}
			}
		}
#else
		RtlCopyMemory(RenderBuffer, Output, OutputLength);
#endif
	}
	if (Output)
	{
		HeapFree(GetProcessHeap(), 0, Output);
		Output = NULL;
	}
}



ULONG_PTR g_ScreenWidth;

PVOID g_FilterDrawRet1, g_FilterDrawCall1;
PVOID g_FilterDrawRet2, g_FilterDrawCall2;
PVOID g_FilterDrawRet3, g_FilterDrawCall3;
PVOID g_FilterDrawRet4, g_FilterDrawCall4;


ASM VOID UpdateGraphFilter4()
{
	INLINE_ASM
	{
		push[esp + 4];
		pop  g_ScreenWidth;
		call g_FilterDrawCall4;
		pushad;
		push g_ScreenWidth;
		push dword ptr[ebp + 8];
		call PALCopyImage;
		add esp, 0x8;
		popad;
		jmp g_FilterDrawRet4;
	}
}

ASM VOID UpdateGraphFilter3()
{
	INLINE_ASM
	{
		push[esp + 0xC];
		pop  g_ScreenWidth;
		call g_FilterDrawCall3;
		pushad;
		push g_ScreenWidth;
		push dword ptr[ebp + 8];
		call PALCopyImage;
		add esp, 0x8;
		popad;
		jmp g_FilterDrawRet3;
	}
}

ASM VOID UpdateGraphFilter2()
{
	INLINE_ASM
	{
		push[esp + 0xC];
		pop  g_ScreenWidth;
		call g_FilterDrawCall2;
		pushad;
		push g_ScreenWidth;
		push dword ptr[ebp + 8];
		call PALCopyImage;
		add esp, 0x8;
		popad;
		jmp g_FilterDrawRet2;
	}
}


ASM VOID UpdateGraphFilter1()
{
	INLINE_ASM
	{
		push[esp + 4];
		pop  g_ScreenWidth;
		call g_FilterDrawCall1;
		pushad;
		push g_ScreenWidth;
		push dword ptr[ebp + 8];
		call PALCopyImage;
		add esp, 0x8;
		popad;
		jmp g_FilterDrawRet1;
	}
}


VOID WINAPI HookPalFontSetType(HMODULE hModule)
{
	BYTE* pfPalFontSetType = (BYTE*)GetProcAddress((HMODULE)hModule, "PalFontSetType");
	SetNopCode((pfPalFontSetType + 0x1E), 7);
}


VOID ReplaceJump(PBYTE src, PBYTE dst)
{
	DWORD   OldProtect;
	VirtualProtect((LPVOID)src, 10, PAGE_EXECUTE_READWRITE, &OldProtect);

	src[0] = 0xE9;
	*(PDWORD)&src[1] = (DWORD)(dst - src - 5);
}

PBYTE GetCALLTarget(PBYTE cal)
{
	return cal + *(PDWORD)&cal[1] + 5;
}


HFONT WINAPI HookCreateFontA(_In_ int cHeight, _In_ int cWidth, _In_ int cEscapement, _In_ int cOrientation, _In_ int cWeight, _In_ DWORD bItalic,
	_In_ DWORD bUnderline, _In_ DWORD bStrikeOut, _In_ DWORD iCharSet, _In_ DWORD iOutPrecision, _In_ DWORD iClipPrecision,
	_In_ DWORD iQuality, _In_ DWORD iPitchAndFamily, _In_opt_ LPCSTR pszFaceName)
{

	auto SelectFont = [&](Void)->LPCWSTR
	{
		return L"黑体";

		switch (Nt_CurrentPeb()->OSMajorVersion)
		{
		case 6:
			if (Nt_CurrentPeb()->OSMinorVersion < 4)
				return L"黑体";
			else
				return L"等线";
			break;

		case 10:
			return L"等线";
			break;

		default:
			return L"黑体";
			break;
		}
	};

	//PrintConsoleW(L"create\n");

	return CreateFontW(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic,
		bUnderline, bStrikeOut, GB2312_CHARSET, iOutPrecision, iClipPrecision,
		iQuality, iPitchAndFamily, SelectFont());
}


HMODULE WINAPI HookLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	NTSTATUS        Status;
	HMODULE         hModule;
	ULONG_PTR       LengthOfString;
	LPCSTR          TagName;
	PBYTE           PatchAddr;
	PVOID           Target;

	hModule = SenaHook::GetSenaHook()->OldLoadLibraryExA(lpLibFileName, hFile, dwFlags);

	LOOP_ONCE
	{
		if (!hModule)
		break;

		if (SenaHook::GetSenaHook()->DllPatchFlag)
			break;

		if (!GetProcAddress(hModule, "PalFontSetType"))
			break;

		LengthOfString = StrLengthA(lpLibFileName);

		if (LengthOfString < 7)
			break;

		TagName = &lpLibFileName[LengthOfString - 7];

		if (!lstrcmpiA(TagName, "PAL.dll"))
		{

			BYTE SpacePatch[] = { 0x81, 0xFF, 0xA1, 0xA1 };

			CODE_PATCH_DATA c[] =
			{
				{ PtrAdd(hModule, 0x8A2A), SpacePatch, sizeof(SpacePatch) },
				{ PtrAdd(hModule, 0x82ED), SpacePatch, sizeof(SpacePatch) }
			};

			IAT_PATCH_DATA f[] = 
			{
				{ GetModuleHandleW(L"PAL.DLL"), MessageBoxA,      HookMessageBoxA,      "USER32.DLL" },
				{ GetModuleHandleW(L"PAL.DLL"), SetWindowTextA,   HookSetWindowTextA,   "USER32.DLL" },
				{ GetModuleHandleW(L"PAL.DLL"), CreateFontA,      HookCreateFontA,      "GDI32.DLL"  }
				//{ GetModuleHandleW(L"PAL.DLL"), GetGlyphOutlineA, HookGetGlyphOutlineA, "GDI32.DLL"  }
			};

			CodePatchMemory(c, countof(c));
			HookPalFontSetType(hModule);

			IATPatchMemory(f, countof(f));

#if 1
			pfPalLoadSprite = (PBYTE)hModule + 0x22DA0;
			Target = pfPalLoadSprite;

			INLINE_PATCH_DATA p[] =
			{
				{ Target, PalLoadSprEx, &pfPalLoadSprite }
			};

			Status = InlinePatchMemory(p, countof(p));
			if (!IsStatusSuccess(Status))
			{
				MessageBoxW(NULL, L"Failed to launch graphics system", L"Sofpal", MB_OK | MB_ICONERROR);
			}

			PatchAddr = (PBYTE)hModule + 0x1C601;
			g_FilterDrawRet1 = PatchAddr + 5;
			g_FilterDrawCall1 = GetCALLTarget(PatchAddr);
			ReplaceJump(PatchAddr, (PBYTE)&UpdateGraphFilter1);

			PatchAddr = (PBYTE)hModule + 0x21108;
			g_FilterDrawRet2 = PatchAddr + 5;
			g_FilterDrawCall2 = GetCALLTarget(PatchAddr);
			ReplaceJump(PatchAddr, (PBYTE)&UpdateGraphFilter2);

			PatchAddr = (PBYTE)hModule + 0x2115C;
			g_FilterDrawRet3 = PatchAddr + 5;
			g_FilterDrawCall3 = GetCALLTarget(PatchAddr);
			ReplaceJump(PatchAddr, (PBYTE)&UpdateGraphFilter3);

			PatchAddr = (PBYTE)hModule + 0x2117E;
			g_FilterDrawRet4 = PatchAddr + 5;
			g_FilterDrawCall4 = GetCALLTarget(PatchAddr);
			ReplaceJump(PatchAddr, (PBYTE)&UpdateGraphFilter4);

			//ReplaceJump(PtrAdd((PBYTE)hModule, (PBYTE)0x2294D), (PBYTE)&UpdateGraphFilter1);
			//ReplaceJump(PtrAdd((PBYTE)hModule, (PBYTE)0x22928), (PBYTE)&UpdateGraphFilter2);
			//ReplaceJump(PtrAdd((PBYTE)hModule, (PBYTE)0x2297C), (PBYTE)&UpdateGraphFilter3);
			//ReplaceJump(PtrAdd((PBYTE)hModule, (PBYTE)0x2299E), (PBYTE)&UpdateGraphFilter4);
#endif
			SenaHook::GetSenaHook()->DllPatchFlag = TRUE;
		}
	}
	return hModule;
}


HFONT WINAPI HookCreateFontIndirectA(LOGFONTA *lplf)
{
	LOGFONTW Info;
	RtlZeroMemory(&Info, sizeof(Info));

	Info.lfHeight = lplf->lfHeight;
	Info.lfWidth = lplf->lfWidth;
	Info.lfEscapement = lplf->lfEscapement;
	Info.lfOrientation = lplf->lfOrientation;
	Info.lfWeight = lplf->lfWeight;
	Info.lfItalic = lplf->lfItalic;
	Info.lfUnderline = lplf->lfUnderline;
	Info.lfStrikeOut = lplf->lfStrikeOut;
	Info.lfCharSet = GB2312_CHARSET;
	Info.lfOutPrecision = lplf->lfOutPrecision;
	Info.lfClipPrecision = lplf->lfClipPrecision;
	Info.lfQuality = lplf->lfQuality;
	Info.lfPitchAndFamily = lplf->lfPitchAndFamily;

#if 0
	switch (Nt_CurrentPeb()->OSMajorVersion)
	{
	case 6:
		if (Nt_CurrentPeb()->OSMinorVersion >= 4)
			StrCopyW(Info.lfFaceName, L"黑体");
		else
			StrCopyW(Info.lfFaceName, L"等线");
		break;

	case 10:
		StrCopyW(Info.lfFaceName, L"等线");
		break;

	default:
		StrCopyW(Info.lfFaceName, L"SimHei");
		break;
	}
#else
	StrCopyW(Info.lfFaceName, L"黑体");
#endif

	//PrintConsoleW(L"indirect create\n");
	
	return CreateFontIndirectW(&Info);
}


UINT WINAPI HookGetOEMCP()
{
	return (UINT)936;
}


UINT WINAPI HookGetACP()
{
	return (UINT)936;
}

//push 4 --const value
LPSTR CDECL HookLoadText(int Args1, int Args2, int Args3, int Args4)
{
	LPSTR     Result;
	LPSTR     pStringStart;

	Result = SenaHook::GetSenaHook()->OldLoadText(Args1, Args2, Args3, Args4);

	if ((Args3 & 0x10000000) || Args4 == 0xFFFFFFF)
		return Result;

	ULONG Index = *(ULONG*)Result;

	if (SenaHook::GetSenaHook()->XmoeCompiler->QueryText(Index, NULL))
	{
		pStringStart = Result + sizeof(ULONG);

		//lstrcpyA(pStringStart, SenaHook::GetSenaHook()->TextPool[Index].c_str());
		SenaHook::GetSenaHook()->XmoeCompiler->QueryText(Index, pStringStart);
	}

	return Result;
}

BOOL CDECL HookCheckFontProc(PBYTE Code)
{
	return  (Code[0] >= 0x81 && Code[0] <= 0xFE)
		&& ((Code[1] >= 0x40 && Code[1] <= 0x7E)
		|| (Code[1] >= 0x80 && Code[1] <= 0xFE));
}

BOOL CDECL FakeDiskChecker(LPSTR String, ULONG args)
{
	return TRUE;
}


/*
CPU Disasm
Address                 Hex dump                       Command                                  Comments
0019409A                |.  50                         push eax                                 ; /Arg2
0019409B                |.  68 F8231E00                push offset 001E23F8                     ; |Arg1 = LeyLine.1E23F8
001940A0                |.  E8 2BEC0200                call 001C2CD0                            ; \LeyLine.001C2CD0

CPU Disasm
Address                 Hex dump                       Command                                  Comments
001940A5                |.  83C4 08                    add esp,8

CPU Disasm
Address                 Hex dump                       Command                                  Comments
001940A8                |.  8B8D C0F8FFFF              mov ecx,dword ptr [ebp-740]
001940AE                |.  0FB751 02                  movzx edx,word ptr [ecx+2]

*/


CHAR NewInfoName[MAX_PATH];
CHAR NewCompanyName[MAX_PATH];

PVOID PatchQueryNameStart = (PVOID)0x40A8;
PVOID PatchQueryNameEnd   = (PVOID)0x40AE;
PVOID PatchMemOffset      = (PVOID)0x523F8;
PVOID NewInfoNameOffset   = NULL;
PVOID NewCompanyNameOffset= NULL;

//CompanyName unk_4521F0
PVOID CompanyNameMemOffset = (PVOID)0x521F0;
ASM Void PatchQueryName()
{
	INLINE_ASM
	{
		pushad;
		push 260;
		push NewInfoNameOffset;
		push PatchMemOffset;
		call my_memcpy_inline;
		add  esp, 0xc;
		push 260;
		push NewCompanyNameOffset;
		push CompanyNameMemOffset;
		call my_memcpy_inline;
		add  esp, 0xc;
		popad;
		mov ecx, dword ptr[ebp - 0x740];
		jmp PatchQueryNameEnd;
	}
}


HWND WINAPI HookCreateWindowExA(
	_In_     DWORD     dwExStyle,
	_In_opt_ LPCSTR    lpClassName,
	_In_opt_ LPCSTR    lpWindowName,
	_In_     DWORD     dwStyle,
	_In_     int       x,
	_In_     int       y,
	_In_     int       nWidth,
	_In_     int       nHeight,
	_In_opt_ HWND      hWndParent,
	_In_opt_ HMENU     hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID    lpParam
	)
{
	if (lstrcmpA(lpClassName,  "Leyline1_chs") == 0 &&
		lstrcmpA(lpWindowName, "Leyline1_chs") == 0)
	{
		SenaHook::GetSenaHook()->hWnd =
			CreateWindowExW(
				dwExStyle,
				L"Leyline1_chs",
				L"时钟机关的Ley-line  -黄昏时的境界线-",
				dwStyle,
				x,
				y,
				nWidth,
				nHeight,
				hWndParent,
				hMenu,
				hInstance,
				lpParam
			);

		//RtlZeroMemory(PatchMemOffset, MAX_PATH);
		//StrCopyA((PCHAR)PatchMemOffset, "时钟机关的Ley-line  -黄昏时的境界线-");
		SenaHook::GetSenaHook()->IsWindowInited = SenaHook::GetSenaHook()->hWnd ? TRUE : FALSE;
		
		return SenaHook::GetSenaHook()->hWnd;
	}
	
	return
		CreateWindowExA(
		dwExStyle,
		lpClassName,
		lpWindowName,
		dwStyle,
		x,
		y,
		nWidth,
		nHeight,
		hWndParent,
		hMenu,
		hInstance,
		lpParam
		);
}

//LRESULT __stdcall sub_42FE80(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
LRESULT NTAPI HookWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_SETTEXT)
	{
		return SetWindowTextW(hWnd, L"时钟机关的Ley-line  -黄昏时的境界线-");
	}
	else
	{
		return SenaHook::GetSenaHook()->OldWindowProc(hWnd, Msg, wParam, lParam);
	}
}


DWORD NTAPI SetWindowTextWorker(LPVOID)
{
	while (1)
	{
		if (SenaHook::GetSenaHook()->IsWindowInited)
		{
			SetWindowTextW(SenaHook::GetSenaHook()->hWnd, L"时钟机关的Ley-line  -黄昏时的境界线-");
		}
		Ps::Sleep(100);
	}
	return NULL;
}


NTSTATUS NTAPI SenaHook::Init()
{
	NTSTATUS   Status;
	LPVOID     ModuleOfHost;
	BOOL       CompilerResult;
	ULONG      TextSize;
	PBYTE      TextBuffer;
	DWORD      ThreadId;
	HANDLE     HandleOfThread;

	RtlZeroMemory(NewInfoName, sizeof(NewInfoName));
	StrCopyA(NewInfoName, "Leyline1_chs");
	NewInfoNameOffset = &NewInfoName[0];

	RtlZeroMemory(NewCompanyName, sizeof(NewCompanyName));
	StrCopyA(NewCompanyName, "UNiSONSHIFT_Blossom");
	NewCompanyNameOffset = &NewCompanyName[0];

	ModuleOfHost = GetModuleHandleW(NULL);

	auto DYNAMIC_ADDRESS = [=](ULONG_PTR Address)->LPVOID
	{
		LONG_PTR     RelocateValue;
		PVOID        DynamicAddress;

		RelocateValue  = (SIZE_T)ModuleOfHost - 0x00400000;
		DynamicAddress = (PVOID)((SIZE_T)Address + RelocateValue);

		return DynamicAddress;
	};

	auto HOOK_FAILED = [=](ULONG Process)
	{
		switch (Process)
		{
		case IAT_HOOK:
			return L"第一部分启动失败";

		case EAT_HOOK:
			return L"第二部分启动失败";

		case INLINE_HOOK:
			return L"第三部分启动失败";

		case MEMORY_HOOK:
			return L"第四部分启动失败";

		default:
			return L"Failed to initialize hook stub(Unknown status)";
		}
	};

	PatchQueryNameStart = (LPVOID)((ULONG_PTR)PatchQueryNameStart + (ULONG_PTR)GetModuleHandleW(NULL));
	PatchQueryNameEnd   = (LPVOID)((ULONG_PTR)PatchQueryNameEnd   + (ULONG_PTR)GetModuleHandleW(NULL));
	PatchMemOffset      = (LPVOID)((ULONG_PTR)PatchMemOffset      + (ULONG_PTR)GetModuleHandleW(NULL));

	CompanyNameMemOffset = (LPVOID)((ULONG_PTR)CompanyNameMemOffset + (ULONG_PTR)GetModuleHandleW(NULL));

	INLINE_PATCH_DATA p[] =
	{
		{ GetACP, HookGetACP, NULL },
		{ GetOEMCP, HookGetOEMCP, NULL },
		{ CreateFontIndirectA, HookCreateFontIndirectA, NULL },
		{ LoadLibraryExA, HookLoadLibraryExA, (PVOID*)&OldLoadLibraryExA },
		{ GetGlyphOutlineA, HookGetGlyphOutlineA, NULL },
		{ DYNAMIC_ADDRESS(0x42FC40), HookLoadText, (PVOID*)&OldLoadText },
		{ DYNAMIC_ADDRESS(0x424B90), HookCheckFontProc, NULL },
		{ DYNAMIC_ADDRESS(0x431430), FakeDiskChecker, NULL },
		{ DYNAMIC_ADDRESS(0x42FE80), HookWindowProc, (PVOID*)&OldWindowProc },
		{ PatchQueryNameStart, PatchQueryName, NULL }
	};

	IAT_PATCH_DATA f[] =
	{
		{ GetModuleHandleW(NULL),  MessageBoxA,     HookMessageBoxA,     "USER32.DLL" },
		{ GetModuleHandleW(NULL),  CreateWindowExA, HookCreateWindowExA, "USER32.DLL" },
		{ GetModuleHandleW(NULL),  SetWindowTextA,  HookSetWindowTextA,  "USER32.DLL" }
	};


	Status = InlinePatchMemory(p, countof(p));

	if (!IsStatusSuccess(Status))
	{
		MessageBoxW(NULL,
			HOOK_FAILED(INLINE_HOOK),
			L"X'moe Sofpal Universal Patch Engine", MB_OK);

		Ps::ExitProcess(0);
	}

	Status = IATPatchMemory(f, countof(f));

	if (!IsStatusSuccess(Status))
	{
		MessageBoxW(NULL, HOOK_FAILED(IAT_HOOK), L"X'moe Sofpal Universal Patch Engine", MB_OK);
		Ps::ExitProcess(0);
	}

	LOOP_ONCE
	{

		Status = InitFileSystem();
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"无法初始化虚拟文件系统", L"X'moe CoreLib", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(0);
		}

		TextBuffer = NULL;
		TextSize   = NULL;

		FileManager::GetFileManager()->QueryFile("yizhanniv.bin", TextBuffer, TextSize);
		CompilerResult = XmoeCompiler->LoadTextFromBuffer(TextBuffer, TextSize);

		if (!CompilerResult)
			CompilerResult = XmoeCompiler->LoadText(NULL);
		
		
		if (!CompilerResult)
		{
			MessageBoxW(NULL, L"内部运行失败", L"X'moe CoreLib", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(0);
		}
	}

	//HandleOfThread = CreateThread(NULL, NULL, SetWindowTextWorker, NULL, NULL, &ThreadId);

	return Status;
}


NTSTATUS NTAPI SenaHook::UnInit()
{
	return STATUS_UNSUCCESSFUL;
}


#define TextMagic "_TEXT_LIST__"

NTSTATUS NTAPI SenaHook::AttachTextBuffer(PBYTE FileBuffer, ULONG Length)
{
	ULONG       Offset, Count;

	if (RtlCompareMemory(FileBuffer, TextMagic, 0xC) != 0xC)
	{
		MessageBoxW(NULL, L"汉化文件被损毁", L"无法启动游戏", MB_OK);
		Ps::ExitProcess(-1);
	}

	Offset = 0xC;
	Count = *(ULONG*)(FileBuffer + Offset);

	TextCount = Count;
	Offset += 4;

	for (ULONG Index = 0; Index < Count; Index++)
	{
		Offset += 4;
		TextPool.push_back((LPSTR)(FileBuffer + Offset));
		Offset += lstrlenA((LPSTR)(FileBuffer + Offset)) + 1;
	}
	return STATUS_SUCCESS;
}



NTSTATUS NTAPI SenaHook::LoadBuffer(LPCSTR lpFileName, PBYTE& Buffer, ULONG_PTR& Size)
{
	NTSTATUS Status;
	WCHAR    ProjectPath[MAX_PATH];

	Status = STATUS_NOT_FOUND;
	LOOP_ONCE
	{

	}
	return Status;
}

NTSTATUS NTAPI SenaHook::LoadBuffer(LPWSTR lpFileName, PBYTE& Buffer, ULONG_PTR& Size)
{
	NTSTATUS Status;
	WCHAR    ProjectPath[MAX_PATH];
	WinFile  File;

	Status = STATUS_NOT_FOUND;

	LOOP_ONCE
	{

	}
	return Status;
}

NTSTATUS NTAPI SenaHook::InitFileSystem()
{
	if (!FileSystemInit)
	{
		FileSystemInit = FileManager::GetFileManager()->Init();
	}
	
	return FileSystemInit ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL ;
}
