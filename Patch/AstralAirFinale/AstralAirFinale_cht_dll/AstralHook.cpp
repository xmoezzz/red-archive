#include "AstralHook.h"
#include "MyHook.h"
#include "blake2.h"
#include "MyCxdec.h"

AstralHook* AstralHook::Handle = NULL;


typedef struct _FILE_ENTRY
{
	WCHAR FileName[MAX_PATH];
	ULONG Hash;
	ULONG Size;
	ULONG Offset;
}FILE_ENTRY, *PFILE_ENTRY;

typedef struct _STRING_ENTRY
{
	PVOID StringBuffer;
}STRING_ENTRY;

AstralHook* AstralHook::GetHook(PVOID Module)
{
	if (Handle == NULL)
		Handle = new AstralHook(Handle);

	if (Handle == NULL)
		return Handle;

	return Handle;
}

AstralHook* AstralHook::GetHook()
{
	if (Handle == NULL)
		return NULL;
	else
		return Handle;
}


AstralHook::AstralHook(PVOID Module) :
	DllModule(NULL),
	FileSystemInited(FALSE),
	UseTraditionalChinese(FALSE),
	MainWindow(FALSE)
{
	DllModule = Module;
}


HWND WINAPI HookCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	ULONG       Length;
	LPWSTR      ClassName;
	AstralHook* GlobalData;

	if (StrCompareA(lpClassName, "HS_MAIN_WINDOW_CLASS00"))
		return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

	static WCHAR WindowName[] = L"[X'moe漢化組]星辰戀曲的白色永恆-Finale- V1.0（本補丁完全免費，未參與收費活動）";

	Length    = StrLengthA(lpClassName) + 1;
	ClassName = (LPWSTR)AllocStack(Length * sizeof(WCHAR));
	RtlZeroMemory(ClassName, Length * sizeof(WCHAR));
	MultiByteToWideChar(932, 0, lpClassName, Length, ClassName, Length * sizeof(WCHAR));

	GlobalData = AstralHook::GetHook();
	
	GlobalData->MainWindow = CreateWindowExW(dwExStyle, ClassName, WindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	return GlobalData->MainWindow;
}

//Translation filter??(mbs string <-> Unicode)
LRESULT CALLBACK HookDefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (hWnd == AstralHook::GetHook()->MainWindow && AstralHook::GetHook()->MainWindow != NULL)
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	else
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
}


LRESULT WINAPI HookDispatchMessageA(MSG *lpMsg)
{
	if (lpMsg->hwnd == AstralHook::GetHook()->MainWindow && AstralHook::GetHook()->MainWindow != NULL)
		return DispatchMessageW(lpMsg);
	else
		return DispatchMessageA(lpMsg);
}

API_POINTER(DefWindowProcW) AstralWindowProc;

//@sub_4434C0 
//|-@sub_443100

//GUI Thread
LRESULT CALLBACK HookAstralWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	AstralHook::GetHook()->MainWindow = hWnd;
	return AstralWindowProc(hWnd, Msg, wParam, lParam);
}

//Must Hook Window Proc
ATOM WINAPI HookRegisterClassExA(CONST WNDCLASSEXA* lpWndClass)
{
	WNDCLASSEXW  ClassInfo;
	LPWSTR       MenuName, ClassName;

	if (StrCompareA(lpWndClass->lpszClassName, "HS_MAIN_WINDOW_CLASS00"))
		return RegisterClassExA(lpWndClass);

	MenuName  = (LPWSTR)AllocStack(MAX_PATH * 2);
	ClassName = (LPWSTR)AllocStack(MAX_PATH * 2);

	RtlZeroMemory(MenuName,  MAX_PATH * 2);
	RtlZeroMemory(ClassName, MAX_PATH * 2);

	MultiByteToWideChar(932, 0, lpWndClass->lpszMenuName,  StrLengthA(lpWndClass->lpszMenuName),  MenuName,  MAX_PATH);
	MultiByteToWideChar(932, 0, lpWndClass->lpszClassName, StrLengthA(lpWndClass->lpszClassName), ClassName, MAX_PATH);

	ClassInfo.cbSize        = sizeof(WNDCLASSEXW);
	ClassInfo.style         = lpWndClass->style;
	ClassInfo.lpfnWndProc   = lpWndClass->lpfnWndProc;
	ClassInfo.cbClsExtra    = lpWndClass->cbClsExtra;
	ClassInfo.cbWndExtra    = lpWndClass->cbWndExtra;
	ClassInfo.hInstance     = lpWndClass->hInstance;
	ClassInfo.hIcon         = lpWndClass->hIcon;
	ClassInfo.hCursor       = lpWndClass->hCursor;
	ClassInfo.hbrBackground = lpWndClass->hbrBackground;
	ClassInfo.lpszMenuName  = MenuName;
	ClassInfo.lpszClassName = ClassName;
	ClassInfo.hIconSm       = lpWndClass->hIconSm;

	if (!AstralHook::GetHook()->MainWindow)
	{
		INLINE_PATCH_DATA p[] = 
		{
			{ lpWndClass->lpfnWndProc, HookAstralWindowProc, (PVOID*)&AstralWindowProc }
		};

		InlinePatchMemory(p, countof(p));
	}

	return RegisterClassExW(&ClassInfo);
}


BOOL WINAPI HookGetVersionExA(LPOSVERSIONINFOA lpVersionInfo)
{
	BOOL Result;

	Result = GetVersionExA(lpVersionInfo);
	lpVersionInfo->dwMajorVersion = 5;
	lpVersionInfo->dwMinorVersion = 1;
	return Result;
}

UINT WINAPI HookGetOEMCP()
{
	return (UINT)950;
}

UINT WINAPI HookGetACP()
{
	return (UINT)950;
}

BOOL WINAPI HookGetCPInfo(UINT CodePage, LPCPINFO lpCPInfo)
{
	return GetCPInfo(950, lpCPInfo);
}

INT WINAPI HooklStrcmpiA(LPCSTR lpString1, LPCSTR lpString2)
{
	Int Result;

	Result = CompareStringA(0x411, 1, lpString1, -1, lpString2, -1);
	return Result - 2;
}


INT WINAPI HookMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	LPWSTR       TextName,   CaptionName;
	ULONG        TextLength, CaptionLength;

	TextLength    = StrLengthA(lpText);
	CaptionLength = StrLengthA(lpCaption);
	TextName      = (LPWSTR)AllocStack((TextLength    + 1) * 2);
	CaptionName   = (LPWSTR)AllocStack((CaptionLength + 1) * 2);

	RtlZeroMemory(TextName,    (TextLength + 1)    * 2);
	RtlZeroMemory(CaptionName, (CaptionLength + 1) * 2);

	MultiByteToWideChar(932, 0, lpText,    TextLength,    TextName,    (TextLength + 1)    * 2);
	MultiByteToWideChar(932, 0, lpCaption, CaptionLength, CaptionName, (CaptionLength + 1) * 2);

	return MessageBoxW(hWnd, TextName, CaptionName, uType);
}

//ole32
//IGraphBuilder...(via CoCreateInstance)
//sub_422430
//.text:004225A1                 call    ds:MultiByteToWideChar

INT WINAPI HookMultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, INT cbMultiByte, LPWSTR lpWideCharStr, INT cchWideChar)
{
	ULONG_PTR ExeModule;
	ULONG_PTR ReturnAddress, StrLength;
	WCHAR     CurrentPath[MAX_PATH];

	ReturnAddress = (ULONG_PTR)_ReturnAddress();
	ExeModule     = (ULONG_PTR)Nt_GetExeModuleHandle();

	if ((ReturnAddress - ExeModule) == 0x225A7 && cchWideChar == 260)
	{
		RtlZeroMemory(lpWideCharStr, cchWideChar * 2);
		RtlZeroMemory(CurrentPath, CONST_STRSIZE(CurrentPath));
		Nt_GetCurrentDirectory(CONST_STRSIZE(CurrentPath), CurrentPath);
		
		FormatStringW(lpWideCharStr, L"%s\\movie\\Astral.wmv", CurrentPath);

		return StrLengthW(lpWideCharStr);
	}

	return MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}


HFONT WINAPI HookCreateFontA(int nHeight, int  nWidth, int  nEscapement, int nOrientation,
	int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut,
	DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision,
	DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
	WCHAR FontFace[128];
	
	RtlZeroMemory(FontFace, countof(FontFace) * sizeof(FontFace[0]));
	MultiByteToWideChar(CP_ACP, 0, lpszFace, lstrlenA(lpszFace), FontFace, 128);

	return  CreateFontW(nHeight, nWidth, nEscapement, nOrientation,
		fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut,
		0x86, fdwOutputPrecision, fdwClipPrecision,
		fdwQuality, fdwPitchAndFamily, FontFace);
}


int WINAPI HookEnumFontFamiliesExA(HDC hdc, LPLOGFONTA lpLogfont, FONTENUMPROCA lpProc, LPARAM lParam, DWORD dwFlags)
{
	lpLogfont->lfCharSet = GB2312_CHARSET;
	return EnumFontFamiliesExA(hdc, lpLogfont, lpProc, lParam, dwFlags);
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
	ULONG      len;
	CHAR       mbchs[2];
	HFONT      hFont, hOldFont;
	LOGFONTW*  lplf;
	WCHAR      OutChar;
	UINT       cp = 950;


	auto ToTraditionalChinese = [](WCHAR InChar, WCHAR& OutChar)->NTSTATUS
	{
		USHORT LangID = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
		LCID   Locale = MAKELCID(LangID, SORT_CHINESE_PRCP);

		OutChar = L'?';

		if (LCMapStringW(Locale, LCMAP_TRADITIONAL_CHINESE, &InChar, 1, &OutChar, 1) == 0)
			return STATUS_UNSUCCESSFUL;

		return STATUS_SUCCESS;
	};

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
		if (AstralHook::GetHook()->UseTraditionalChinese)
		{
			if (NT_FAILED(ToTraditionalChinese(LOWORD(uChar), OutChar)))
			{
				OutChar = uChar;
			}
		}
		else
		{
			OutChar = uChar;
		}
		Result = GetGlyphOutlineW(hdc, OutChar, uFormat,
			lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return Result;
}


/*
主循环
CPU Disasm
Address                 Hex dump                       Command                                          Comments
004454B0                |> >8B86 1C080000              /mov eax,dword ptr [esi+81C]
004454B6                |. |8B4E 04                    |mov ecx,dword ptr [esi+4]
004454B9                |. |0FB60C01                   |movzx ecx,byte ptr [eax+ecx]
004454BD                |. |0FB6D1                     |movzx edx,cl
004454C0                |. |40                         |inc eax
004454C1                |. |8986 1C080000              |mov dword ptr [esi+81C],eax
004454C7                |. |8B0495 E0154800            |mov eax,dword ptr [edx*4+4815E0]                ; ASCII "P¥A"
004454CE                |. |8BCE                       |mov ecx,esi
004454D0                |. |FFD0                       |call eax
004454D2                |. |80BE 10080000 00           |cmp byte ptr [esi+810],0
004454D9                |.^\74 D5                      \je short 004454B0
*/


/*
PushString

CPU Disasm
Address                 Hex dump                       Command                                          Comments
00445B50                /.  8B81 20080000              mov eax,dword ptr [ecx+820]      //StackTop Num
00445B56                |.  8D50 01                    lea edx,[eax+1]
00445B59                |.  8991 20080000              mov dword ptr [ecx+820],edx
00445B5F                |.  C644C1 08 04               mov byte ptr [eax*8+ecx+8],4     //当前栈顶，type(4)为String
00445B64                |.  8B91 1C080000              mov edx,dword ptr [ecx+81C]      //[ecx+81C], 当前ScriptPos
00445B6A                |.  56                         push esi
00445B6B                |.  8B71 04                    mov esi,dword ptr [ecx+4]
00445B6E                |.  0FB63416                   movzx esi,byte ptr [edx+esi]     //esi，当前String长度（含0）
00445B72                |.  42                         inc edx                          //跳过字符串长度记录段(BYTE)
00445B73                |.  8991 1C080000              mov dword ptr [ecx+81C],edx      
00445B79                |.  8954C1 0C                  mov dword ptr [eax*8+ecx+0C],edx //String对象，const，不需要释放
00445B7D                |.  01B1 1C080000              add dword ptr [ecx+81C],esi      //跳过字符串内容，准备放回Main Dispatcher执行下一条VM代码
00445B83                |.  5E                         pop esi
00445B84                \.  C3                         retn

*/

//graph Anzu...
ForceInline ULONG_PTR FASTCALL MakePrimaryHash(PWSTR DirName)
{
	if (StrNICompareW(DirName, L"graph", 5, StrCmp_ToLower) == 0)
		return TAG4('grap');
	else if (StrNICompareW(DirName, L"Anzu", 4, StrCmp_ToLower) == 0)
		return TAG4('Anzu');
	
	//Unique Directory Name
	return TAG4('Yuki');
}

//in graph folder
ForceInline ULONG_PTR FASTCALL MakeSecondaryHash(PWSTR DirName)
{
	auto MurMurHash = [](LPCVOID key, ULONG Length)->ULONG
	{
		const ULONG m    = 0x5bd1e995;
		const int   r    = 24;
		const int   seed = 97;
		ULONG       h    =   seed ^ Length;
		LPCBYTE     data = (LPCBYTE)key;

		while (Length >= 4)
		{
			unsigned int k = *(unsigned int *)data;
			k *= m;
			k ^= k >> r;
			k *= m;
			h *= m;
			h ^= k;
			data += 4;
			Length -= 4;
		}

		switch (Length)
		{
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0];
			h *= m;
		};
		h ^= h >> 13;
		h *= m;
		h ^= h >> 15;
		return h;
	};

	return MurMurHash(DirName, StrLengthW(DirName) * 2);
}

//from file system
NTSTATUS AstralHook::LookupFileInternal(PCSTR FileName, ULONG& Size, PBYTE& Buffer)
{
	NTSTATUS            Status;
	WCHAR               UnicodeName[MAX_PATH];
	ULONG               StrLength,   CurPos;
	PWSTR               PrimaryPart, SecondaryPart;
	PVOID               MainEntry,   SubEntry;
	LARGE_INTEGER       Offset;

	RtlZeroMemory(UnicodeName, countof(UnicodeName) * sizeof(UnicodeName[0]));
	MultiByteToWideChar(932, 0, FileName, StrLengthA(FileName), UnicodeName, countof(UnicodeName));

	Size          = 0;
	Buffer        = NULL;
	StrLength     = StrLengthW(UnicodeName);
	CurPos        = 0;
	PrimaryPart   = UnicodeName;
	SecondaryPart = NULL;

	while (CurPos < StrLength)
	{
		if (UnicodeName[CurPos] == L'\\' || UnicodeName[CurPos] == L'/')
		{
			SecondaryPart = &UnicodeName[CurPos + 1];
			break;
		}
		CurPos++;
	}

	MainEntry = FileSystemEntry.Lookup(MakePrimaryHash(PrimaryPart));
	if (!MainEntry)
		return STATUS_NO_SUCH_DOMAIN;

	LOOP_ONCE
	{
		if (!SecondaryPart)
		{
			Size = ((PFILE_ENTRY)MainEntry)->Size;
			Buffer = (PBYTE)AllocateMemoryP(Size);
			if (!Buffer)
			{
				Size = 0;
				Status = STATUS_NO_MEMORY;
				break;
			}
			Offset.QuadPart = 0;
			Offset.LowPart  = ((PFILE_ENTRY)MainEntry)->Offset;
			AsFile.Seek(Offset, FILE_BEGIN);
			AsFile.Read(Buffer, Size);
			Status = STATUS_SUCCESS;
		}
		else
		{
			SubEntry = ((MlHandleTable*)MainEntry)->Lookup(MakeSecondaryHash(SecondaryPart));
			if (!SubEntry)
			{
				Status = STATUS_NO_SUCH_DOMAIN;
				break;
			}

			Size = ((PFILE_ENTRY)SubEntry)->Size;
			Buffer = (PBYTE)AllocateMemoryP(Size);
			if (!Buffer)
			{
				Size = 0;
				Status = STATUS_NO_SUCH_FILE;
				break;
			}
			Offset.QuadPart = 0;
			Offset.LowPart = ((PFILE_ENTRY)SubEntry)->Offset;
			AsFile.Seek(Offset, FILE_BEGIN);
			AsFile.Read(Buffer, Size);
			Status = STATUS_SUCCESS;
		}
	}
	return Status;
}


NTSTATUS AstralHook::LookupFile(PCSTR FileName, ULONG& Size, PBYTE& Buffer)
{
	NTSTATUS            Status;
	NtFileDisk          File;
	WCHAR               UnicodeName[MAX_PATH];
	ml::StringT<USHORT> FullUnicodeName;
	
	
	if (FileSystemInited)
		return LookupFileInternal(FileName, Size, Buffer);
	
	RtlZeroMemory(UnicodeName, countof(UnicodeName) * sizeof(UnicodeName[0]));
	MultiByteToWideChar(932, 0, FileName, StrLengthA(FileName), UnicodeName, countof(UnicodeName));

	FullUnicodeName = L"ProjectDir\\";
	FullUnicodeName += UnicodeName;

	FullUnicodeName.Replace(L'/', L'\\');

	LOOP_ONCE
	{
		Size   = 0;
		Buffer = NULL;
		Status = File.Open(FullUnicodeName);
		if (NT_FAILED(Status))
			break;

		Size = File.GetSize32();
		Buffer = (PBYTE)AllocateMemoryP(Size);
		if (!Buffer)
		{
			Status = STATUS_NO_MEMORY;
			File.Close();
			break;
		}

		File.Read(Buffer, Size);
		File.Close();
	}
	return Status;
}

NTSTATUS AstralHook::InitFileSystem()
{
	NTSTATUS Status;

	Status = AsFile.Open(L"Astral.anz");
	if (NT_FAILED(Status))
		return STATUS_ABANDONED;

	FileSystemEntry.Create();

	FileSystemInited = TRUE;
	return STATUS_SUCCESS;
}


ForceInline Void blake2bp_buffer(LPCBYTE buffer, size_t length, PBYTE resstream)
{
	blake2bp_state S[1];

	blake2bp_init(S, BLAKE2B_OUTBYTES);
	blake2bp_update(S, buffer, length);
	blake2bp_final(S, resstream, BLAKE2B_OUTBYTES);
}

static CCxdec* g_cxdec = NULL;


ForceInline Void EncodeString(LPSTR Buffer, ULONG Size, ULONG Hash, ULONG Index)
{
	//VMStart();
	LARGE_INTEGER Offset;

	auto DecryptCxdecInternal = [](ULONG Hash, LARGE_INTEGER Offset, PVOID lpBuffer, ULONG BufferSize, ULONG Index)->BOOL
	{
		PBYTE           pbBuffer;
		ULONG           Mask, Mask2;
		LARGE_INTEGER   CurrentPos;
		CCxdec         *pCxdec;
		Byte            HashTable[64];

		pCxdec = g_cxdec;
		if (pCxdec == NULL)
		{
			pCxdec = g_cxdec = new CCxdec;
		}

		pbBuffer = (PBYTE)lpBuffer;
		Mask = pCxdec->GetMask(Hash);

		Mask2 = LOWORD(Mask);
		CurrentPos.QuadPart = Offset.QuadPart + BufferSize;

		if (Mask2 >= Offset.QuadPart && Mask2 < CurrentPos.QuadPart)
		{
			*(pbBuffer + Mask2 - Offset.LowPart) ^= Hash >> 16;
		}

		Mask2 = HIWORD(Mask);
		if (Mask2 >= Offset.QuadPart && Mask2 < CurrentPos.QuadPart)
		{
			*(pbBuffer + Mask2 - Offset.LowPart) ^= Hash >> 8;
		}

		XorMemory(pbBuffer, Hash, BufferSize);

		union
		{
			ULONG InfoHash[2];
			BYTE  ByteInfo[8];
		}PreHash;

		PreHash.InfoHash[0] = Hash;
		PreHash.InfoHash[1] = Index;

		blake2bp_buffer(PreHash.ByteInfo, 8, HashTable);

		PreHash.InfoHash[0] = PreHash.InfoHash[1] = 0;

		for (ULONG i = 0; i < BufferSize; i++)
			((PBYTE)lpBuffer)[i] ^= HashTable[i % countof(HashTable)];

		RtlZeroMemory(HashTable, sizeof(HashTable));
		return TRUE;
	};

	Offset.QuadPart = 0;
	DecryptCxdecInternal(Hash, Offset, Buffer, Size, Index);
	//VMEnd();
}


ForceInline ULONG MyHashMask(ULONG Hash)
{
	union
	{
		BYTE  ByteInfo[4];
		ULONG Info;
	};


	static BYTE TempTable[512] =
	{
		0x7d, 0x54, 0x11, 0xd1, 0xfa, 0xad, 0xb0,
		0xc2, 0x77, 0x0f, 0xe2, 0x5a, 0x95, 0x19, 0x4e,
		0x93, 0x27, 0x43, 0x15, 0x7d, 0xb5, 0x54, 0x48,
		0xba, 0xfb, 0xe9, 0x6d, 0x02, 0x6b, 0xf4, 0xfe,
		0xa0, 0xa7, 0x3b, 0xe9, 0xf3, 0x08, 0xd5, 0x11,
		0xee, 0x1a, 0xb0, 0xcb, 0x98, 0x1d, 0x0e, 0x62,
		0x8d, 0x86, 0x03, 0x94, 0x7b, 0x7b, 0xf9, 0x13,
		0xa5, 0x5b, 0x2c, 0x05, 0x64, 0x34, 0x2f, 0x84,
		0xa1, 0x4b, 0x65, 0x1e, 0x5c, 0x97, 0x88, 0x56,
		0x29, 0x46, 0x25, 0x21, 0xad, 0x37, 0x1e, 0x6b,
		0x25, 0x7e, 0x27, 0x90, 0xe0, 0xe3, 0x4a, 0xe3,
		0xc0, 0x63, 0x63, 0x2a, 0xbd, 0xae, 0xa4, 0x1f,
		0x61, 0xa7, 0x12, 0xf1, 0x4d, 0xe8, 0x06, 0xc1,
		0xb3, 0x3b, 0xad, 0x25, 0xda, 0x21, 0x89, 0xa9,
		0x9d, 0x4f, 0xee, 0x49, 0xec, 0x2c, 0x86, 0xf8,
		0x4a, 0x55, 0xcd, 0x1c, 0x4d, 0x19, 0x95, 0x10,
		0x21, 0xfd, 0x83, 0xa0, 0x05, 0x39, 0x90, 0x91,
		0xcc, 0x39, 0x89, 0x16, 0x5e, 0x1e, 0x8f, 0x5c,
		0x34, 0x3a, 0x98, 0xff, 0xe0, 0x97, 0xed, 0x93,
		0x83, 0x70, 0xaa, 0x1c, 0x54, 0xb6, 0x41, 0x95,
		0x20, 0x8d, 0xf7, 0x6d, 0xc4, 0xcc, 0x65, 0x06,
		0xb5, 0x81, 0xf8, 0x35, 0x79, 0x6b, 0x71, 0xc4,
		0x2b, 0x7e, 0x66, 0xf3, 0xfb, 0x62, 0xbf, 0xf2,
		0xab, 0xf4, 0x3a, 0x69, 0x13, 0xc4, 0xe8, 0xf1,
		0x9e, 0x95, 0xae, 0x98, 0xcb, 0xe1, 0xc5, 0x60,
		0xad, 0x52, 0x3a, 0xc0, 0x6b, 0x49, 0x6e, 0x22,
		0xc1, 0x5b, 0x97, 0x64, 0x7d, 0xcf, 0x3d, 0x57,
		0x02, 0x22, 0xbe, 0x43, 0xc9, 0x83, 0xcb, 0x61,
		0xdb, 0x57, 0xe8, 0x5f, 0x58, 0xb6, 0xf0, 0xe0,
		0xf4, 0xec, 0x8f, 0xf9, 0x74, 0xf9, 0xc6, 0xb5,
		0x36, 0x12, 0x6b, 0x92, 0xa6, 0x1d, 0xa6, 0x02,
		0xc9, 0x39, 0x75, 0xeb, 0xb6, 0x33, 0x28, 0x27,
		0x18, 0x12, 0xe6, 0x04, 0xad, 0x8d, 0x26, 0xc5,
		0xca, 0x8f, 0x37, 0x1f, 0xd5, 0xba, 0xb9, 0xbd,
		0xca, 0xe1, 0x22, 0xbd, 0xb7, 0x8d, 0x3a, 0x31,
		0x3f, 0x79, 0x9f, 0x9f, 0x1a, 0x15, 0x41, 0x81,
		0x94, 0x07, 0xe7, 0xc6, 0x0a, 0xa5, 0xa8, 0x4f,
		0x70, 0x7c, 0x73, 0x73, 0xcd, 0xcc, 0x88, 0x7b,
		0xbd, 0x0a, 0xfc, 0x26, 0xee, 0x5d, 0x39, 0x26,
		0xa4, 0x22, 0x7c, 0xa1, 0x36, 0x68, 0x55, 0xb2,
		0x8f, 0x74, 0x2b, 0xe5, 0xad, 0x3e, 0xb5, 0xbe,
		0x25, 0xf2, 0x82, 0x33, 0x9d, 0x70, 0x72, 0x2e,
		0x50, 0xcc, 0x3a, 0x0c, 0x8e, 0xcf, 0xe4, 0x20,
		0x39, 0x74, 0x4d, 0x30, 0x49, 0x6c, 0xa5, 0xf7,
		0x49, 0x9b, 0xf2, 0xa2, 0xd8, 0x98, 0x8e, 0x53,
		0x29, 0x31, 0xa4, 0xa1, 0x83, 0xe5, 0xb7, 0x16,
		0xc2, 0x68, 0x1b, 0xaf, 0xd4, 0x22, 0x7a, 0x5f,
		0x3d, 0xb0, 0x50, 0x8d, 0x93, 0x62, 0x70, 0x92,
		0x02, 0xbb, 0x7d, 0x3c, 0xca, 0xf4, 0x71, 0x4d,
		0xbc, 0x79, 0x1a, 0xfc, 0xc1, 0x6b, 0x97, 0x73,
		0x53, 0x1c, 0xdf, 0x4f, 0x01, 0x97, 0x3b, 0x24,
		0xf0, 0x15, 0xc7, 0xf7, 0x55, 0x88, 0xf6, 0xc1,
		0xfb, 0x14, 0x0b, 0xf3, 0xc3, 0x91, 0xa0, 0xec,
		0x1f, 0x0b, 0x22, 0x84, 0x96, 0x42, 0x53, 0x85,
		0x43, 0x2a, 0xc7, 0x2d, 0x56, 0x6c, 0x68, 0xae,
		0x92, 0xe3, 0xf2, 0xae, 0xcd, 0x20, 0x77, 0xc7,
		0x73, 0xe7, 0xdc, 0x07, 0x03, 0xaf, 0x5a, 0x71,
		0x91, 0x26, 0xfe, 0x7a, 0x42, 0xab, 0x2a, 0x8d,
		0xd3, 0xd2, 0x12, 0x88, 0x12, 0xe3, 0x3f, 0x3d,
		0x63, 0x5b, 0x0f, 0xf2, 0x3d, 0x69, 0x33, 0xe1,
		0xab, 0x73, 0x30, 0xb8, 0xcb, 0x8f, 0xdf, 0x1a,
		0x52, 0x0a, 0xed, 0x1d, 0x06, 0xe4, 0x5c, 0xca,
		0x42, 0x52, 0x00, 0xa0, 0x76, 0x3b, 0x02, 0x11,
		0xa4, 0xbc, 0x60, 0x03, 0xe4, 0xa4, 0x6b, 0x51,
		0xe1
	};

	ByteInfo[0] = TempTable[Hash & 0xFF];
	ByteInfo[1] = TempTable[((Hash >> 3) * 4) % countof(TempTable)];
	ByteInfo[2] = TempTable[((Hash * 6) ^ 0x1542) % countof(TempTable)];
	ByteInfo[3] = TempTable[Hash % countof(TempTable)];

	return Info;
}


#include "ThemidaSDK.h"

//from koikake
NTSTATUS AstralHook::LoadBinaryScriptInternal(PBYTE Buffer, ULONG Size)
{
	VMStart();

	ULONG                  CurPos, Length, Hash, Index;
	PSTR                   DecryptedBuffer;
	PML_HANDLE_TABLE_ENTRY HtEntry;

	CurPos = 4;
	while (CurPos < Size)
	{

		Length = *(PULONG)(Buffer + CurPos);
		CurPos += 4;
		Hash   = *(PULONG)(Buffer + CurPos);
		CurPos += 4;
		Index  = *(PULONG)(Buffer + CurPos);
		CurPos += 4;

		DecryptedBuffer = (PSTR)AllocateMemoryP(ROUND_UP((Length ^ MyHashMask(Hash)) + 1, 4), HEAP_ZERO_MEMORY);
		if (!DecryptedBuffer)
		{
			FreeMemoryP(Buffer);
			return STATUS_NO_MEMORY;
		}

		RtlCopyMemory(DecryptedBuffer, &Buffer[CurPos], (Length ^ MyHashMask(Hash)));
		EncodeString(DecryptedBuffer, Size, Hash, Index);

		HtEntry = StringHashMap.Insert(Index);

		if (HtEntry)
			HtEntry->Handle = DecryptedBuffer;
	}
	FreeMemoryP(Buffer);

	VMEnd();

	return STATUS_SUCCESS;
}

NTSTATUS AstralHook::CompileTextScriptInternal(PCWSTR FileName)
{
	NTSTATUS               Status;
	NtFileDisk             File;
	PBYTE                  Buffer;
	ULONG                  Size, Index;
	BOOL                   ResultOfParse;
	CHAR                   OutString[2000];
	PCHAR                  StringBuffer;
	ml::StringT<ULONG>     ReadLine;
	PML_HANDLE_TABLE_ENTRY HtEntry;


	auto ProcessLineXmoe = [](ULONG& LineNum, PSTR OutString, ULONG BufferSize, ml::StringT<ULONG>& Input)->BOOL
	{
		BOOL   Result;
		WCHAR  NumStr[12];

		LOOP_ONCE
		{
			Result = FALSE;

			if (StrLengthW(Input) <= 13)
				break;

			if (Input[12] != ']')
				break;

			RtlZeroMemory(NumStr, CONST_STRSIZE(NumStr));
			RtlCopyMemory(NumStr, &Input[4], 16);

			LineNum = StringToInt32HexW(NumStr);
			Input   = Input.SubString(13);

			WideCharToMultiByte(950, 0, Input, StrLengthW(Input), OutString, BufferSize, 0, 0);
			Result = TRUE;
		}
		return Result;
	};

	LOOP_ONCE
	{
		Status = File.Open(FileName);
		if (NT_FAILED(Status))
			break;
		
		Size   = File.GetSize32();
		File.Seek(2, FILE_BEGIN); //ship bom
		Buffer = (PBYTE)AllocateMemoryP(ROUND_UP(Size + 1, 4), HEAP_ZERO_MEMORY);
		if (!Buffer)
		{
			Status = STATUS_NO_MEMORY;
			break;
		}

		File.Read(Buffer, Size);
		File.Close();

		ReadLine = (PCWSTR)Buffer;
		auto Lines = ReadLine.SplitLines();

		for (auto& Line : Lines)
		{
			if (StrLengthW(Line) == 0)
				continue;

			if (Line[0] == L';')
			{
				RtlZeroMemory(OutString, CONST_STRSIZE(OutString));
				ResultOfParse = ProcessLineXmoe(Index, OutString, CONST_STRSIZE(OutString), Line);

				if (ResultOfParse)
				{
					StringBuffer = (PCHAR)AllocateMemoryP(ROUND_UP(StrLengthA(OutString) + 1, 4), HEAP_ZERO_MEMORY);
					StrCopyA(StringBuffer, OutString);

					HtEntry = StringHashMap.Insert(Index);

					if (HtEntry) 
						HtEntry->Handle = StringBuffer;
				}
			}
		}

		FreeMemoryP(Buffer);
	}
	return Status;
}

NTSTATUS AstralHook::CompileScript()
{
	NTSTATUS              Status;
	PBYTE                 Buffer;
	ULONG                 Size, Attribute;
	NtFileDisk            File;
	HANDLE                hSearch;
	WIN32_FIND_DATAW      FindInfo;
	ml::StringT<USHORT>   FileName;

	static WCHAR ScriptFileName[] = L"Astral.anz";

	StringHashMap.Create();
	RtlZeroMemory(&FindInfo, sizeof(FindInfo));

	Status = STATUS_SUCCESS;

	Attribute = Nt_GetFileAttributes(ScriptFileName);
	if (Attribute == INVALID_FILE_ATTRIBUTES)
	{
		hSearch = Nt_FindFirstFile(L"ProjectDir\\*.txt", &FindInfo);

		if (hSearch == INVALID_HANDLE_VALUE)
			return Status;

		do
		{
			FileName = L"ProjectDir\\";
			FileName += FindInfo.cFileName;
			CompileTextScriptInternal(FileName);

		} while (Nt_FindNextFile(hSearch, &FindInfo));

		Nt_FindClose(hSearch);
		return Status;
	}
	else
	{
		LOOP_ONCE
		{
			Status = File.Open(ScriptFileName);
			if (NT_FAILED(Status))
				break;

			Size   = File.GetSize32();
			Buffer = (PBYTE)AllocateMemoryP(Size);
			if (!Buffer)
			{
				Status = STATUS_NO_MEMORY;
				break;
			}

			File.Read(Buffer, Size);
			File.Close();

			return LoadBinaryScriptInternal(Buffer, Size);
		}
		return Status;
	}
}

PCSTR AstralHook::LookupString(ULONG Index)
{
	PML_HANDLE_TABLE_ENTRY Entry;

	Entry = StringHashMap.Lookup(Index);
	if (Entry)
		return (PCSTR)Entry->Handle;

	return NULL;
}

PCSTR NTAPI QuoteString(ULONG Index)
{
	return AstralHook::GetHook()->LookupString(Index - 2);
}

ForceInline PCSTR FASTCALL QuoteStringCorrectOffset(ULONG Index)
{
	return AstralHook::GetHook()->LookupString(Index);
}

ASM Void HookPushString()
{
	INLINE_ASM
	{
		mov   eax,  dword ptr[ecx + 0x820];
		lea   edx,  [eax + 1];
		mov   dword ptr[ecx + 0x820], edx;
		mov   byte  ptr[eax * 8 + ecx + 8], 4;
		mov   edx,  dword ptr[ecx + 0x81C];
		push  esi;
		mov   esi,  dword ptr[ecx + 4];
		movzx esi,  byte ptr[edx + esi];
		inc   edx;

		push  edi;
		push  eax;
		push  ecx;
		push  edx;
		push  ebx;
		push  esi;

		push  edx;
		call  QuoteString;
		
		
		test  eax, eax;
		jz    QUOTE_ORIGINAL_STRING;

		mov   edi, eax;

		pop   esi;
		pop   ebx;
		pop   edx;
		pop   ecx;
		pop   eax;
		
		mov   dword ptr[eax * 8 + ecx + 0xC], edi;

		pop   edi;

		mov   dword ptr[ecx + 0x81C], edx;
		add   dword ptr[ecx + 0x81C], esi;
		jmp   READY_FOR_RETURN;

	QUOTE_ORIGINAL_STRING:
		
		pop   esi;
		pop   ebx;
		pop   edx;
		pop   ecx;
		pop   eax;
		pop   edi;

		mov   dword ptr[ecx + 0x81C], edx;
		mov   dword ptr[eax * 8 + ecx + 0xC], edx;
		add   dword ptr[ecx + 0x81C], esi;
		jmp   READY_FOR_RETURN;


	READY_FOR_RETURN:
		pop   esi;
		retn;
	}
}


/*
HookCopyPrefix
00444B0A                |.  52                         push edx                                         ; /Arg1
00444B0B                |.  03C8                       add ecx,eax                                      ; |
00444B0D                |.  E8 4EF1FEFF                call 00433C60
*/


/*
HookCatString
00444B1B                |.  8D04AD EC496A00            lea eax,[ebp*4+6A49EC]                           ; |UNICODE ":00090c00;0409:00000409"
00444B22                |.  03C8                       add ecx,eax                                      ; |
00444B24                |.  E8 97F1FEFF                call 00433CC0
*/

/*
HookCatQuote
00444BBC                |.  8D04AD EC496A00            lea eax,[ebp*4+6A49EC]                           ; UNICODE ":00090c00;0409:00000409"
00444BC3                |.  52                         push edx                                         ; /Arg1
00444BC4                |.  03C8                       add ecx,eax                                      ; |
00444BC6                |.  E8 F5F0FEFF                call 00433CC0
*/

/*
HookCopyDescription
0043CB4E                |>  56                         push esi                                         ; /Arg1
0043CB4F                |.  8BCD                       mov ecx,ebp                                      ; |
0043CB51                |.  E8 BA6AFFFF                call 00433610
*/


struct IrotoriScriptObject
{
	PSTR *Prefix;

	VOID THISCALL HookCopyPrefix(PSTR String);
	VOID THISCALL HookCatString(PSTR String);
	VOID THISCALL HookCatQuote(PSTR String);
	VOID THISCALL HookCopyDescription(PSTR String);
};

typedef struct
{
	UCHAR   Type;
	UCHAR   PadFor2Byte;
	USHORT  StackBase;
	ULONG   Value;

} IROTORI_STACK_ENTRY, *PIROTORI_STACK_ENTRY;

typedef struct
{
	DUMMY_STRUCT(4);                        // 0x000

	PVOID ScriptBuffer;                     // 0x004
	IROTORI_STACK_ENTRY StackEntry[0x100];  // 0x008

	DUMMY_STRUCT(0x14);                     // 0x808

	ULONG_PTR NextOpOfffset;                // 0x81C
	ULONG_PTR StackTop;                     // 0x820

} IROTORI_SCRIPT_OBJECT, *PIROTORI_SCRIPT_OBJECT;


TYPE_OF(&IrotoriScriptObject::HookCatString) StubCatString;
TYPE_OF(&IrotoriScriptObject::HookCatString) StubCatQuote;
TYPE_OF(&IrotoriScriptObject::HookCatString) StubCopyPrefix;
TYPE_OF(&IrotoriScriptObject::HookCatString) StubCopyDescription;

PVOID StubCatStringEnd;
PVOID StubCatQuoteEnd;
PVOID StubCopyPrefixEnd;
PVOID StubCopyDescriptionEnd;

PCSTR GetTextBuffer(PIROTORI_SCRIPT_OBJECT Object, PCSTR String)
{
	ULONG_PTR               Offset;
	PCSTR                   Buffer;

	if (String[-2] != 0xE)
		return String;

	if (String[(BYTE)String[-1] - 1] != 0)
		return String;

	Offset = PtrOffset(String, Object->ScriptBuffer) - 2;
	Buffer = QuoteStringCorrectOffset(Offset);
	if (!Buffer)
		return String;

	String = Buffer;

	return Buffer;
}

//In 'PrintText'
NAKED VOID THISCALL IrotoriScriptObject::HookCopyDescription(PSTR String)
{
    INLINE_ASM
    {
        push    edi;
        mov     edi, ecx;

		lea     ecx, dword ptr[ebx + 0x68DFA8];
		; mov     edx, [esp + 8];
        call    GetTextBuffer;
        mov     ecx, edi;
		mov     edx, eax;
        pop     edi;
		mov     [esp + 4], eax;
		call    StubCopyDescription;
		jmp     StubCopyDescriptionEnd;
    }
}


NAKED VOID THISCALL IrotoriScriptObject::HookCopyPrefix(PSTR String)
{
    INLINE_ASM
    {
        push    edi;
        mov     edi, ecx;

        mov     ecx, esi;
		; mov     edx, [esp + 8];
        call    GetTextBuffer;
        mov     ecx, edi;
		mov     edx, eax;
        pop     edi;
		mov     [esp + 4], eax;
		call    StubCopyPrefix;
		jmp     StubCopyPrefixEnd;
    }
}


NAKED VOID IrotoriScriptObject::HookCatString(PSTR String)
{
    INLINE_ASM
    {
        push    edi;
        mov     edi, ecx;

        mov     ecx, esi;
		; mov     edx, [esp + 8];
        call    GetTextBuffer;
        mov     ecx, edi;
		mov     edx, eax;
        pop     edi;
		mov     [esp + 4], eax;
		call    StubCatString;
		jmp     StubCatStringEnd;
    }
}

NAKED VOID IrotoriScriptObject::HookCatQuote(PSTR String)
{
    INLINE_ASM
    {
        push    edi;
        mov     edi, ecx;

        mov     ecx, esi;
		; mov     edx, [esp + 8];
        call    GetTextBuffer;
        mov     ecx, edi;
		mov     edx, eax;
        pop     edi;
		mov     [esp + 4], eax;
		call    StubCatQuote;
		jmp     StubCatQuoteEnd;
    }
}


Void MyProcessString(PIROTORI_SCRIPT_OBJECT obj, char** str)
{
	BYTE* ps = (BYTE*)*str;
	if (ps[-2] != 0xe || ps[ps[-1] - 1] != 0)
		return;

	ULONG Offset = PtrOffset(*str, obj->ScriptBuffer) - 2;
	char* ns = (char*)QuoteStringCorrectOffset(Offset);

	if (ns)
		*str = ns;
}

void MyProcessString2(BYTE* s, char** str)
{
	PIROTORI_SCRIPT_OBJECT obj = (PIROTORI_SCRIPT_OBJECT)(s + 0x68DFA8);
	MyProcessString(obj, str);
}


typedef struct _FVP_BUFFER
{
	PBYTE   Buffer;
	ULONG   Size;
}FVP_BUFFER, *PFVP_BUFFER;


class FVPBufferManager
{
public:
	PFVP_BUFFER FVPBuffer;

	BOOL THISCALL FVPReadFile(LPCSTR lpFileName);
};


API_POINTER(FVPBufferManager::FVPReadFile) StubFVPReadFile;

//ReadFile
//sub_4344C0
BOOL THISCALL FVPBufferManager::FVPReadFile(LPCSTR lpFileName)
{
	return FALSE;
}

Void ReplaceJump(PBYTE Address, PBYTE DetouredCall)
{
	ULONG   OldProtect;
	ULONG   Size;

	Size = 5;
	VirtualProtect(Address, Size, PAGE_EXECUTE_READWRITE, &OldProtect);
	Address[0]           = JUMP;
	*(PDWORD)&Address[1] = (DWORD)(DetouredCall - Address - 5);
	VirtualProtect(Address, Size, OldProtect, &OldProtect);
}

#include "ilhook.h"
#pragma comment(lib, "BeaEngine_s_l.lib")

NTSTATUS AstralHook::Init()
{
	NTSTATUS Status;
	PVOID    ExeModule;

	ExeModule = Nt_GetExeModuleHandle();

	auto PatchFailed = [&](BOOL IsPatchViaJmp)->Void
	{
		static WCHAR IatFailed[]    = L"無法啟動第一部分";
		static WCHAR InlineFailed[] = L"無法啟動第二部分";

		MessageBoxW(NULL, IsPatchViaJmp ? InlineFailed : IatFailed, L"補丁啟動失敗", MB_OK | MB_ICONERROR);
	};
	
	LOOP_ONCE
	{
		IAT_PATCH_DATA f[] =
		{
			{ ExeModule, CreateWindowExA,     HookCreateWindowExA,     "USER32.DLL"   },
			{ ExeModule, MessageBoxA,         HookMessageBoxA,         "USER32.DLL"   },
			{ ExeModule, DefWindowProcA,      HookDefWindowProcA,      "USER32.DLL"   },
			{ ExeModule, RegisterClassExA,    HookRegisterClassExA,    "USER32.DLL"   },
			{ ExeModule, lstrcmpiA,           HooklStrcmpiA,           "KERNEL32.DLL" },
			{ ExeModule, GetOEMCP,            HookGetOEMCP,            "KERNEL32.DLL" },
			{ ExeModule, GetACP,              HookGetACP,              "KERNEL32.DLL" },
			{ ExeModule, GetVersionExA,       HookGetVersionExA,       "KERNEL32.DLL" },
			{ ExeModule, MultiByteToWideChar, HookMultiByteToWideChar, "KERNEL32.DLL" },
			{ ExeModule, CreateFontA,         HookCreateFontA,         "GDI32.DLL"    },
			{ ExeModule, EnumFontFamiliesExA, HookEnumFontFamiliesExA, "GDI32.DLL"    },
			{ ExeModule, GetGlyphOutlineA,    HookGetGlyphOutlineA,    "GDI32.DLL"    }
		};

		Status = IATPatchMemory(f, countof(f));
		if (NT_FAILED(Status))
		{
			PatchFailed(FALSE);
			break;
		}

		typedef struct _CALL_PATCH_DATA
		{
			PVOID  CallDestination;
			PVOID  DetouredCall;
			PVOID* OriginalCall;
		}CALL_PATCH_DATA, *PCALL_PATCH_DATA;

		auto PatchViaReplaceCall = [&](PCALL_PATCH_DATA Data, ULONG_PTR Count)
		{
			for (ULONG i = 0; i < Count; i++)
			{
				*Data[i].OriginalCall = GetCallDestination(Data[i].CallDestination);
				ReplaceJump((PBYTE)Data[i].CallDestination, (PBYTE)Data[i].DetouredCall);
			}
		};

		StubCatStringEnd       = (PVOID)((ULONG_PTR)ExeModule + 0x44B0D + 5);
		StubCatQuoteEnd        = (PVOID)((ULONG_PTR)ExeModule + 0x44B24 + 5);
		StubCopyPrefixEnd      = (PVOID)((ULONG_PTR)ExeModule + 0x44BC6 + 5);
		StubCopyDescriptionEnd = (PVOID)((ULONG_PTR)ExeModule + 0x3CB51 + 5);

		CALL_PATCH_DATA Call[] = 
		{
			{ (PVOID)((ULONG_PTR)ExeModule + 0x44B0D), PtrAdd((PVOID)nullptr, &IrotoriScriptObject::HookCopyPrefix),      (PVOID*)&StubCopyPrefix },
			{ (PVOID)((ULONG_PTR)ExeModule + 0x44B24), PtrAdd((PVOID)nullptr, &IrotoriScriptObject::HookCatString),       (PVOID*)&StubCatString },
			{ (PVOID)((ULONG_PTR)ExeModule + 0x44BC6), PtrAdd((PVOID)nullptr, &IrotoriScriptObject::HookCatQuote),        (PVOID*)&StubCatQuote },
			{ (PVOID)((ULONG_PTR)ExeModule + 0x3CB51), PtrAdd((PVOID)nullptr, &IrotoriScriptObject::HookCopyDescription), (PVOID*)&StubCopyDescription }
		};

		//PatchViaReplaceCall(Call, countof(Call));

		INLINE_PATCH_DATA p[] = 
		{
			{ (PVOID)0x445B50, HookPushString, NULL }
		};

		//Status = InlinePatchMemory(p, countof(p));


		HookSrcObject  srcObj;
		HookStubObject stubObj;

		Status = STATUS_UNSUCCESSFUL;
		PBYTE stubMem = (PBYTE)VirtualAlloc(0, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!stubMem)
		{
			MessageBoxW(NULL, L"初始化失敗", L"啟動失敗", MB_OK | MB_ICONERROR);
			break;
		}

		if (!InitializeHookSrcObject(&srcObj, (PVOID)0x444B0D) ||
			!InitializeStubObject(&stubObj, stubMem, 0x1000) ||
			!Hook32(&srcObj, 0, &stubObj, MyProcessString, "y\x01"))
		{
			MessageBoxW(NULL, L"初始化失敗", L"啟動失敗", MB_OK | MB_ICONERROR);
			break;
		}
		if (!InitializeHookSrcObject(&srcObj, (PVOID)0x444B24) ||
			!InitializeStubObject(&stubObj, stubMem + 100, 0x1000 - 100) ||
			!Hook32(&srcObj, 0, &stubObj, MyProcessString, "y\x01"))
		{
			MessageBoxW(NULL, L"初始化失敗", L"啟動失敗", MB_OK | MB_ICONERROR);
			break;
		}
		if (!InitializeHookSrcObject(&srcObj, (PVOID)0x444BC6) ||
			!InitializeStubObject(&stubObj, stubMem + 200, 0x1000 - 200) ||
			!Hook32(&srcObj, 0, &stubObj, MyProcessString, "y\x01"))
		{
			MessageBoxW(NULL, L"初始化失敗", L"啟動失敗", MB_OK | MB_ICONERROR);
			break;
		}
		if (!InitializeHookSrcObject(&srcObj, (PVOID)0x43CB51) ||
			!InitializeStubObject(&stubObj, stubMem + 300, 0x1000 - 300) ||
			!Hook32(&srcObj, 0, &stubObj, MyProcessString2, "b\x01"))
		{
			MessageBoxW(NULL, L"初始化失敗", L"啟動失敗", MB_OK | MB_ICONERROR);
			break;
		}

		Status = STATUS_SUCCESS;
		//InitFileSystem();
		Status = CompileScript();
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"初始化失敗", L"啟動失敗", MB_OK | MB_ICONERROR);
			break;
		}

		/*
		00443B68                |>  85C0                       test eax,eax
		*/

		static BYTE SelectFontPath[] = { 0x33 };

		CODE_PATCH_DATA c[] = 
		{
			{ (PVOID)0x443B68, SelectFontPath, sizeof(SelectFontPath) }
		};

		Status = CodePatchMemory(c, countof(c));
		if (NT_FAILED(Status))
		{
			MessageBoxW(NULL, L"初始化失敗", L"啟動失敗", MB_OK | MB_ICONERROR);
			break;
		}
	}
	return Status;
}
