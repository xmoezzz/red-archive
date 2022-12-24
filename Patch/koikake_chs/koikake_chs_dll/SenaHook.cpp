#include "SenaHook.h"
#include "MyHook.h"
#include "my.h"
#include <new>
#include <dshow.h>
#include <atlbase.h>
#include "FileManager.h"
#include <stdint.h>


SenaHook* SenaHook::m_Inst = nullptr;

//ML_OVERLOAD_NEW;

EXTERN_C void* CDECL memset32_sse2(void * p, int c, size_t n);

void* SSE2ZeroMemory(void* p, size_t n)
{
	return memset32_sse2(p, 0, n);
}

void* X86ZeroMemory(void* p, size_t n)
{
	return my_memset_inline(p, 0, n);
}

static void CopyWithAVXNoCache(uint8_t* dst, uint8_t* src, size_t size)
{
	size_t stride = 2 * sizeof(__m256i);
	while (size)
	{
		__m256i a = _mm256_load_si256((__m256i*)src + 0);
		__m256i b = _mm256_load_si256((__m256i*)src + 1);
		_mm256_stream_si256((__m256i*)dst + 0, a);
		_mm256_stream_si256((__m256i*)dst + 1, b);

		size -= stride;
		src += stride;
		dst += stride;
	}
}

static void* CopyMemorySSE(void *dst, const void *src, size_t size)
{
	if (size <= 96)
	{
		if (size >= 16)
		{
			if (size > 16)
			{
				if (size > 32)
				{
					const __m128i xmm0 = _mm_loadu_si128((__m128i*)src);
					const __m128i xmm1 = _mm_loadu_si128((__m128i*)((char*)src + 16));
					const __m128i xmm6 = _mm_loadu_si128((__m128i*)((char*)src + size - 32));
					const __m128i xmm7 = _mm_loadu_si128((__m128i*)((char*)src + size - 16));
					if (size > 64)
					{
						const __m128i xmm2 = _mm_loadu_si128((__m128i*)((char*)src + 32));
						const __m128i xmm5 = _mm_loadu_si128((__m128i*)((char*)src + size - 48));
						_mm_storeu_si128((__m128i*)dst, xmm0);
						_mm_storeu_si128((__m128i*)((char*)dst + 16), xmm1);
						_mm_storeu_si128((__m128i*)((char*)dst + 32), xmm2);
						_mm_storeu_si128((__m128i*)((char*)dst + size - 48), xmm5);
						_mm_storeu_si128((__m128i*)((char*)dst + size - 32), xmm6);
						_mm_storeu_si128((__m128i*)((char*)dst + size - 16), xmm7);
						return dst;
					}
					_mm_storeu_si128((__m128i*)dst, xmm0);
					_mm_storeu_si128((__m128i*)((char*)dst + 16), xmm1);
					_mm_storeu_si128((__m128i*)((char*)dst + size - 32), xmm6);
					_mm_storeu_si128((__m128i*)((char*)dst + size - 16), xmm7);
					return dst;
				}
				// The order has been mixed so the compiler will not re-order the statements!
				// I don't want the compiler to separate these statements, because they are faster when used in groups of two on Core i!
				const __m128i xmm7 = _mm_loadu_si128((__m128i*)((char*)src + size - 16));
				const __m128i xmm0 = _mm_loadu_si128((__m128i*)src);
				_mm_storeu_si128((__m128i*)((char*)dst + size - 16), xmm7);
				_mm_storeu_si128((__m128i*)dst, xmm0);
				return dst;
			}
			_mm_storeu_si128((__m128i*)dst, _mm_loadu_si128((__m128i*)src));
			return dst;
		}
		if (size >= 8)
		{
			long long rax = *(long long*)src;
			if (size > 8)
			{
				long long rcx = *(long long*)((char*)src + size - 8);
				*(long long*)dst = rax;
				*(long long*)((char*)dst + size - 8) = rcx;
			}
			else *(long long*)dst = rax;
		}
		else if (size >= 4)
		{
			int eax = *(int*)src;
			if (size > 4)
			{
				int ecx = *(int*)((char*)src + size - 4);
				*(int*)dst = eax;
				*(int*)((char*)dst + size - 4) = ecx;
			}
			else *(int*)dst = eax;
		}
		else if (size >= 1)
		{
			char al = *(char*)src;
			if (size > 1)
			{
				short cx = *(short*)((char*)src + size - 2);
				*(char*)dst = al;
				*(short*)((char*)dst + size - 2) = cx;
			}
			else *(char*)dst = al;
		}
		return dst;
	}
	else
	{
		void * const ret = dst;
		if (size < (1024 * 256))
		{
			long long offset = (long long)(size & -0x40);		// "Round down to nearest multiple of 64"
			dst = (char*)dst + offset;							// "Point to the end"
			src = (char*)src + offset;							// "Point to the end"
			size -= offset;										// "Remaining data after loop"
			offset = -offset;									// "Negative index from the end"

			do
			{
				const __m128i xmm0 = _mm_loadu_si128((__m128i*)((char*)src + offset));
				const __m128i xmm1 = _mm_loadu_si128((__m128i*)((char*)src + offset + 16));
				const __m128i xmm2 = _mm_loadu_si128((__m128i*)((char*)src + offset + 32));
				const __m128i xmm3 = _mm_loadu_si128((__m128i*)((char*)src + offset + 48));
				_mm_storeu_si128((__m128i*)((char*)dst + offset), xmm0);
				_mm_storeu_si128((__m128i*)((char*)dst + offset + 16), xmm1);
				_mm_storeu_si128((__m128i*)((char*)dst + offset + 32), xmm2);
				_mm_storeu_si128((__m128i*)((char*)dst + offset + 48), xmm3);
			} while (offset += 64);

			if (size >= 16)
			{
				if (size > 16)
				{
					if (size > 32)
					{
						const __m128i xmm0 = _mm_loadu_si128((__m128i*)src);
						const __m128i xmm1 = _mm_loadu_si128((__m128i*)((char*)src + 16));
						const __m128i xmm6 = _mm_loadu_si128((__m128i*)((char*)src + size - 32));
						const __m128i xmm7 = _mm_loadu_si128((__m128i*)((char*)src + size - 16));
						_mm_storeu_si128((__m128i*)dst, xmm0);
						_mm_storeu_si128((__m128i*)((char*)dst + 16), xmm1);
						_mm_storeu_si128((__m128i*)((char*)dst + size - 32), xmm6);
						_mm_storeu_si128((__m128i*)((char*)dst + size - 16), xmm7);
						return ret;
					}
					// The order has been mixed so the compiler will not re-order the statements!
					const __m128i xmm7 = _mm_loadu_si128((__m128i*)((char*)src + size - 16));
					const __m128i xmm0 = _mm_loadu_si128((__m128i*)src);
					_mm_storeu_si128((__m128i*)((char*)dst + size - 16), xmm7);
					_mm_storeu_si128((__m128i*)dst, xmm0);
					return ret;
				}
				_mm_storeu_si128((__m128i*)dst, _mm_loadu_si128((__m128i*)src));
				return ret;
			}
		}
		else	// do forward streaming copy/move
		{
			// We MUST do prealignment on streaming copies!
			const size_t prealign = -(size_t)dst & 0xf;
			if (prealign)
			{
				if (prealign >= 8)
				{
					long long rax = *(long long*)src;
					if (prealign > 8)
					{
						long long rcx = *(long long*)((char*)src + prealign - 8);
						*(long long*)dst = rax;
						*(long long*)((char*)dst + prealign - 8) = rcx;
					}
					else *(long long*)dst = rax;
				}
				else if (prealign >= 4)
				{
					int eax = *(int*)src;
					if (prealign > 4)
					{
						int ecx = *(int*)((char*)src + prealign - 4);
						*(int*)dst = eax;
						*(int*)((char*)dst + prealign - 4) = ecx;
					}
					else *(int*)dst = eax;
				}
				else
				{
					char al = *(char*)src;
					if (prealign > 1)
					{
						short cx = *(short*)((char*)src + prealign - 2);
						*(char*)dst = al;
						*(short*)((char*)dst + prealign - 2) = cx;
					}
					else *(char*)dst = al;
				}
				src = (char*)src + prealign;
				dst = (char*)dst + prealign;
				size -= prealign;
			}

			// Begin prefetching upto 4KB
			for (long long offset = 0; offset < 4096; offset += 256)
			{
				_mm_prefetch(((char*)src + offset), _MM_HINT_NTA);
				_mm_prefetch(((char*)src + offset + 64), _MM_HINT_NTA);
				_mm_prefetch(((char*)src + offset + 128), _MM_HINT_NTA);
				_mm_prefetch(((char*)src + offset + 192), _MM_HINT_NTA);
			}

			long long offset = (long long)(size & -0x40);		// "Round down to nearest multiple of 64"
			size -= offset;										// "Remaining data after loop"
			offset -= 4096;										// stage 1 INCLUDES prefetches
			dst = (char*)dst + offset;							// "Point to the end"
			src = (char*)src + offset;							// "Point to the end"
			offset = -offset;									// "Negative index from the end"

			do													// stage 1 ~~ WITH prefetching
			{
				_mm_prefetch((char*)src + offset + 4096, _MM_HINT_NTA);
				const __m128i xmm0 = _mm_loadu_si128((__m128i*)((char*)src + offset));
				const __m128i xmm1 = _mm_loadu_si128((__m128i*)((char*)src + offset + 16));
				const __m128i xmm2 = _mm_loadu_si128((__m128i*)((char*)src + offset + 32));
				const __m128i xmm3 = _mm_loadu_si128((__m128i*)((char*)src + offset + 48));
				_mm_stream_si128((__m128i*)((char*)dst + offset), xmm0);
				_mm_stream_si128((__m128i*)((char*)dst + offset + 16), xmm1);
				_mm_stream_si128((__m128i*)((char*)dst + offset + 32), xmm2);
				_mm_stream_si128((__m128i*)((char*)dst + offset + 48), xmm3);
			} while (offset += 64);

			offset = -4096;
			dst = (char*)dst + 4096;
			src = (char*)src + 4096;

			_mm_prefetch(((char*)src + size - 64), _MM_HINT_NTA);	// prefetch the final tail section

			do													// stage 2 ~~ WITHOUT further prefetching
			{
				const __m128i xmm0 = _mm_loadu_si128((__m128i*)((char*)src + offset));
				const __m128i xmm1 = _mm_loadu_si128((__m128i*)((char*)src + offset + 16));
				const __m128i xmm2 = _mm_loadu_si128((__m128i*)((char*)src + offset + 32));
				const __m128i xmm3 = _mm_loadu_si128((__m128i*)((char*)src + offset + 48));
				_mm_stream_si128((__m128i*)((char*)dst + offset), xmm0);
				_mm_stream_si128((__m128i*)((char*)dst + offset + 16), xmm1);
				_mm_stream_si128((__m128i*)((char*)dst + offset + 32), xmm2);
				_mm_stream_si128((__m128i*)((char*)dst + offset + 48), xmm3);
			} while (offset += 64);

			if (size >= 16)
			{
				const __m128i xmm0 = _mm_loadu_si128((__m128i*)src);
				if (size > 16)
				{
					if (size > 32)
					{
						const __m128i xmm1 = _mm_loadu_si128((__m128i*)((char*)src + 16));
						const __m128i xmm6 = _mm_loadu_si128((__m128i*)((char*)src + size - 32));
						const __m128i xmm7 = _mm_loadu_si128((__m128i*)((char*)src + size - 16));
						_mm_stream_si128((__m128i*)dst, xmm0);
						_mm_stream_si128((__m128i*)((char*)dst + 16), xmm1);
						_mm_storeu_si128((__m128i*)((char*)dst + size - 32), xmm6);
						_mm_storeu_si128((__m128i*)((char*)dst + size - 16), xmm7);
						return ret;
					}
					const __m128i xmm7 = _mm_loadu_si128((__m128i*)((char*)src + size - 16));
					_mm_stream_si128((__m128i*)dst, xmm0);
					_mm_storeu_si128((__m128i*)((char*)dst + size - 16), xmm7);
					return ret;
				}
				_mm_stream_si128((__m128i*)dst, xmm0);
				return ret;
			}
		}

		if (size >= 8)
		{
			long long rax = *(long long*)src;
			if (size > 8)
			{
				long long rcx = *(long long*)((char*)src + size - 8);
				*(long long*)dst = rax;
				*(long long*)((char*)dst + size - 8) = rcx;
			}
			else *(long long*)dst = rax;
		}
		else if (size >= 4)
		{
			int eax = *(int*)src;
			if (size > 4)
			{
				int ecx = *(int*)((char*)src + size - 4);
				*(int*)dst = eax;
				*(int*)((char*)dst + size - 4) = ecx;
			}
			else *(int*)dst = eax;
		}
		else if (size >= 1)
		{
			char al = *(char*)src;
			if (size > 1)
			{
				short cx = *(short*)((char*)src + size - 2);
				*(char*)dst = al;
				*(short*)((char*)dst + size - 2) = cx;
			}
			else *(char*)dst = al;
		}
		return ret;
	}
}

///////////////////////////////////////////////////////////////////


NTSTATUS NTAPI SenaHook::ApplyOptimization()
{
	
}


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


#define IDM_INFO     40001  
#define IDM_NVIDIA   40000  
#define IDM_INTEL    40003
#define IDM_COMMON   40004
#define IDM_CHS      40005
#define IDM_CHT      40006

class tTJSCriticalSection
{
	CRITICAL_SECTION CS;
public:
	tTJSCriticalSection() { InitializeCriticalSection(&CS); }
	~tTJSCriticalSection() { DeleteCriticalSection(&CS); }

	void Enter() { EnterCriticalSection(&CS); }
	void Leave() { LeaveCriticalSection(&CS); }
};


class tTJSCriticalSectionHolder
{
	tTJSCriticalSection *Section;
public:
	tTJSCriticalSectionHolder(tTJSCriticalSection &cs)
	{
		Section = &cs;
		Section->Enter();
	}

	~tTJSCriticalSectionHolder()
	{
		Section->Leave();
	}

};

static tTJSCriticalSection XmoeWriteConfig;

BOOL FileIsExist(LPCWSTR FileName)
{
	return Nt_GetFileAttributes(FileName) == INVALID_FILE_ATTRIBUTES ? FALSE : TRUE;
}

BOOL WriteConfig(ULONG Optimization, ULONG UseTraditionalChinese)
{
	tTJSCriticalSectionHolder CSHolder(XmoeWriteConfig);

	NtFileDisk File;

	if (NT_FAILED(File.Create(L"Config.int")))
		return FALSE;

	File.Write(&Optimization,          sizeof(Optimization));
	File.Write(&UseTraditionalChinese, sizeof(UseTraditionalChinese));
	File.Close();
	return TRUE;
}

BOOL ReadConfig(ULONG& Optimization, ULONG& UseTraditionalChinese)
{
	NtFileDisk File;
	BOOL       NeedModify;
	
	if (NT_FAILED(File.Open(L"Config.int")) || File.GetSize32() < 8)
	{
		Optimization          = SenaHook::OP_COMMON;
		UseTraditionalChinese = FALSE;
		File.Close();

		return WriteConfig(Optimization, UseTraditionalChinese);
	}
	else
	{
		File.Read(&Optimization,          sizeof(ULONG));
		File.Read(&UseTraditionalChinese, sizeof(ULONG));
		NeedModify = FALSE;

		if (!IN_RANGE(SenaHook::OP_COMMON, Optimization, SenaHook::OP_INTEL))
		{
			Optimization = SenaHook::OP_COMMON;
			NeedModify = TRUE;
		}
		if (!IN_RANGE(FALSE, UseTraditionalChinese, TRUE))
		{
			UseTraditionalChinese = FALSE;
			NeedModify = TRUE;
		}
		File.Close();
		
		if (NeedModify)
			return WriteConfig(Optimization, UseTraditionalChinese);
	}
	File.Close();
	return TRUE;
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
OldWindowProc(NULL),
OldMainWindowProc(NULL),
hInfoWnd(NULL),
CurIndex(NULL),
PrivateMemory(NULL),
OptimizationMode(OP_COMMON),
OptimizationFlag(0),
XmoeCopyMemory(my_memcpy_inline),
XmoeZeroMemory(X86ZeroMemory),
UseTraditionalChinese(FALSE)
{
	OriHeight = 720;
	OriWidth  = 1280;

	hMenuMain = CreateMenu();
	hMenuPop  = CreateMenu();
	hMenuPop2 = CreateMenu();

	ULONG ShadowOptimizationFlag, ShadowUseTraditionalChinese;

	ReadConfig(ShadowOptimizationFlag, ShadowUseTraditionalChinese);

	OptimizationFlag      = ShadowOptimizationFlag;
	UseTraditionalChinese = ShadowUseTraditionalChinese;

	if (!UseTraditionalChinese)
	{
		AppendMenuW(hMenuPop,  MF_STRING, IDM_NVIDIA,         L"针对Nvidia");
		AppendMenuW(hMenuPop,  MF_STRING, IDM_INTEL,          L"针对Intel");
		AppendMenuW(hMenuPop,  MF_STRING, IDM_COMMON,         L"通用方案");
		AppendMenuW(hMenuPop2, MF_STRING, IDM_CHS,            L"简体中文");
		AppendMenuW(hMenuPop2, MF_STRING, IDM_CHT,            L"繁体中文");
		AppendMenuW(hMenuMain, MF_POPUP, (UINT_PTR)hMenuPop,  L"优化模式");
		AppendMenuW(hMenuMain, MF_POPUP, (UINT_PTR)hMenuPop2, L"选择语言");
		AppendMenuW(hMenuMain, MF_STRING, IDM_INFO,           L"汉化信息");
	}
	else
	{
		AppendMenuW(hMenuPop,  MF_STRING, IDM_NVIDIA,         L"針對Nvidia");
		AppendMenuW(hMenuPop,  MF_STRING, IDM_INTEL,          L"針對Intel");
		AppendMenuW(hMenuPop,  MF_STRING, IDM_COMMON,         L"通用方案");
		AppendMenuW(hMenuPop2, MF_STRING, IDM_CHS,            L"簡體中文");
		AppendMenuW(hMenuPop2, MF_STRING, IDM_CHT,            L"繁體中文");
		AppendMenuW(hMenuMain, MF_POPUP, (UINT_PTR)hMenuPop,  L"優化模式");
		AppendMenuW(hMenuMain, MF_POPUP, (UINT_PTR)hMenuPop2, L"選擇語言");
		AppendMenuW(hMenuMain, MF_STRING, IDM_INFO,           L"漢化信息");
	}
	if (!CreateCompiler(&XmoeCompiler))
	{
		MessageBoxW(NULL, L"Failed to create X'moe compiler instance!",
			L"X'moe common lib", MB_OK | MB_ICONERROR);
		Ps::ExitProcess(-1);
	}

	//XmoeCompiler->LoadText(NULL);
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

NTSTATUS NTAPI SenaHook::ChangeToTraditionalChinese()
{
	if (UseTraditionalChinese)
		return STATUS_SUCCESS;

	InterlockedExchange((PLONG)&UseTraditionalChinese, TRUE);
	MessageBoxW(hWnd, L"成功切換到繁體中文模式。\n下次啟動生效。\n注意：本模式僅僅將文字轉換為繁體中文。", L"(Information)", MB_OK | MB_ICONINFORMATION);
	WriteConfig(OptimizationFlag, UseTraditionalChinese);
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI SenaHook::ChangeToSimplifiedChinese()
{
	if (!UseTraditionalChinese)
		return STATUS_SUCCESS;

	InterlockedExchange((PLONG)&UseTraditionalChinese, FALSE);
	MessageBoxW(hWnd, L"成功切换到简体中文模式。\n下次启动生效。", L"(Information)", MB_OK);
	WriteConfig(OptimizationFlag, UseTraditionalChinese);
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI SenaHook::ReadOrCreateConfig()
{
	return STATUS_SUCCESS;
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
	ULONG      len;
	CHAR       mbchs[2];
	HFONT      hFont, hOldFont;
	LOGFONTW*  lplf;
	WCHAR      OutChar;
	UINT       cp = 936;


	auto ToTraditionalChinese = [](WCHAR InChar, WCHAR& OutChar)->NTSTATUS
	{
		USHORT LangID = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
		LCID   Locale = MAKELCID(LangID, SORT_CHINESE_PRCP);
		
		OutChar= L'?';

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
		if (SenaHook::GetSenaHook()->UseTraditionalChinese)
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


int WINAPI HookMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	WCHAR  Text[400];
	WCHAR  Caption[200];

	SenaHook*  GlobalData = SenaHook::GetSenaHook();

	GlobalData->XmoeZeroMemory(Text, countof(Text) * sizeof(Text[0]));
	GlobalData->XmoeZeroMemory(Caption, countof(Caption) * sizeof(Caption[0]));

	MultiByteToWideChar(CP_ACP, 0, lpCaption, StrLengthA(lpCaption), Caption, countof(Caption));
	MultiByteToWideChar(932, 0, lpText, StrLengthA(lpText), Text, countof(Text));

	return MessageBoxW(hWnd, Text, Caption, uType);
}


BOOL WINAPI HookSetWindowTextA(HWND hWnd, LPCSTR lpString)
{
	if (hWnd == SenaHook::GetSenaHook()->hWnd && SenaHook::GetSenaHook()->IsWindowInited)
	{
		return SetWindowTextW(hWnd, L"[X'moe第三制药厂]想要传达给你的爱恋Ver2.0最终版(本补丁未参与任何收费活动，请勿直播本补丁内容)");
	}
	return SetWindowTextA(hWnd, lpString);
}


#if 0
__global__ static Void RGBToTexture(PBYTE SourceBits, PBYTE TargetBits)
{
}
#endif

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
	SenaHook*            GlobalData;

	GlobalData = SenaHook::GetSenaHook();

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
		GlobalData->XmoeCopyMemory(TargetBits, SourceBits, TargetBitsLength);
	}
	else // if (GlobalData->OptimizationMode != SenaHook::OP_NV)
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

	SenaHook*   GlobalData;

	GlobalData = SenaHook::GetSenaHook();

	AlignWidth = (Width * 32 + 31) / 32;
	BufferSize = 4 * AlignWidth * Height;
	TempBuffer = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BufferSize);

	WidthLen = Width * (BitCount / 8);
	for (ULONG i = 0; i < Height; i++)
	{
		GlobalData->XmoeCopyMemory(&TempBuffer[(((Height - i) - 1) * WidthLen)], &Data[WidthLen * i], WidthLen);
	}
	GlobalData->XmoeCopyMemory(Data, TempBuffer, BufferSize);

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

	SenaHook* GlobalData = SenaHook::GetSenaHook();

	GlobalData->XmoeZeroMemory(ConvertName, MAX_PATH);
	StrCopyA(ConvertName, FileName);
	NameLength = StrLengthA(ConvertName);

	ConvertName[NameLength - 3] = 'b';
	ConvertName[NameLength - 2] = 'm';
	ConvertName[NameLength - 1] = 'p';

	ULONG FileSize = 0;
	PBYTE FileBuffer = 0;

	if (FileManager::GetFileManager()->QueryFile(ConvertName, FileBuffer, FileSize))
	{
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

	SenaHook::GetSenaHook()->XmoeZeroMemory(TmpBuffer, *OutputLength);

	for (ULONG i = 0; i<Height; i++)
	{
		SourceLines = &SourceDIB[nAlignWidth * i * 4];
		TargetLines = &TmpBuffer[nNewAlignWidth * i * 4];

		for (ULONG j = 0; j<Width; j++)
		{
			//TargetLines[0] = SourceLines[0];
			//TargetLines[1] = SourceLines[1];
			//TargetLines[2] = SourceLines[2];
			//TargetLines[3] = SourceLines[3];
			//RtlCopyMemory(TargetLines, SourceLines, sizeof(DWORD));
			*(PDWORD)TargetLines = *(PDWORD)SourceLines;
			SourceLines += 4;
			TargetLines += 4;
		}
	}
}

ULONG FixedList[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
#define FixedListLength 13

void PALCopyImage(PBYTE RenderBuffer, ULONG NewWidth)
{
	PBYTE     Output = NULL;
	ULONG     OutputLength;
	ULONG     FixedWidth = NewWidth;
	BOOL      Found = FALSE;
	SenaHook* GlobalData;

	GlobalData = SenaHook::GetSenaHook();

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

		GlobalData->XmoeCopyMemory(RenderBuffer, Output, OutputLength);
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
		push[esp + 0];
		pop  g_ScreenWidth;
		call g_FilterDrawCall4;
		pushad;
		push g_ScreenWidth;
		push dword ptr[esp + 0x4];
		call PALCopyImage;
		add esp, 0x8;
		popad;
		jmp g_FilterDrawRet4;
	}
}

//fake
ASM VOID UpdateGraphFilter3()
{
	INLINE_ASM
	{
		push[esp + 4];
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
		push[esp + 8];
		pop  g_ScreenWidth;
		call g_FilterDrawCall2;
		pushad;
		push g_ScreenWidth;
		push dword ptr[esp + 4];
		call PALCopyImage;
		add esp, 0x8;
		popad;
		jmp g_FilterDrawRet2;
	}
}

//fake
ASM VOID UpdateGraphFilter1()
{
	INLINE_ASM
	{
		push[esp + 8];
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


void WINAPI SetNopCode(BYTE* pnop, ULONG_PTR size)
{
	DWORD      oldProtect;

	VirtualProtect((PVOID)pnop, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	for (ULONG_PTR i = 0; i<size; i++)
	{
		pnop[i] = 0x90;
	}
}

VOID WINAPI HookPalFontSetType(HMODULE hModule)
{
	BYTE* pfPalFontSetType = (BYTE*)GetProcAddress((HMODULE)hModule, "PalFontSetType");
	SetNopCode((pfPalFontSetType + 0x25), 7);
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
	return CreateFontW(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic,
		bUnderline, bStrikeOut, GB2312_CHARSET, iOutPrecision, iClipPrecision,
		iQuality, iPitchAndFamily, L"黑体");
}


ATOM WINAPI HookRegisterClassExA(WNDCLASSEXA *lpWndClass)
{
	WNDCLASSEXW  ClassInfo;
	LPWSTR       MenuName, ClassName;
	SenaHook*    GlobalData;

	GlobalData = SenaHook::GetSenaHook();

	MenuName  = (LPWSTR)AllocStack(MAX_PATH * 2);
	ClassName = (LPWSTR)AllocStack(MAX_PATH * 2);

	GlobalData->XmoeZeroMemory(MenuName,  MAX_PATH * 2);
	GlobalData->XmoeZeroMemory(ClassName, MAX_PATH * 2);

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

	return RegisterClassExW(&ClassInfo);
}



NTSTATUS NTAPI SenaHook::ChangeToCommonMode()
{
	NTSTATUS  Status;
	Status = STATUS_SUCCESS;
	InterlockedExchange(&OptimizationMode, OP_COMMON);
	InterlockedExchange((PLONG)&XmoeCopyMemory, (LONG)my_memcpy_inline);
	InterlockedExchange((PLONG)&XmoeZeroMemory, (LONG)X86ZeroMemory);
	WriteConfig(OptimizationFlag, UseTraditionalChinese);
	return Status;
}

NTSTATUS NTAPI SenaHook::ChangeToIntelMode()
{
	NTSTATUS  Status;
	if (FLAG_ON(OptimizationFlag, SENA_HAS_INTEL))
	{
		InterlockedExchange(&OptimizationMode, OP_INTEL);
		InterlockedExchange((PLONG)&XmoeCopyMemory, (LONG)CopyMemorySSE);
		InterlockedExchange((PLONG)&XmoeZeroMemory, (LONG)SSE2ZeroMemory);
		WriteConfig(OptimizationFlag, UseTraditionalChinese);
		Status = STATUS_SUCCESS;
	}
	else
	{
		MessageBoxW(hWnd, L"无法切换到Intel加速模式，因为硬件不支持", L"不支持本模式", MB_OK | MB_ICONERROR);
		Status = STATUS_UNSUCCESSFUL;
	}
	return Status;
}

NTSTATUS NTAPI SenaHook::ChangeToNVIDIAMode()
{
	NTSTATUS  Status;
	if (FLAG_ON(OptimizationFlag, SENA_HAS_NVIDIA))
	{
		InterlockedExchange(&OptimizationMode, OP_NV);
		InterlockedExchange((PLONG)&XmoeCopyMemory, (LONG)CopyMemorySSE);
		InterlockedExchange((PLONG)&XmoeZeroMemory, (LONG)SSE2ZeroMemory);
		WriteConfig(OptimizationFlag, UseTraditionalChinese);
		Status = STATUS_SUCCESS;
	}
	else
	{
		MessageBoxW(hWnd, L"无法切换到NVIDIA(CUDA)加速模式，因为硬件不支持", L"不支持本模式", MB_OK | MB_ICONERROR);
		Status = STATUS_UNSUCCESSFUL;
	}
	
	return Status;
}


#include "resource.h"
#define ID_TIMER 233
#define FRAME_COUNT 117

Void DrawBmp(HDC hDC, int nWidth, int nHeight, int Index)
{
	BITMAP            bm;
	HDC               hdcImage;
	HDC               hdcMEM;

	HBITMAP  bitmap = LoadBitmapW(SenaHook::GetSenaHook()->hSelfModule, 
		MAKEINTRESOURCEW(IDB_BITMAP1 + SenaHook::GetSenaHook()->CurIndex));

	hdcMEM      = CreateCompatibleDC(hDC);
	hdcImage    = CreateCompatibleDC(hDC);
	HBITMAP bmp = CreateCompatibleBitmap(hDC, nWidth, nHeight);
	GetObjectW(bitmap, sizeof(bm), (LPSTR)&bm);
	SelectObject(hdcMEM, bmp);
	SelectObject(hdcImage, bitmap);
	StretchBlt(hdcMEM, 0, 0, nWidth, nHeight, hdcImage, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	StretchBlt(hDC, 22, 20, nWidth, nHeight, hdcMEM, 0, 0, nWidth, nHeight, SRCCOPY);

	DeleteObject(bmp);
	DeleteObject(hdcImage);
	DeleteObject(bitmap);
	DeleteDC(hdcImage);
	DeleteDC(hdcMEM);
}

INT_PTR CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH hBrush;
	static INT    cxClient, cyClient;
	PAINTSTRUCT ps;

	switch (msg)
	{
	case WM_INITDIALOG:
		SenaHook::GetSenaHook()->CurIndex = 0;
		//30fps
		SetTimer(hWnd, ID_TIMER, 33, NULL);
		SendMessageW(hWnd, WM_SETICON, (WPARAM)TRUE, 
			(LPARAM)LoadIconW(SenaHook::GetSenaHook()->hSelfModule, MAKEINTRESOURCEW(IDI_ICON1)));
		break;

	case WM_CREATE:
		break;

	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		break;

	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		DrawBmp(GetDC(hWnd), 520, 292, SenaHook::GetSenaHook()->CurIndex);
		EndPaint(hWnd, &ps);
		break;

	case WM_SYSCOMMAND:
	{
		switch (wParam)
		{
		case SC_CLOSE:
			DestroyWindow(hWnd);
			SenaHook::GetSenaHook()->hInfoWnd = NULL;
			break;
		}
	}
	break;

	case WM_DESTROY:
		SenaHook::GetSenaHook()->hInfoWnd = NULL;
		KillTimer(hWnd, ID_TIMER);
		break;

	case WM_TIMER:
		switch (wParam) 
		{
		case ID_TIMER:
			SenaHook::GetSenaHook()->CurIndex++;
			SenaHook::GetSenaHook()->CurIndex %= FRAME_COUNT;
			RECT Rect;
			Rect.left   = 22;
			Rect.top    = 20;
			Rect.right  = 542;
			Rect.bottom = 312;
			InvalidateRect(hWnd, &Rect, FALSE);
			break;
		}
		break;

	case WM_COMMAND:
	{
		WORD WmId    = LOWORD(wParam);
		WORD WmEvent = HIWORD(wParam);

		switch (WmId)
		{
		case IDOK:
			DestroyWindow(hWnd);
			SenaHook::GetSenaHook()->hInfoWnd = NULL;
			break;
		}
	}
	break;
	}
	return 0;
}

NTSTATUS NTAPI SenaHook::CreateInfoWindowAndCheck(HWND hWnd)
{
	NTSTATUS  Status;

	if (hWnd == NULL)
		return STATUS_UNSUCCESSFUL;

	if (hInfoWnd != NULL)
	{
		ShowWindow(hInfoWnd, SW_SHOW);
		BringWindowToTop(hInfoWnd);
		return STATUS_SUCCESS;
	}

	LOOP_ONCE
	{
		Status = STATUS_UNSUCCESSFUL;
		
		CurIndex = 0;
		hInfoWnd = CreateDialogParamW(hSelfModule, MAKEINTRESOURCEW(IDD_SENA_INFO), hWnd, DlgProc, WM_INITDIALOG);
		MessageBeep(0);
		if (!hInfoWnd)
		{
			MessageBoxW(hWnd, L"本汉化补丁由X'moe汉化组制作\n"
							  L"请勿将本补丁用于商业目的\n"
							  L"同时请勿直播本补丁内容", L"汉化信息", 0);

			break;
		}
		ShowWindow(hInfoWnd, SW_SHOW);
		UpdateWindow(hInfoWnd);

	}

	return Status;
}



BOOL
WINAPI
HookSetWindowPos(
_In_ HWND hWnd,
_In_opt_ HWND hWndInsertAfter,
_In_ int X,
_In_ int Y,
_In_ int cx,
_In_ int cy,
_In_ UINT uFlags)
{
	ULONG Address;

	if ((SenaHook::GetSenaHook()->hWnd == hWnd))
	{
		Address = (ULONG)_ReturnAddress() - (ULONG)Nt_GetModuleHandle(L"PAL.DLL");
		if (Address == 0xA8BBC)
			SetMenu(hWnd, NULL);
		return SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
	}
	else
	{
		return SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
	}
}

HWND WINAPI HookCreateWindowExA_Dll(
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
	if (StrCompareA(lpClassName, "Koikake_chs") == 0)
	{
		INT32 Height = GetSystemMetrics(SM_CYMENUSIZE);

		SenaHook::GetSenaHook()->hWnd = CreateWindowExW(
			dwExStyle,
			L"Koikake_chs",
			L"[X'moe第三制药厂]想要传达给你的爱恋Ver2.0最终版(本补丁未参与任何收费活动，请勿直播本补丁内容)",
			dwStyle,
			x,
			y,
			nWidth,
			nHeight + Height,
			hWndParent,
			SenaHook::GetSenaHook()->hMenuMain,
			hInstance,
			lpParam
			);

		SenaHook::GetSenaHook()->OriWidth  = nWidth;
		SenaHook::GetSenaHook()->OriHeight = nHeight;
		
		//SetWindowPos(SenaHook::GetSenaHook()->hWnd, HWND_TOPMOST, 0, 0, 1280, 720 + Height, SWP_NOMOVE | SWP_NOZORDER);

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


LRESULT WINAPI HookDefWindowProcA(
	_In_ HWND hWnd,
	_In_ UINT Msg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam)
{
	if (hWnd == SenaHook::GetSenaHook()->hWnd && hWnd && SenaHook::GetSenaHook()->hWnd)
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	else 
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
}


LRESULT WINAPI HookDispatchMessageA(MSG *lpMsg)
{
	if (lpMsg->hwnd == SenaHook::GetSenaHook()->hWnd && lpMsg->hwnd && SenaHook::GetSenaHook()->hWnd)
		return DispatchMessageW(lpMsg);
	else
		return DispatchMessageA(lpMsg);
}


BOOL WINAPI HookPeekMessageA(
_Out_ LPMSG lpMsg,
_In_opt_ HWND hWnd,
_In_ UINT wMsgFilterMin,
_In_ UINT wMsgFilterMax,
_In_ UINT wRemoveMsg)
{
	if (hWnd == SenaHook::GetSenaHook()->hWnd && hWnd && SenaHook::GetSenaHook()->hWnd)
		return PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	else
		return PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg); 
}




BOOL WINAPI HookPostMessageA(
_In_opt_ HWND hWnd,
_In_ UINT Msg,
_In_ WPARAM wParam,
_In_ LPARAM lParam)
{
	if (hWnd == SenaHook::GetSenaHook()->hWnd && hWnd && SenaHook::GetSenaHook()->hWnd)
		return PostMessageW(hWnd, Msg, wParam, lParam);
	else
		return PostMessageW(hWnd, Msg, wParam, lParam);
}

INT NTAPI KoikakeWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	SenaHook::GetSenaHook()->hWnd = hWnd;
	int      wmId, wmEvent;
	ULONG    Previous;
	NTSTATUS Status;
	HMENU    Menu     = GetMenu(hWnd);
	HMENU    SubMenu  = GetSubMenu(Menu, 0);
	HMENU    SubMenu2 = GetSubMenu(Menu, 1);

	switch (Msg)
	{
	case WM_CREATE:
		switch (SenaHook::GetSenaHook()->OptimizationMode)
		{
		default:
		case SenaHook::OP_COMMON:
			CheckMenuItem(SubMenu, IDM_COMMON, MF_CHECKED);
			break;

		case SenaHook::OP_INTEL:
			CheckMenuItem(SubMenu, IDM_INTEL, MF_CHECKED);
			break;

		case SenaHook::OP_NV:
			CheckMenuItem(SubMenu, IDM_NVIDIA, MF_CHECKED);
			break;
		}
		
		switch (SenaHook::GetSenaHook()->UseTraditionalChinese)
		{
		case FALSE:
			CheckMenuItem(SubMenu2, IDM_CHS, MF_CHECKED);
			break;

		default:
			CheckMenuItem(SubMenu2, IDM_CHT, MF_CHECKED);
			break;
		}
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch (wmId)
		{
		case IDM_INFO:
			SenaHook::GetSenaHook()->CreateInfoWindowAndCheck(hWnd);
			break;

		case IDM_COMMON:
			CheckMenuItem(SubMenu, IDM_COMMON, MF_CHECKED);
			CheckMenuItem(SubMenu, IDM_INTEL,  MF_UNCHECKED);
			CheckMenuItem(SubMenu, IDM_NVIDIA, MF_UNCHECKED);
			SenaHook::GetSenaHook()->ChangeToCommonMode();
			break;

		case IDM_INTEL:
			Previous = SenaHook::GetSenaHook()->OptimizationFlag;
			Status   = SenaHook::GetSenaHook()->ChangeToIntelMode();
			if (NT_SUCCESS(Status))
			{
				CheckMenuItem(SubMenu, IDM_COMMON, MF_UNCHECKED);
				CheckMenuItem(SubMenu, IDM_INTEL, MF_CHECKED);
				CheckMenuItem(SubMenu, IDM_NVIDIA, MF_UNCHECKED);
			}
			break;

		case IDM_NVIDIA:
			Previous = SenaHook::GetSenaHook()->OptimizationFlag;
			Status   = SenaHook::GetSenaHook()->ChangeToNVIDIAMode();
			if (NT_SUCCESS(Status))
			{
				CheckMenuItem(SubMenu, IDM_COMMON, MF_UNCHECKED);
				CheckMenuItem(SubMenu, IDM_INTEL, MF_UNCHECKED);
				CheckMenuItem(SubMenu, IDM_NVIDIA, MF_CHECKED);
			}
			break;

		case IDM_CHS:
			CheckMenuItem(SubMenu2, IDM_CHS, MF_CHECKED);
			CheckMenuItem(SubMenu2, IDM_CHT, MF_UNCHECKED);
			SenaHook::GetSenaHook()->ChangeToSimplifiedChinese();
			break;

		case IDM_CHT:
			CheckMenuItem(SubMenu2, IDM_CHS, MF_UNCHECKED);
			CheckMenuItem(SubMenu2, IDM_CHT, MF_CHECKED);
			SenaHook::GetSenaHook()->ChangeToTraditionalChinese();
			break;
		}
		break;

	}
	return SenaHook::GetSenaHook()->OldMainWindowProc(hWnd, Msg, wParam, lParam);
}



typedef INT(CDECL* PalVideoPlayProc)(LPSTR, DWORD);
PalVideoPlayProc PalVideoPlay = NULL;
INT CDECL HookPalVideoPlay(LPSTR FileName, DWORD Unk)
{
	static PChar MovieName = "koikake.wmv";
	RtlCopyMemory(FileName, MovieName, StrLengthA(MovieName) + 1);

	return PalVideoPlay(FileName, Unk);
}

typedef INT(CDECL* PalErrorLogPrintfProc)(LPCSTR, INT);
PalErrorLogPrintfProc PalErrorLogPrintf = NULL;
INT CDECL HookPalErrorLogPrintf(LPCSTR Info, INT Unk)
{
	PrintConsoleA(Info);
	return 1;
}

INT CDECL HookDebugInfoOutput(LPCSTR Info, INT Result)
{
	PrintConsoleA("Debug Info :  Result = [0x%08x] %s", Result, Info);
	return Result;
}

BOOL
WINAPI
SetWindowPosForWindowMode(
_In_ HWND hWnd,
_In_opt_ HWND hWndInsertAfter,
_In_ int X,
_In_ int Y,
_In_ int cx,
_In_ int cy,
_In_ UINT uFlags)
{
	SenaHook* GlobalData;
	INT32     Height;
	ULONG     Style;

	GlobalData = SenaHook::GetSenaHook();
	Height     = GetSystemMetrics(SM_CYMENUSIZE);
	Style      = GetWindowLongW(GlobalData->hWnd, GWL_STYLE);
	SET_FLAG(Style, WS_SYSMENU);
	SetWindowLongW(GlobalData->hWnd, GWL_STYLE, Style);
	return SetWindowPos(
		GlobalData->hWnd,
		HWND_NOTOPMOST,
		0, 0,
		GlobalData->OriWidth,
		GlobalData->OriHeight + Height,
		42);
}


ULONG_PTR SetWindowPosForWindowMode_End = 0xCCB4F;

ASM Void DetourCallSetWindowPos()
{
	INLINE_ASM
	{
		call SetWindowPosForWindowMode;
		jmp  SetWindowPosForWindowMode_End;
	}
}



BOOL
WINAPI
SetWindowPos_WindowMode(
_In_ HWND hWnd,
_In_opt_ HWND hWndInsertAfter,
_In_ int X,
_In_ int Y,
_In_ int cx,
_In_ int cy,
_In_ UINT uFlags)
{
	SenaHook* GlobalData;

	GlobalData = SenaHook::GetSenaHook();
	
	SetMenu(GlobalData->hWnd, GlobalData->hMenuMain);

	return SetWindowPos(
		hWnd,
		hWndInsertAfter,
		X,
		Y,
		cx,
		cy,
		uFlags);
}

BOOL
WINAPI
SetWindowPos_FullMode(
_In_ HWND hWnd,
_In_opt_ HWND hWndInsertAfter,
_In_ int X,
_In_ int Y,
_In_ int cx,
_In_ int cy,
_In_ UINT uFlags)
{
	SenaHook* GlobalData;

	GlobalData = SenaHook::GetSenaHook();

	SetMenu(hWnd, NULL);

	return SetWindowPos(
		hWnd,
		hWndInsertAfter,
		X,
		Y,
		cx,
		cy,
		uFlags);
}


ULONG_PTR SetWindowPos_WindowMode_End = 0xA8C15;
ULONG_PTR DetourCallSetWindowPos_Full_End = 0xA8BBC;

ASM Void DetourCallSetWindowPos_Window()
{
	INLINE_ASM
	{
		call SetWindowPos_WindowMode;
		jmp  SetWindowPos_WindowMode_End;
	}
}

ASM Void DetourCallSetWindowPos_Full()
{
	INLINE_ASM
	{
		call SetWindowPos_FullMode;
		jmp  DetourCallSetWindowPos_Full_End;
	}
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
			static BYTE SpacePatch1[] = { 0x81, 0xBD, 0xCC, 0xFE, 0xFF, 0xFF, 0xA1, 0xA1 };
			static BYTE SpacePatch2[] = { 0x81, 0xBD, 0xC0, 0xFE, 0xFF, 0xFF, 0xA1, 0xA1 };
			static CHAR MovieName[]   = { '.',  'm',  'k',  'v' };
			
			CODE_PATCH_DATA c[] =
			{
				{ PtrAdd((PBYTE)hModule, 0xB5F7A), SpacePatch1, sizeof(SpacePatch1) },
				{ PtrAdd((PBYTE)hModule, 0xB687F), SpacePatch2, sizeof(SpacePatch2) }

				////100EFC00 @ PalPlayVideo call CmpExtName
				//{ PtrAdd((PBYTE)hModule, 0xEFC00), MovieName, sizeof(MovieName) }
			};

			PalVideoPlay      = (PalVideoPlayProc)     Nt_GetProcAddress(hModule, "PalVideoPlay");
			PalErrorLogPrintf = (PalErrorLogPrintfProc)Nt_GetProcAddress(hModule, "PalErrorLogPrintf");
		
			IAT_PATCH_DATA f[] =
			{
				{ hModule,                      MessageBoxA,      HookMessageBoxA,         "USER32.DLL" },
				{ hModule,                      SetWindowTextA,   HookSetWindowTextA,      "USER32.DLL" },
				{ hModule,                      CreateFontA,      HookCreateFontA,         "GDI32.DLL"  },
				{ hModule,                      RegisterClassExA, HookRegisterClassExA,    "USER32.DLL" },
				{ hModule,                      CreateWindowExA,  HookCreateWindowExA_Dll, "USER32.DLL" },
				{ hModule,                      DefWindowProcA,   HookDefWindowProcA,      "USER32.DLL" },
				{ hModule,                      DispatchMessageA, HookDispatchMessageA,    "USER32.DLL" },
				{ hModule,                      PeekMessageA,     HookPeekMessageA,        "USER32.DLL" },
				{ hModule,                      PostMessageA,     HookPostMessageA,        "USER32.DLL" },
				{ hModule,                      SetWindowPos,     HookSetWindowPos,        "USER32.DLL" }
			};
			
			CodePatchMemory(c, countof(c));
			HookPalFontSetType(hModule);

			IATPatchMemory(f, countof(f));

			
			pfPalLoadSprite = (PBYTE)hModule + 0xABAE0;
			Target = pfPalLoadSprite;

			INLINE_PATCH_DATA p[] =
			{
				{ Target,                    PalLoadSprEx,          &pfPalLoadSprite                                      },
				{ PalVideoPlay,              HookPalVideoPlay,      (PVOID*)&PalVideoPlay                                 },
				{ PalErrorLogPrintf,         HookPalErrorLogPrintf, (PVOID*)&PalErrorLogPrintf                            },
				{ (PBYTE)hModule + 0xA2BF0,  HookDebugInfoOutput,   NULL                                                  },
				{ (PBYTE)hModule + 0xADCA0,  KoikakeWindowProc,     (PVOID*)&(SenaHook::GetSenaHook()->OldMainWindowProc) },
				{ (PBYTE)hModule + 0xCCB49,  DetourCallSetWindowPos,        NULL },
				{ (PBYTE)hModule + 0xA8C0F,  DetourCallSetWindowPos_Window, NULL }
				//{ (PBYTE)hModule + 0xA8BB6,  DetourCallSetWindowPos_Full,   NULL }
			};

			SetWindowPosForWindowMode_End   += (ULONG_PTR)hModule;
			SetWindowPos_WindowMode_End     += (ULONG_PTR)hModule;
			DetourCallSetWindowPos_Full_End += (ULONG_PTR)hModule;

			Status = InlinePatchMemory(p, countof(p));
			if (!IsStatusSuccess(Status))
			{
				MessageBoxW(NULL, L"Failed to launch graphics system", L"Sofpal", MB_OK | MB_ICONERROR);
			}

			//24 - 1
			PatchAddr = (PBYTE)hModule + 0xA9E70;
			g_FilterDrawRet1 = PatchAddr + 5;
			g_FilterDrawCall1 = GetCALLTarget(PatchAddr);
			ReplaceJump(PatchAddr, (PBYTE)&UpdateGraphFilter1);

			//32
			PatchAddr = (PBYTE)hModule + 0xA9EC7;
			g_FilterDrawRet2 = PatchAddr + 5;
			g_FilterDrawCall2 = GetCALLTarget(PatchAddr);
			ReplaceJump(PatchAddr, (PBYTE)&UpdateGraphFilter2);

			PatchAddr = (PBYTE)hModule + 0xA4E74;
			g_FilterDrawRet3 = PatchAddr + 5;
			g_FilterDrawCall3 = GetCALLTarget(PatchAddr);
			//ReplaceJump(PatchAddr, (PBYTE)&UpdateGraphFilter3);

			//24 - 2
			PatchAddr = (PBYTE)hModule + 0xA9E8B;
			g_FilterDrawRet4 = PatchAddr + 5;
			g_FilterDrawCall4 = GetCALLTarget(PatchAddr);
			ReplaceJump(PatchAddr, (PBYTE)&UpdateGraphFilter4);

			SenaHook::GetSenaHook()->DllPatchFlag = TRUE;
		}

	}
	return hModule;
}


HFONT WINAPI HookCreateFontIndirectA(LOGFONTA *lplf)
{
	LOGFONTW Info;

	SenaHook*    GlobalData;

	GlobalData = SenaHook::GetSenaHook(); 
	GlobalData->XmoeZeroMemory(&Info, sizeof(Info));

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
	lstrcpyW(Info.lfFaceName, L"黑体");

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
	ULONG     Size, DecIndex;
	PBYTE     Data;
	SenaHook* GlobalData;

	GlobalData = SenaHook::GetSenaHook();

	Result = GlobalData->OldLoadText(Args1, Args2, Args3, Args4);

	if ((Args3 & 0x10000000) || Args4 == 0xFFFFFFF)
	{
		return Result;
	}

	ULONG Index = *(ULONG*)Result;

	if (GlobalData->XmoeCompiler->QueryText(Index, NULL, &Size, NULL))
	{
		pStringStart = Result + sizeof(ULONG);
		GlobalData->XmoeZeroMemory(GlobalData->PrivateMemory, Size);
		GlobalData->XmoeCompiler->QueryText(Index, GlobalData->PrivateMemory, &Size, &DecIndex);
		GlobalData->XmoeCompiler->DoPostDec(DecIndex, GlobalData->PrivateMemory, Size, pStringStart, Index);
	}

	return Result;
}


BOOL CDECL HookCheckFontProc(PBYTE Code)
{
	return  (Code[0] >= 0x81 && Code[0] <= 0xFE)
		&& ((Code[1] >= 0x40 && Code[1] <= 0x7E)
		|| (Code[1] >= 0x80 && Code[1] <= 0xFE));
}

ATOM WINAPI HookRegisterClassA(WNDCLASSA *lpWndClass)
{
	WNDCLASSW  ClassInfo;
	LPWSTR     MenuName, ClassName;

	SenaHook*    GlobalData;

	GlobalData = SenaHook::GetSenaHook();

	MenuName  = (LPWSTR)AllocStack(MAX_PATH * 2);
	ClassName = (LPWSTR)AllocStack(MAX_PATH * 2);

	GlobalData->XmoeZeroMemory(MenuName, MAX_PATH * 2);
	GlobalData->XmoeZeroMemory(ClassName, MAX_PATH * 2);

	MultiByteToWideChar(932, 0, lpWndClass->lpszMenuName,  StrLengthA(lpWndClass->lpszMenuName),  MenuName,  MAX_PATH);
	MultiByteToWideChar(932, 0, lpWndClass->lpszClassName, StrLengthA(lpWndClass->lpszClassName), ClassName, MAX_PATH);

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
	
	if (ClassInfo.hInstance == NULL || ClassInfo.hInstance == Nt_GetExeModuleHandle())
		return RegisterClassW(&ClassInfo);
	else
		return SenaHook::GetSenaHook()->OldRegisterClassA(lpWndClass);
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
	if (StrCompareA(lpClassName, "Koikake_chs") == 0)
	{
		SenaHook::GetSenaHook()->hWnd =
			CreateWindowExW(
			dwExStyle,
			L"Koikake_chs",
			L"[X'moe第三制药厂]想要传达给你的爱恋Ver2.0最终版(请勿直播本补丁内容)",
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


CHAR NewInfoName[MAX_PATH];
CHAR NewCompanyName[MAX_PATH];

PVOID PatchQueryNameStart = (PVOID)0x6F28;
PVOID PatchQueryNameEnd = (PVOID)0x6F2E;
PVOID PatchMemOffset = (PVOID)0x7AFF0;
PVOID NewInfoNameOffset = NULL;
PVOID NewCompanyNameOffset = NULL;

//@sub_406AD0

//CompanyName unk_4521F0
PVOID CompanyNameMemOffset = (PVOID)0x7ADE8;
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
		mov edx, dword ptr[ebp - 0x72C];
		jmp PatchQueryNameEnd;
	}
}


Void Executecpuid(DWORD veax, DWORD& ieax, DWORD& iebx, DWORD& iecx, DWORD& iedx)
{ 
	DWORD deax;
	DWORD debx;
	DWORD decx;
	DWORD dedx;
	__asm 
	{
		mov eax, veax; 
			cpuid; 
			mov deax, eax; 
			mov debx, ebx
			mov decx, ecx
			mov dedx, edx
	}
	ieax = deax;
	iebx = debx;
	iecx = decx;
	iedx = dedx;
}

#include <string>

void GetDisplayCardInfo(DWORD &dwNum, std::wstring chCardName[])
{
	HKEY keyServ;
	HKEY keyEnum;
	HKEY key;
	HKEY key2;
	LONG lResult;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services"), 0, KEY_READ, &keyServ);
	if (ERROR_SUCCESS != lResult)
		return;


	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Enum"), 0, KEY_READ, &keyEnum);
	if (ERROR_SUCCESS != lResult)
		return;

	int i = 0, count = 0;
	DWORD size = 0, type = 0;
	for (;; ++i)
	{
		size = 512;
		TCHAR name[512] = { 0 };

		lResult = RegEnumKeyEx(keyServ, i, name, &size, NULL, NULL, NULL, NULL);

		if (lResult == ERROR_NO_MORE_ITEMS)
			break;

		lResult = RegOpenKeyEx(keyServ, name, 0, KEY_READ, &key);
		if (lResult != ERROR_SUCCESS)
		{
			RegCloseKey(keyServ);
			return;
		}


		size = 512;
		lResult = RegQueryValueEx(key, TEXT("Group"), 0, &type, (LPBYTE)name, &size);
		if (lResult == ERROR_FILE_NOT_FOUND)
		{
			RegCloseKey(key);
			continue;
		};

		if (lstrcmp(TEXT("Video"), name) != 0)
		{
			RegCloseKey(key);
			continue;     //返回for循环  
		};

		//如果程序继续往下执行的话说明已经查到了有关显卡的信息，所以在下面的代码执行完之后要break第一个for循环，函数返回  
		lResult = RegOpenKeyEx(key, TEXT("Enum"), 0, KEY_READ, &key2);
		RegCloseKey(key);
		key = key2;
		size = sizeof(count);
		lResult = RegQueryValueEx(key, TEXT("Count"), 0, &type, (LPBYTE)&count, &size);//查询Count字段（显卡数目）  

		dwNum = count;//保存显卡数目  
		for (int j = 0; j <count; ++j)
		{
			TCHAR sz[512] = { 0 };
			TCHAR name[64] = { 0 };
			wsprintf(name, TEXT("%d"), j);
			size = sizeof(sz);
			lResult = RegQueryValueEx(key, name, 0, &type, (LPBYTE)sz, &size);

			lResult = RegOpenKeyEx(keyEnum, sz, 0, KEY_READ, &key2);
			if (ERROR_SUCCESS)
			{
				RegCloseKey(keyEnum);
				return;
			}


			size = sizeof(sz);
			lResult = RegQueryValueEx(key2, TEXT("FriendlyName"), 0, &type, (LPBYTE)sz, &size);
			if (lResult == ERROR_FILE_NOT_FOUND)
			{
				size = sizeof(sz);
				lResult = RegQueryValueEx(key2, TEXT("DeviceDesc"), 0, &type, (LPBYTE)sz, &size);
				chCardName[j] = sz;
			};
			RegCloseKey(key2);
			key2 = NULL;
		};
		RegCloseKey(key);
		key = NULL;
		break;
	}
}


BOOL NTAPI InitGPU(INT32* Index)
{
	DISPLAY_DEVICEW dd;
	ULONG           i;

	static WCHAR ooxx[] = L"NVIDIA";

	RtlZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);
	i = 0;

	while (EnumDisplayDevicesW(NULL, i, &dd, 0))
	{
		if (StrNICompareW(dd.DeviceString, ooxx, StrLengthW(ooxx)))
		{
			return TRUE;
		}
		i++;
	}
	return FALSE;
}

#pragma comment(lib, "cuda.lib")
#pragma comment(lib, "cudart_static.lib")
#pragma comment(lib, "nvcuvid.lib")

NTSTATUS NTAPI SenaHook::Init()
{
	NTSTATUS         Status;
	LPVOID           ModuleOfHost, UtilsModule;
	BOOL             CompilerResult;
	ULONG            TextSize;
	PBYTE            TextBuffer;
	DWORD            Eax, Ebx, Ecx, Edx;
	INT32            CudaIndex;
	bool             Flag;

	ModuleOfHost = GetModuleHandleW(NULL);

	PrivateMemory = (PBYTE)RtlAllocateHeap(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024 * 1024 * 10);
	if (!PrivateMemory)
	{
		MessageBoxW(NULL, L"内存不足", L"X'moe CoreLib(错误)", MB_OK | MB_ICONERROR);
		Ps::ExitProcess(0);
	}
	
	LOOP_ONCE
	{
		CudaIndex = -1;
		Flag = InitGPU(&CudaIndex);
		if (Flag)
		{
			SET_FLAG(OptimizationFlag, SENA_HAS_NVIDIA);
		}

		Executecpuid(0, Eax, Ebx, Ecx, Edx);

		if (Ebx == TAG4('Genu') && Edx == TAG4('ineI') && Ecx == TAG4('ntel'))
		{
			Flag = TRUE;
		}

		Executecpuid(1, Eax, Ebx, Ecx, Edx);
		Flag = (Edx & (1 << 26)) && Flag;

		if (Flag)
			SET_FLAG(OptimizationFlag, SENA_HAS_INTEL);

		if (OptimizationMode == OP_COMMON || !IN_RANGE(OP_COMMON, OptimizationMode, OP_INTEL))
		{
			OptimizationMode = OP_COMMON;

			if (FLAG_ON(OptimizationFlag, SENA_HAS_NVIDIA))
				OptimizationMode = OP_NV;

			switch (Flag)
			{
			case TRUE:
				if (OptimizationMode != OP_NV)
					OptimizationMode = OP_INTEL;

				XmoeCopyMemory = CopyMemorySSE;
				XmoeZeroMemory = SSE2ZeroMemory;
				break;

			default:
				XmoeCopyMemory = my_memcpy_inline;
				XmoeZeroMemory = X86ZeroMemory;
				break;
			}
		}

		if (OptimizationMode != OP_COMMON)
			WriteConfig(OptimizationFlag, UseTraditionalChinese);
	}

#if 0
	Flag = InitCUDA();
	if (Flag)
		SET_FLAG(OptimizationFlag, SENA_HAS_NVIDIA);
#endif

	XmoeZeroMemory(NewInfoName, sizeof(NewInfoName));
	StrCopyA(NewInfoName, "Koikake_chs");
	NewInfoNameOffset = &NewInfoName[0];

	XmoeZeroMemory(NewCompanyName, sizeof(NewCompanyName));
	StrCopyA(NewCompanyName, "Us_track");
	NewCompanyNameOffset = &NewCompanyName[0];

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

	Nt_LoadLibrary(L"USER32.DLL");
	Nt_LoadLibrary(L"ole32.dll");

	//PalVideoPlay = (PalVideoPlayProc)Nt_GetProcAddress(Nt_GetModuleHandle(L"PAL.DLL"), "PalVideoPlay");

	INLINE_PATCH_DATA p[] =
	{
		{ GetACP,                    HookGetACP,              NULL },
		{ GetOEMCP,                  HookGetOEMCP,            NULL },
		{ CreateFontIndirectA,       HookCreateFontIndirectA, NULL },
		{ LoadLibraryExA,            HookLoadLibraryExA,      (PVOID*)&OldLoadLibraryExA },
		{ GetGlyphOutlineA,          HookGetGlyphOutlineA,    NULL },
		{ DYNAMIC_ADDRESS(0x436EA0), HookLoadText,            (PVOID*)&OldLoadText },
		{ DYNAMIC_ADDRESS(0x42FC90), HookCheckFontProc,       NULL },
		{ PatchQueryNameStart,       PatchQueryName,          NULL },
		//{ RegisterClassA,            HookRegisterClassA,      (PVOID*)&OldRegisterClassA },
	};

	IAT_PATCH_DATA f[] =
	{
		{ GetModuleHandleW(NULL), MessageBoxA,     HookMessageBoxA,     "USER32.DLL" }
		//{ GetModuleHandleW(NULL), SetWindowTextA,  HookSetWindowTextA,  "USER32.DLL" },
		//{ GetModuleHandleW(NULL), CreateWindowExA, HookCreateWindowExA, "USER32.DLL" }
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
		TextSize = NULL;

		FileManager::GetFileManager()->QueryFile("Sena.bin", TextBuffer, TextSize);
		CompilerResult = XmoeCompiler->LoadTextFromBuffer(TextBuffer, TextSize);

		if (!CompilerResult)
			CompilerResult = XmoeCompiler->LoadText(NULL);


		if (!CompilerResult)
		{
			MessageBoxW(NULL, L"内部运行失败", L"X'moe CoreLib", MB_OK | MB_ICONERROR);
			Ps::ExitProcess(0);
		}
	}

	return Status;
}


NTSTATUS NTAPI SenaHook::UnInit()
{
	return STATUS_SUCCESS;
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

	return FileSystemInit ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

