#pragma once

#include <Windows.h>

#define True  TRUE
#define False FALSE


#define IMAGE_INVALID_ORDINAL   ((ULONG_PTR)(~0ull))

#define IMAGE_INVALID_RVA       ((ULONG_PTR)(~0ui64))
#define IMAGE_INVALID_VA        ((PVOID)(~0ui64))
#define IMAGE_INVALID_OFFSET    IMAGE_INVALID_RVA
#define IMAGE_MINIMUM_ADDRESS   (ULONG_PTR)0x10000


#define TYPE_OF decltype

#define FUNC_POINTER(__func) TYPE_OF(__func)*
#define API_POINTER(__func) TYPE_OF(&__func)

typedef float               Float, *PFloat, *LPFloat;
typedef double              Double, *PDouble, *LPDouble;

typedef char                Char, s8, Int8, *ps8, *PInt8, *PChar;
typedef const char         *PCChar;
typedef wchar_t             WChar, *PWChar;
typedef const wchar_t      *PCWChar;
typedef unsigned char       u8, UChar, UInt8, UInt24, Byte, *pu8, *PByte, *PUInt8, *PUChar;
typedef short               Short, s16, Int16, *ps16, *PInt16, *PShort, *LPShort;
typedef unsigned short      UShort, Word, u16, UInt16, *pu16, *PUInt16, *PWord, *LPWord, *PUShort, *LPUShort;
typedef long                Long, *PLong, *LPLong;
typedef long long           Long64, LongLong, *PLong64, *PLongLong;
typedef unsigned long       Dword, ULong, *PULong, *LPULong, *PDword, *LPDword;
typedef unsigned long long  ULong64, ULongLong, *PULong64, *PULongLong;
typedef void                Void, *PVoid, *LPVoid;
typedef const void         *LPCVoid, *PCVoid;

typedef int                 Bool, Int, s32, Int32, *PInt, *ps32, *PInt32;
typedef unsigned int        UInt, u32, UInt32, *PUInt, *pu32, *PUInt32;

typedef __int64             s64, Int64, *ps64, *PInt64;
typedef unsigned __int64    u64, UInt64, *pu64, *PUInt64;

typedef int(__cdecl *QSORT_COMPARE_ROUTINE)(const void *, const void *);

#ifndef STDCALL
#define STDCALL __stdcall
#endif

typedef struct
{
	/* 0x000 */ ULONG   Flags;
	/* 0x004 */ PSTR    FrameName;

} TEB_ACTIVE_FRAME_CONTEXT, *PTEB_ACTIVE_FRAME_CONTEXT;

typedef struct TEB_ACTIVE_FRAME
{
	/* 0x000 */ ULONG                       Context;  // Flags;
	/* 0x004 */ struct TEB_ACTIVE_FRAME    *Previous;
	ULONG_PTR                   Data;
	// /* 0x008 */ PTEB_ACTIVE_FRAME_CONTEXT   Context;

} TEB_ACTIVE_FRAME, *PTEB_ACTIVE_FRAME;

#if defined(MY_X64)
typedef __int64 Int_Ptr, *PInt_Ptr;
typedef unsigned __int64 UInt_Ptr, *PUInt_Ptr;

typedef __int64 Long_Ptr, *PLong_Ptr, LongPtr, *PLongPtr;
typedef unsigned __int64 ULong_Ptr, *PULong_Ptr, ULongPtr, *PULongPtr;
#else
typedef int __w64 Int_Ptr, *PInt_Ptr;
typedef unsigned int __w64 UInt_Ptr, *PUInt_Ptr;

typedef long __w64 Long_Ptr, *PLong_Ptr, LongPtr, *PLongPtr;
typedef unsigned long __w64 ULong_Ptr, *PULong_Ptr, ULongPtr, *PULongPtr;
#endif

typedef ULong_Ptr SizeT, *PSizeT;
typedef Long_Ptr  SSizeT, *PSSizeT;

typedef ULong_Ptr SizeT, *PSizeT;
typedef Long_Ptr  SSizeT, *PSSizeT;

#define MAX_SHORT  (Short) (0x7FFF)
#define MAX_USHORT (UShort)(0xFFFF)
#define MAX_INT    (Int)   (0x7FFFFFFF)
#define MAX_UINT   (UInt)  (0xFFFFFFFF)
#define MAX_INT64  (Int64) (0x7FFFFFFFFFFFFFFF)
#define MAX_UINT64 (UInt64)(0xFFFFFFFFFFFFFFFF)
#define MAX_NTPATH  0x220


#define CHAR_UPPER(ch) (IN_RANGE('a', (ch), 'z') ? ((ch) & (Byte)0xDF) : ch)
#define CHAR_LOWER(ch) (IN_RANGE('A', (ch), 'Z') ? ((ch) | (Byte)~0xDF) : ch)

#define _CHAR_UPPER4W(ch) (UInt64)((ch) & 0xFFDFFFDFFFDFFFDF)
#define CHAR_UPPER4W(ch) _CHAR_UPPER4W((UInt64)(ch))
#define CHAR_UPPER3W(ch) (UInt64)(CHAR_UPPER4W(ch) & 0x0000FFFFFFFFFFFF)
#define CHAR_UPPER2W(ch) (UInt64)(CHAR_UPPER4W(ch) & 0x00000000FFFFFFFF)
#define CHAR_UPPER1W(ch) (UInt64)(CHAR_UPPER4W(ch) & 0x000000000000FFFF)

#define _CHAR_UPPER4(ch) (UInt32)((ch) & 0xDFDFDFDF)
#define CHAR_UPPER4(ch) (UInt32)_CHAR_UPPER4((UInt32)(ch))
#define CHAR_UPPER3(ch) (UInt32)(CHAR_UPPER4(ch) & 0x00FFFFFF)
#define CHAR_UPPER2(ch) (UInt32)(CHAR_UPPER4(ch) & 0x0000FFFF)
#define CHAR_UPPER1(ch) (UInt32)(CHAR_UPPER4(ch) & 0x000000FF)
#define CHAR_UPPER8(ch) ((UInt64)(ch) & 0xDFDFDFDFDFDFDFDF)

#define _TAG2(s) ((((s) << 8) | ((s) >> 8)) & 0xFFFF)
#define TAG2(s) _TAG2((u16)(s))

#define _TAG3(s) ( \
                (((s) >> 16) & 0xFF)       | \
                (((s)        & 0xFF00))    | \
                (((s) << 16) & 0x00FF0000) \
                )
#define TAG3(s) _TAG3((u32)(s))

#define _TAG4(s) ( \
                (((s) >> 24) & 0xFF)       | \
                (((s) >> 8 ) & 0xFF00)     | \
                (((s) << 24) & 0xFF000000) | \
                (((s) << 8 ) & 0x00FF0000) \
                )
#define TAG4(s) _TAG4((u32)(s))

#define TAG8(left, right) (((UInt64)TAG4(right) << 32) | TAG4(left))

#define _TAG2W(x) (((x) & 0xFF) << 16 | ((x) & 0xFF00) >> 8)
#define TAG2W(x) (UInt32)_TAG2W((UInt32)(x))

#define _TAG3W(x) (TAG4W(x) >> 16)
#define TAG3W(x) (UInt64)_TAG3W((UInt64)(x))

#define _TAG4W(x) (((UInt64)TAG2W((x) & 0xFFFF) << 32) | ((UInt64)TAG2W((x) >> 16)))
#define TAG4W(x) (UInt64)_TAG4W((UInt64)(x))

#pragma warning(disable:4310)
#define SWAP2(v) (u16)(((u32)(v) << 8) | ((u16)(v) >> 8))
#define SWAPCHAR(v) ((u32)SWAP2(v))

#define LoByte(v)  (u8) ((v & 0xFF))
#define HiByte(v)  (u8) (((v) >> 8) & 0xFF)
#define LoWord(v)  (u16)((v) & 0xFFFF)
#define HiWord(v)  (u16)(((v) >> 16) & 0xFFFF)
#define LoDword(v) (u32)((v))
#define HiDword(v) (u32)(((v) >> 32))

#define MakeLong(l, h)   (long)((s32)(l) | ((s32)(h) << 16))
#define MakeLong64(l, h) (s64)((s64)(l) | (s64)(h) << 32)

#define MakeDword(l, h) (u32)((u32)(l) | ((u32)(h) << 16))
#define MakeQword(l, h) (u64)((u64)(l) | (u64)(h) << 32)

#define MAKEINTRESA(i) ((PChar)(Word)(i))
#define MAKEINTRESW(i) ((PWChar)(Word)(i))

#define STRTOULONG(x) (ULong_Ptr)(x)

#define ML_IP_ADDRESS(a1, a2, a3, a4) ((a1) | ((a2) << 8) | ((a3) << 16) | ((a4) << 24))
#define ML_PORT(_port) SWAP2(_port)

#define FOR_EACH(_it, _base, _n) for (auto _Count = ( ((_it) = (_base)), (_n)); _Count != 0; ++(_it), --_Count)
#define FOR_EACH_REVERSE(_it, _base, _n) for (auto _Count = ( ((_it) = (_base) + (_n) - 1), (_n)); _Count != 0; --(_it), --_Count)
#define FOR_EACH_ARRAY(_it, _arr) FOR_EACH(_it, _arr, countof(_arr))
#define FOR_EACH_S(_it, _base, _n, _size) for (auto _Count = ( ((_it) = (_base)), (_n)); _Count != 0; ((_it) = PtrAdd(_it, _size)), --_Count)
#define FOR_EACH_X(_it, _base, _n) for (auto _Count = ( ((_it) = (_base)), (_n); _Count != 0; ++(_it), --(_n), --_Count)

#define FOR_EACH_FORWARD(_it, _n) { (_it) += (_n); (_Count) += (_n); }
#define FOR_EACH_BACKWARD(_it, _n) { (_it) -= (_n); (_Count) -= (_n); }

#define LOOP_ALWAYS for (;;)
#define LOOP_FOREVER LOOP_ALWAYS
#define LOOP_ONCE   for (Bool __condition_ = True; __condition_; __condition_ = False)

enum // ECodePage
{
	CP_SHIFTJIS = 932,
	CP_GBK = 936,
	CP_GB2312 = CP_GBK,
	CP_BIG5 = 950,
	CP_UTF16_LE = 1200,
	CP_UTF16_BE = 1201,
};

#define BOM_UTF8        (UInt32)(0xBFBBEF)
#define BOM_UTF16_LE    (UInt16)(0xFEFF)
#define BOM_UTF16_BE    (UInt16)(0xFFFE)

#define __MAKE_WSTRING(str) L##str
#define MAKE_WSTRING(str) __MAKE_WSTRING(str)

#define ASM_UNIQUE() INLINE_ASM mov eax, __LINE__

#define ASM_DUMMY(Bytes) ASM_DUMMY_##Bytes

#define ASM_DUMMY_1 INLINE_ASM nop
#define ASM_DUMMY_2 INLINE_ASM mov eax, eax

// lea eax, [eax+0];
#define ASM_DUMMY_3 INLINE_ASM __emit 0x8D INLINE_ASM __emit 0x40 INLINE_ASM __emit 0x00

// // lea esi, [esi]
#define ASM_DUMMY_4 INLINE_ASM __emit 0x8D \
                    INLINE_ASM __emit 0x74 \
                    INLINE_ASM __emit 0x26 \
                    INLINE_ASM __emit 0x00

#define ASM_DUMMY_5 INLINE_ASM mov eax, 1
#define ASM_DUMMY_6 INLINE_ASM __emit 0x8D INLINE_ASM __emit 0x80 INLINE_ASM __emit 0x00 INLINE_ASM __emit 0x00 INLINE_ASM __emit 0x00 INLINE_ASM __emit 0x00

#define ASM_DUMMY_7 INLINE_ASM __emit 0x8D \
                    INLINE_ASM __emit 0xB4 \
                    INLINE_ASM __emit 0x26 \
                    INLINE_ASM __emit 0x00 \
                    INLINE_ASM __emit 0x00 \
                    INLINE_ASM __emit 0x00 \
                    INLINE_ASM __emit 0x00

#define ASM_DUMMY_AUTO() INLINE_ASM mov eax, 1 INLINE_ASM mov ecx, 1 INLINE_ASM mov edx, 1 ASM_UNIQUE() INLINE_ASM ret

#if !defined(_M_IA64)
#define MEMORY_PAGE_SIZE (4 * 1024)
#else
#define MEMORY_PAGE_SIZE (8 * 1024)
#endif

#define NO_BREAK

#define NT6_LIB(lib) "D:/Dev/Windows Kits/8.0/Lib/win8/um/x86/" #lib

#define LOOP_ALWAYS for (;;)
#define LOOP_FOREVER LOOP_ALWAYS
#define LOOP_ONCE   for (Bool __condition_ = True; __condition_; __condition_ = False)

#if !defined(BREAK_IF)
#define BREAK_IF(c) if (c) break;
#endif /* BREAK_IF */

#if !defined(CONTINUE_IF)
#define CONTINUE_IF(c) if (c) continue;
#endif /* CONTINUE_IF */

#if !defined(RETURN_IF)
#define RETURN_IF(c, r) if (c) return r
#endif /* RETURN_IF */

#if !defined(countof)
#define countof(x) (sizeof((x)) / sizeof(*(x)))
#endif /* countof */

#if !defined(CONST_STRLEN)
#define CONST_STRLEN(str) (countof(str) - 1)
#define CONST_STRSIZE(str) (CONST_STRLEN(str) * sizeof(str[0]))
#endif // CONST_STRLEN

#if !defined(bitsof)
#define bitsof(x) (sizeof(x) * 8)
#endif /* bitsof */

#define FIELD_BASE(address, type, field) (type *)((ULONG_PTR)address - (ULONG_PTR)&((type *)0)->field)
#define FIELD_TYPE(_Type, _Field)  TYPE_OF(((_Type*)0)->_Field)

#ifndef FIELD_SIZE
#define FIELD_SIZE(type, field) (sizeof(((type *)0)->field))
#endif // FIELD_SIZE

#define SET_FLAG(_V, _F)    ((_V) |= (_F))
#define CLEAR_FLAG(_V, _F)  ((_V) &= ~(_F))
#define FLAG_ON(_V, _F)     (!!((_V) & (_F)))
#define FLAG_OFF(_V, _F)     (!FLAG_ON(_V, _F))

#if !defined(TEST_BIT)
#define TEST_BIT(value, bit) ((value) & (1 << bit))
#endif /* TEST_BIT */

#if !defined(TEST_BITS)
#define TEST_BITS(value, bits) (Bool)(!!((value) & (bits)))
#endif /* TEST_BITS */

#if !defined(ROUND_DOWN)
#define ROUND_DOWN(Value, Multiple) ((Value) / (Multiple) * (Multiple))
#endif /* ROUND_DOWN */

#if !defined(ROUND_UP)
#define ROUND_UP(Value, Multiple) (ROUND_DOWN((Value) + (Multiple) - 1, (Multiple)))
#endif /* ROUND_UP */

#if !defined(IN_RANGE)
#define IN_RANGE(low, value, high) (((low) <= (value)) && (value) <= (high))
#define IN_RANGEEX(low, value, high) (((low) <= (value)) && (value) < (high))
#endif

#if !defined(MEMORY_PAGE_ADDRESS)
#define MEMORY_PAGE_ADDRESS(Address) (ROUND_DOWN((ULongPtr)(Address), MEMORY_PAGE_SIZE))
#endif /* MEMORY_PAGE_ADDRESS */

#if defined(DEFAULT_VALUE)
#undef DEFAULT_VALUE
#endif // DEFAULT_VALUE

#ifdef __cplusplus
#define DEFAULT_VALUE(type, var, value) type var = value
#define DEF_VAL(var, value)             var = value
#else
#define DEFAULT_VALUE(type, var, value) type var
#define DEF_VAL(var, value)             var
#endif //CPP_DEFINED

#ifndef FASTCALL
#define FASTCALL __fastcall
#endif

#ifndef ASM
#define ASM __declspec(naked)
#endif /* ASM */

#ifndef NAKED
#define NAKED __declspec(naked)
#endif /* ASM */

#ifndef INLINE_ASM
#define INLINE_ASM __asm
#endif

#ifndef NATIVE_API
#define NATIVE_API  extern "C" __declspec(dllimport)
#endif

#if !defined(NtCurrentProcess)
#define NtCurrentProcess() (HANDLE)-1
#define NtCurrentProcess64() (HANDLE64)-1
#endif 

#if !defined(NtCurrentThread)
#define NtCurrentThread() (HANDLE)-2
#define NtCurrentThread64() (HANDLE64)-2
#endif 


static const HANDLE CurrentProcess = NtCurrentProcess();
static const HANDLE CurrentThread = NtCurrentThread();


typedef struct _CLIENT_ID
{
	HANDLE  UniqueProcess;
	HANDLE  UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _CLIENT_ID32
{
	ULONG32  UniqueProcess;
	ULONG32  UniqueThread;

} CLIENT_ID32, *PCLIENT_ID32;


typedef struct _FILE_BASIC_INFORMATION
{
	LARGE_INTEGER   CreationTime;
	LARGE_INTEGER   LastAccessTime;
	LARGE_INTEGER   LastWriteTime;
	LARGE_INTEGER   ChangeTime;
	ULONG           FileAttributes;
	ULONG           Dummy;

} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _SECTION_BASIC_INFORMATION
{
	PVOID           BaseAddress;
	ULONG           Attributes;
	LARGE_INTEGER   Size;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

typedef struct _SECTION_IMAGE_INFORMATION
{
	PVOID   TransferAddress;                    // 0x00
	ULONG   ZeroBits;                           // 0x04
	SIZE_T  MaximumStackSize;                   // 0x08
	SIZE_T  CommittedStackSize;                 // 0x0C
	ULONG   SubSystemType;                      // 0x10

	union
	{
		struct
		{
			USHORT SubSystemMinorVersion;
			USHORT SubSystemMajorVersion;
		};
		ULONG SubSystemVersion;                 // 0x14
	};

	ULONG   GpValue;                            // 0x18
	USHORT  ImageCharacteristics;               // 0x1C
	USHORT  DllCharacteristics;                 // 0x1E
	USHORT  Machine;                            // 0x20
	UCHAR   ImageContainsCode;                  // 0x22
	union
	{
		UCHAR ImageFlags;                       // 0x23
		struct
		{
			UCHAR ComPlusNativeReady : 1;
			UCHAR ComPlusILOnly : 1;
			UCHAR ImageDynamicallyRelocated : 1;
			UCHAR ImageMappedFlat : 1;
		};
	} ImageFlags;

	ULONG   LoaderFlags;                        // 0x24
	ULONG   ImageFileSize;                      // 0x28
	ULONG   CheckSum;                           // 0x2C

} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;


enum
{
	IMAGE_VALID_EXPORT_ADDRESS_TABLE = 0x00000001,
	IMAGE_VALID_IMPORT_ADDRESS_TABLE = 0x00000002,
	IMAGE_VALID_RESOURCE = 0x00000004,
	IMAGE_VALID_RELOC = 0x00000008,
};


typedef enum
{
	SectionBasicInformation,
	SectionImageInformation,
	SectionRelocationInformation,   // ret = now_base - desire_base

} SECTION_INFORMATION_CLASS;


typedef struct
{
	union
	{
		NTSTATUS    Status;
		PVOID       Pointer;
	};
	ULONG_PTR Information;

} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

/************************************************************************/
/* strings                                                              */
/************************************************************************/

typedef struct _STRING
{
	USHORT  Length;
	USHORT  MaximumLength;
	PCHAR   Buffer;
} STRING;

typedef STRING *PSTRING;
typedef STRING ANSI_STRING;
typedef PSTRING PANSI_STRING;

typedef struct _UNICODE_STRING {
	USHORT  Length;
	USHORT  MaximumLength;
	PWSTR   Buffer;
} UNICODE_STRING;

typedef struct _LARGE_UNICODE_STRING
{
	ULONG Length;
	ULONG MaximumLength : 31;
	ULONG Ansi : 1;

	union
	{
		PWSTR   UnicodeBuffer;
		PSTR    AnsiBuffer;
		ULONG64 Buffer;
	};

} LARGE_UNICODE_STRING, *PLARGE_UNICODE_STRING;

#pragma pack(push, 8)

typedef struct
{
	USHORT Length;
	USHORT MaximumLength;
	union
	{
		PWSTR  Buffer;
		ULONG64 Dummy;
	};

} ANSI_STRING3264, *PANSI_STRING3264;

typedef struct
{
	USHORT Length;
	USHORT MaximumLength;
	union
	{
		PWSTR  Buffer;
		ULONG64 Dummy;
	};

} UNICODE_STRING3264, *PUNICODE_STRING3264;

typedef UNICODE_STRING3264 UNICODE_STRING64;
typedef PUNICODE_STRING3264 PUNICODE_STRING64;

#pragma pack(pop)

typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

#define RTL_CONST_STRING(_str, _string) \
            (_str).Length = sizeof(_string) - sizeof((_str).Buffer[0]); \
            (_str).MaximumLength = sizeof(_string); \
            (_str).Buffer = (_string);

#define GetStructMemberOffset(type, member_begin, member_end) \
    (PByte)&(*(type*)0).member_end - (PByte)&(*(type*)0).member_begin

typedef struct _RTLP_CURDIR_REF *PRTLP_CURDIR_REF;

typedef struct _RTL_RELATIVE_NAME_U
{
	UNICODE_STRING RelativeName;
	HANDLE ContainingDirectory;
	PRTLP_CURDIR_REF CurDirRef;
} RTL_RELATIVE_NAME_U, *PRTL_RELATIVE_NAME_U;

#define LDRP_STATIC_LINK                0x00000002
#define LDRP_IMAGE_DLL                  0x00000004
#define LDRP_LOAD_IN_PROGRESS           0x00001000
#define LDRP_UNLOAD_IN_PROGRESS         0x00002000
#define LDRP_ENTRY_PROCESSED            0x00004000
#define LDRP_ENTRY_INSERTED             0x00008000
#define LDRP_CURRENT_LOAD               0x00010000
#define LDRP_FAILED_BUILTIN_LOAD        0x00020000
#define LDRP_DONT_CALL_FOR_THREADS      0x00040000
#define LDRP_PROCESS_ATTACH_CALLED      0x00080000
#define LDRP_DEBUG_SYMBOLS_LOADED       0x00100000
#define LDRP_IMAGE_NOT_AT_BASE          0x00200000
#define LDRP_COR_IMAGE                  0x00400000
#define LDRP_COR_OWNS_UNMAP             0x00800000
#define LDRP_SYSTEM_MAPPED              0x01000000
#define LDRP_IMAGE_VERIFYING            0x02000000
#define LDRP_DRIVER_DEPENDENT_DLL       0x04000000
#define LDRP_ENTRY_NATIVE               0x08000000
#define LDRP_REDIRECTED                 0x10000000
#define LDRP_NON_PAGED_DEBUG_INFO       0x20000000
#define LDRP_MM_LOADED                  0x40000000
#define LDRP_COMPAT_DATABASE_PROCESSED  0x80000000

typedef struct _LDR_DATA_TABLE_ENTRY
{
	/* +0x000 */ LIST_ENTRY     InLoadOrderLinks;
	/* +0x008 */ LIST_ENTRY     InMemoryOrderLinks;
	/* +0x010 */ LIST_ENTRY     InInitializationOrderLinks;
	/* +0x018 */ PVOID          DllBase;
	/* +0x01c */ PVOID          EntryPoint;
	/* +0x020 */ ULONG          SizeOfImage;
	/* +0x024 */ UNICODE_STRING FullDllName;
	/* +0x02c */ UNICODE_STRING BaseDllName;
	/* +0x034 */ ULONG          Flags;
	/* +0x038 */ USHORT         LoadCount;
	/* +0x03a */ USHORT         TlsIndex;

	union
	{
		/* +0x03c */    LIST_ENTRY     HashLinks;
		struct
		{
			/* +0x03c */        PVOID          SectionPointer;
			/* +0x040 */        ULONG          CheckSum;
		};
	};

	union
	{
		/* +0x044 */    ULONG          TimeDateStamp;
		/* +0x044 */    PVOID          LoadedImports;
	};

	/* +0x048 */ PVOID          EntryPointActivationContext;
	/* +0x04c */ PVOID          PatchInformation;
	/* +0x050 */ LIST_ENTRY     ForwarderLinks;
	/* +0x058 */ LIST_ENTRY     ServiceTagLinks;
	/* +0x060 */ LIST_ENTRY     StaticLinks;
	/* +0x068 */ PVOID          ContextInformation;
	/* +0x06c */ ULONG          OriginalBase;
	/* +0x070 */ LARGE_INTEGER  LoadTime;

} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef const LDR_DATA_TABLE_ENTRY* PCLDR_DATA_TABLE_ENTRY;

typedef LDR_DATA_TABLE_ENTRY LDR_MODULE;
typedef PLDR_DATA_TABLE_ENTRY PLDR_MODULE;
typedef PCLDR_DATA_TABLE_ENTRY PCLDR_MODULE;

#ifdef __cplusplus
extern "C" {
#endif

#define MAXIMUM_LEADBYTES   12

	typedef struct _CPTABLEINFO {
		USHORT CodePage;                    // code page number
		USHORT MaximumCharacterSize;        // max length (bytes) of a char
		USHORT DefaultChar;                 // default character (MB)
		USHORT UniDefaultChar;              // default character (Unicode)
		USHORT TransDefaultChar;            // translation of default char (Unicode)
		USHORT TransUniDefaultChar;         // translation of Unic default char (MB)
		USHORT DBCSCodePage;                // Non 0 for DBCS code pages
		UCHAR  LeadByte[MAXIMUM_LEADBYTES]; // lead byte ranges
		PUSHORT MultiByteTable;             // pointer to MB translation table
		PVOID   WideCharTable;              // pointer to WC translation table
		PUSHORT DBCSRanges;                 // pointer to DBCS ranges
		PUSHORT DBCSOffsets;                // pointer to DBCS offsets
	} CPTABLEINFO, *PCPTABLEINFO;

	typedef struct _NLSTABLEINFO {
		CPTABLEINFO OemTableInfo;
		CPTABLEINFO AnsiTableInfo;
		PUSHORT UpperCaseTable;             // 844 format upcase table
		PUSHORT LowerCaseTable;             // 844 format lower case table
	} NLSTABLEINFO, *PNLSTABLEINFO;

#ifdef __cplusplus
}
#endif

typedef
NTSTATUS
(NTAPI
*PRTL_HEAP_COMMIT_ROUTINE)(
IN     PVOID    Base,
IN OUT PVOID   *CommitAddress,
IN OUT PSIZE_T  CommitSize
);

typedef struct _RTL_HEAP_PARAMETERS
{
	ULONG                       Length;
	SIZE_T                      SegmentReserve;
	SIZE_T                      SegmentCommit;
	SIZE_T                      DeCommitFreeBlockThreshold;
	SIZE_T                      DeCommitTotalFreeThreshold;
	SIZE_T                      MaximumAllocationSize;
	SIZE_T                      VirtualMemoryThreshold;
	SIZE_T                      InitialCommit;
	SIZE_T                      InitialReserve;
	PRTL_HEAP_COMMIT_ROUTINE    CommitRoutine;
	SIZE_T                      Reserved[2];

} RTL_HEAP_PARAMETERS, *PRTL_HEAP_PARAMETERS;

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status)  (((NTSTATUS)(Status)) >= 0)
#endif /* NT_SUCCESS */

#define CopyStruct(dest, src, size) \
	{ \
	memcpy(dest, src, size); \
	}

#if !defined(FAIL_RETURN)
#define FAIL_RETURN(Status) { NTSTATUS __Status__;  __Status__ = (Status); if (!NT_SUCCESS(__Status__)) return __Status__; }
#define FAIL_CONTINUE(Status) if (!NT_SUCCESS(Status)) continue
#define FAIL_BREAK(Status) if (!NT_SUCCESS(Status)) break
#define SUCCESS_RETURN(Status) { NTSTATUS __Status__;  __Status__ = (Status); if (NT_SUCCESS(__Status__)) return __Status__; }
#endif  // FAIL_RETURN

#ifndef MAX
#define MAX(a, b) a > b ? a : b
#endif

#ifndef MIN
#define MIN(a, b) a < b ? a : b
#endif

char _RTL_CONSTANT_STRING_type_check(const void *s);
#define _RTL_CONSTANT_STRING_remove_const_macro(s) (s)

#define RTL_CONSTANT_STRING(s) \
{ \
    sizeof( s ) - sizeof( (s)[0] ), \
    sizeof( s ) / sizeof(_RTL_CONSTANT_STRING_type_check(s)), \
    _RTL_CONSTANT_STRING_remove_const_macro(s) \
}

extern "C" IMAGE_DOS_HEADER __ImageBase;

#ifndef EXTC_IMPORT
#define EXTC_IMPORT extern "C" __declspec(dllimport)
#endif

#ifndef POINTER_SIZE
#define POINTER_SIZE sizeof(PVOID)
#endif

#ifdef _WIN64
#define INT_PTR_MAX     9223372036854775807i64
#define UINT_PTR_MAX    0xffffffffffffffffui64
#define LONG_PTR_MAX    9223372036854775807i64
#define ULONG_PTR_MAX   0xffffffffffffffffui64
#define DWORD_PTR_MAX   0xffffffffffffffffui64
#define PTRDIFF_T_MAX   9223372036854775807i64
#define SIZE_T_MAX      0xffffffffffffffffui64
#define SSIZE_T_MAX     9223372036854775807i64
#define _SIZE_T_MAX     0xffffffffffffffffui64
#else
#define INT_PTR_MAX     2147483647 
#define UINT_PTR_MAX    0xffffffff
#define LONG_PTR_MAX    2147483647L
#define ULONG_PTR_MAX   0xffffffffUL
#define DWORD_PTR_MAX   0xffffffffUL
#define PTRDIFF_T_MAX   2147483647
#define SIZE_T_MAX      0xffffffff
#define SSIZE_T_MAX     2147483647L
#define _SIZE_T_MAX     0xffffffffUL
#endif


	typedef struct
	{
		UNICODE_STRING  WindowsDirectory;
		UNICODE_STRING  SystemDirectory;
		UNICODE_STRING  BaseNamedObjects;
		ULONG           Unknown;
		USHORT          Unknown2;
		USHORT          ServicePackLength;
		USHORT          ServicePackMaximumLength;
		WCHAR           ServicePack[1];

	} *PSTATIC_SERVER_DATA;

	typedef struct
	{
		PVOID               Reserve;
		PSTATIC_SERVER_DATA StaticServerData;

	} *STATIC_SERVER_DATA_PTR;


	typedef struct
	{
		/* 0x000 */ USHORT      Flags;
		/* 0x002 */ USHORT      Length;
		/* 0x004 */ ULONG       TimeStamp;
		/* 0x008 */ ANSI_STRING DosPath;
	} RTL_DRIVE_LETTER_CURDIR;

	typedef struct
	{
		UNICODE_STRING  DosPath;
		HANDLE          Handle;
	} CURDIR;

	typedef struct
	{
		/* 0x000 */ ULONG                   MaximumLength;
		/* 0x004 */ ULONG                   Length;
		/* 0x008 */ ULONG                   Flags;
		/* 0x00c */ ULONG                   DebugFlags;
		/* 0x010 */ HANDLE                  ConsoleHandle;
		/* 0x014 */ ULONG                   ConsoleFlags;
		/* 0x018 */ HANDLE                  StandardInput;
		/* 0x01c */ HANDLE                  StandardOutput;
		/* 0x020 */ HANDLE                  StandardError;
		/* 0x024 */ CURDIR                  CurrentDirectory;
		/* 0x030 */ UNICODE_STRING          DllPath;
		/* 0x038 */ UNICODE_STRING          ImagePathName;
		/* 0x040 */ UNICODE_STRING          CommandLine;
		/* 0x048 */ PWCHAR                  Environment;
		/* 0x04c */ ULONG                   StartingX;
		/* 0x050 */ ULONG                   StartingY;
		/* 0x054 */ ULONG                   CountX;
		/* 0x058 */ ULONG                   CountY;
		/* 0x05c */ ULONG                   CountCharsX;
		/* 0x060 */ ULONG                   CountCharsY;
		/* 0x064 */ ULONG                   FillAttribute;
		/* 0x068 */ ULONG                   WindowFlags;
		/* 0x06c */ ULONG                   ShowWindowFlags;
		/* 0x070 */ UNICODE_STRING          WindowTitle;
		/* 0x078 */ UNICODE_STRING          DesktopInfo;
		/* 0x080 */ UNICODE_STRING          ShellInfo;
		/* 0x088 */ UNICODE_STRING          RuntimeData;
		/* 0x090 */ RTL_DRIVE_LETTER_CURDIR CurrentDirectores[32];
		/* 0x290 */ ULONG_PTR               EnvironmentSize;
		/* 0x294 */ ULONG_PTR               EnvironmentVersion;
	} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;


	typedef struct
	{
		ULONG       Length;                             // +0x00
		BOOL        Initialized;                        // +0x04
		PVOID       SsHandle;                           // +0x08
		LIST_ENTRY  InLoadOrderModuleList;              // +0x0c
		LIST_ENTRY  InMemoryOrderModuleList;            // +0x14
		LIST_ENTRY  InInitializationOrderModuleList;    // +0x1c
		PVOID       EntryInProgress;                    // +0x24
		ULONG       ShutdownInProgress;                 // +0x28
		ULONG_PTR   ShutdownThreadId;                   // +0x2C
	} PEB_LDR_DATA, *PPEB_LDR_DATA;                     // +0x30

	typedef struct PEB_BASE
	{
		/* 0x000 */ UCHAR                           InheritedAddressSpace;
		/* 0x001 */ UCHAR                           ReadImageFileExecOptions;
		/* 0x002 */ UCHAR                           BeingDebugged;
		/* 0x003 */ struct
		{
			UCHAR                       ImageUsesLargePages : 1;
			UCHAR                       IsProtectedProcess : 1;
			UCHAR                       IsLegacyProcess : 1;
			UCHAR                       IsImageDynamicallyRelocated : 1;
			UCHAR                       SkipPatchingUser32Forwarders : 1;
			UCHAR                       SpareBits : 3;
		};
		/* 0x004 */ PVOID                           Mutant;
		/* 0x008 */ PVOID                           ImageBaseAddress;
		/* 0x00c */ PPEB_LDR_DATA                   Ldr;
		/* 0x010 */ PRTL_USER_PROCESS_PARAMETERS    ProcessParameters;
		/* 0x014 */ PVOID                           SubSystemData;
		/* 0x018 */ HANDLE                          ProcessHeap;
		/* 0x01c */ PRTL_CRITICAL_SECTION           FastPebLock;
		/* 0x020 */ PVOID                           AtlThunkSListPtr;
		/* 0x024 */ PVOID                           IFEOKey;
		/* 0x028 */ union
		{
			ULONG_PTR CrossProcessFlags;
			struct
			{
				UCHAR ProcessInJob : 1;
				UCHAR ProcessInitializing : 1;
				UCHAR ProcessUsingVEH : 1;
				UCHAR ProcessUsingVCH : 1;
				UCHAR ProcessUsingFTH : 1;
			};
		};

		/* 0x02C */ union
		{
			PVOID                       KernelCallbackTable;
			PVOID                       UserSharedInfoPtr;
		};

		/* 0x030 */ ULONG                           SystemReserved[1];
		/* 0x034 */ ULONG                           AtlThunkSListPtr32;
		/* 0x038 */ PVOID                           ApiSetMap;
		/* 0x03c */ ULONG                           TlsExpansionCounter;
		/* 0x040 */ PVOID                           TlsBitmap;
		/* 0x044 */ ULONG                           TlsBitmapBits[2];
		/* 0x04c */ PVOID                           ReadOnlySharedMemoryBase;
		/* 0x050 */ PVOID                           HotpatchInformation;
		/* 0x054 */ STATIC_SERVER_DATA_PTR          ReadOnlyStaticServerData;
		/* 0x058 */ PUSHORT                         AnsiCodePageData;
		/* 0x05c */ PUSHORT                         OemCodePageData;
		/* 0x060 */ PVOID                           UnicodeCaseTableData;
		/* 0x064 */ ULONG                           NumberOfProcessors;
		/* 0x068 */ ULONG                           NtGlobalFlag;
		/* 0x06C */ ULONG                           Dummy;
		/* 0x070 */ LARGE_INTEGER                   CriticalSectionTimeout;
		/* 0x078 */ ULONG                           HeapSegmentReserve;
		/* 0x07c */ ULONG                           HeapSegmentCommit;
		/* 0x080 */ ULONG                           HeapDeCommitTotalFreeThreshold;
		/* 0x084 */ ULONG                           HeapDeCommitFreeBlockThreshold;
		/* 0x088 */ ULONG                           NumberOfHeaps;
		/* 0x08c */ ULONG                           MaximumNumberOfHeaps;
		/* 0x090 */ PVOID                           ProcessHeaps;
		/* 0x094 */ PVOID                           GdiSharedHandleTable;
		/* 0x098 */ PVOID                           ProcessStarterHelper;
		/* 0x09c */ ULONG                           GdiDCAttributeList;
		/* 0x0a0 */ PRTL_CRITICAL_SECTION           LoaderLock;
		/* 0x0a4 */ ULONG                           OSMajorVersion;
		/* 0x0a8 */ ULONG                           OSMinorVersion;
		/* 0x0ac */ USHORT                          OSBuildNumber;
		/* 0x0ae */ USHORT                          OSCSDVersion;
		/* 0x0b0 */ ULONG                           OSPlatformId;
	} PEB_BASE, *PPEB_BASE;


	typedef struct TEB_BASE
	{
		/* 0x000 */ NT_TIB      NtTib;
		/* 0x01C */ PVOID       EnvironmentPointer;
		/* 0x020 */ CLIENT_ID   ClientId;
		/* 0x028 */ HANDLE      ActiveRpcHandle;
		/* 0x02C */ PVOID       ThreadLocalStoragePointer;
		/* 0x030 */ PPEB_BASE   ProcessEnvironmentBlock;
		/* 0x034 */ ULONG       LastErrorValue;
		/* 0x038 */ ULONG       CountOfOwnedCriticalSections;
		/* 0x03C */ PVOID       CsrClientThread;
		/* 0x040 */ PVOID       Win32ThreadInfo;
		/* 0x044 */ ULONG       User32Reserved[26];
		/* 0x0AC */ ULONG       UserReserved[5];
		/* 0x0C0 */ PVOID       WOW32Reserved;
		/* 0x0C4 */ ULONG       CurrentLocale;
		/* 0x0C8 */ ULONG       FpSoftwareStatusRegister;
		/* 0x0CC */ PVOID       SystemReserved1[54];
		/* 0x1A4 */ LONG        ExceptionCode;

	} TEB_BASE, *PTEB_BASE;

	typedef struct _OBJECT_ATTRIBUTES {
		ULONG Length;
		HANDLE RootDirectory;
		PUNICODE_STRING ObjectName;
		ULONG Attributes;
		PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
		PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
	} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

#define FIELD_BASE(address, type, field) (type *)((ULONG_PTR)address - (ULONG_PTR)&((type *)0)->field)
#define FIELD_TYPE(_Type, _Field)  TYPE_OF(((_Type*)0)->_Field)

#define TEB_OFFSET FIELD_OFFSET(TEB_BASE, NtTib.Self)


	FORCEINLINE PPEB_BASE Nt_CurrentPeb()
	{
		return (PPEB_BASE)(ULONG_PTR)__readfsdword(FIELD_OFFSET(TEB_BASE, ProcessEnvironmentBlock));
	}

	FORCEINLINE PTEB_BASE Nt_CurrentTeb()
	{
		return (PTEB_BASE)__readfsdword(TEB_OFFSET);
	}

#ifdef __cplusplus
#define EXTCPP extern "C++"
#endif

	namespace PointerOperationType
	{
		enum PointerOperationClass
		{
			POINTER_OP_ADD,
			POINTER_OP_SUB,
			POINTER_OP_AND,
			POINTER_OP_OR,
			POINTER_OP_XOR,
			POINTER_OP_MOD,
		};
	}

	EXTCPP
		template<PointerOperationType::PointerOperationClass OP, class PtrType1, class PtrType2>
	FORCEINLINE
		PtrType1 PtrOperator(PtrType1 Ptr1, PtrType2 Ptr2)
	{
#pragma warning(push, 0)
		struct
		{
			union
			{
				PtrType1        _Ptr1;
				LARGE_INTEGER   Value1;
			};

			union
			{
				PtrType2        _Ptr2;
				LARGE_INTEGER   Value2;
			};
		} u;

#pragma warning(push)
#pragma warning(disable:4702)

		if (MAX(sizeof(Ptr1), sizeof(Ptr2)) == sizeof(u.Value1.QuadPart))
		{
			u.Value1.QuadPart = 0;
			u.Value2.QuadPart = 0;
		}
		else
		{
			u.Value1.LowPart = NULL;
			u.Value2.LowPart = NULL;
		}

#pragma warning(pop)

		u._Ptr1 = Ptr1;
		u._Ptr2 = Ptr2;

		if (MAX(sizeof(Ptr1), sizeof(Ptr2)) == sizeof(u.Value1.QuadPart))
		{
			switch (OP)
			{
			case PointerOperationType::POINTER_OP_ADD:
				u.Value1.QuadPart += u.Value2.QuadPart;
				break;

			case PointerOperationType::POINTER_OP_SUB:
				u.Value1.QuadPart -= u.Value2.QuadPart;
				break;

			case PointerOperationType::POINTER_OP_AND:
				u.Value1.QuadPart &= u.Value2.QuadPart;
				break;

			case PointerOperationType::POINTER_OP_OR:
				u.Value1.QuadPart |= u.Value2.QuadPart;
				break;

			case PointerOperationType::POINTER_OP_XOR:
				u.Value1.QuadPart ^= u.Value2.QuadPart;
				break;

			case PointerOperationType::POINTER_OP_MOD:
				u.Value1.QuadPart %= u.Value2.QuadPart;
				break;
			}
		}
		else
		{
			switch (OP)
			{
			case PointerOperationType::POINTER_OP_ADD:
				u.Value1.LowPart += u.Value2.LowPart;
				break;

			case PointerOperationType::POINTER_OP_SUB:
				u.Value1.LowPart -= u.Value2.LowPart;
				break;

			case PointerOperationType::POINTER_OP_AND:
				u.Value1.LowPart &= u.Value2.LowPart;
				break;

			case PointerOperationType::POINTER_OP_OR:
				u.Value1.LowPart |= u.Value2.LowPart;
				break;

			case PointerOperationType::POINTER_OP_XOR:
				u.Value1.LowPart ^= u.Value2.LowPart;
				break;

			case PointerOperationType::POINTER_OP_MOD:
				u.Value1.LowPart %= u.Value2.LowPart;
				break;
			}
		}

#pragma warning(pop)

		return u._Ptr1;
	}

	EXTCPP
		template<PointerOperationType::PointerOperationClass OP, class PtrType1>
	FORCEINLINE
		PtrType1 PtrOperator(PtrType1 Ptr1, unsigned short Ptr2)
	{
		return PtrOperator<OP>(Ptr1, (ULONG_PTR)Ptr2);
	}

	EXTCPP
		template<PointerOperationType::PointerOperationClass OP, class PtrType1>
	FORCEINLINE
		PtrType1 PtrOperator(PtrType1 Ptr1, short Ptr2)
	{
		return PtrOperator<OP>(Ptr1, (ULONG_PTR)Ptr2);
	}

	EXTCPP
		template<PointerOperationType::PointerOperationClass OP, class PtrType1>
	FORCEINLINE
		PtrType1 PtrOperator(PtrType1 Ptr1, unsigned char Ptr2)
	{
		return PtrOperator<OP>(Ptr1, (ULONG_PTR)Ptr2);
	}

	EXTCPP
		template<PointerOperationType::PointerOperationClass OP, class PtrType1>
	FORCEINLINE
		PtrType1 PtrOperator(PtrType1 Ptr1, char Ptr2)
	{
		return PtrOperator<OP>(Ptr1, (ULONG_PTR)Ptr2);
	}

	EXTCPP
		template<class PtrType1, class PtrType2>
	FORCEINLINE
		ULONG_PTR PtrOffset(PtrType1 Ptr, PtrType2 Offset)
	{
#pragma warning(push, 0)
		union
		{
			PtrType1    _Ptr1;
			ULONG_PTR   Value;
		};
#pragma warning(pop)

		_Ptr1 = Ptr;

		return PtrOperator<PointerOperationType::POINTER_OP_SUB>(Value, Offset);
	}

	EXTCPP
		template<class PtrType1, class PtrType2>
	FORCEINLINE
		PtrType1 PtrAdd(PtrType1 Ptr, PtrType2 Offset)
	{
		return PtrOperator<PointerOperationType::POINTER_OP_ADD>(Ptr, Offset);
	}

	EXTCPP
		template<class PtrType1, class PtrType2>
	FORCEINLINE
		PtrType1 PtrSub(PtrType1 Ptr, PtrType2 Offset)
	{
		return PtrOperator<PointerOperationType::POINTER_OP_SUB>(Ptr, Offset);
	}

	EXTCPP
		template<class PtrType1, class PtrType2>
	FORCEINLINE
		PtrType1 PtrAnd(PtrType1 Ptr, PtrType2 Offset)
	{
		return PtrOperator<PointerOperationType::POINTER_OP_AND>(Ptr, Offset);
	}

	EXTCPP
		template<class PtrType1, class PtrType2>
	FORCEINLINE
		PtrType1 PtrOr(PtrType1 Ptr, PtrType2 Offset)
	{
		return PtrOperator<PointerOperationType::POINTER_OP_OR>(Ptr, Offset);
	}

	EXTCPP
		template<class PtrType1, class PtrType2>
	FORCEINLINE
		PtrType1 PtrXor(PtrType1 Ptr, PtrType2 Offset)
	{
		return PtrOperator<PointerOperationType::POINTER_OP_XOR>(Ptr, Offset);
	}

	EXTCPP
		template<class PtrType1, class PtrType2>
	FORCEINLINE
		PtrType1 PtrMod(PtrType1 Ptr, PtrType2 Offset)
	{
		return PtrOperator<PointerOperationType::POINTER_OP_MOD>(Ptr, Offset);
	}



NATIVE_API
PVOID
NTAPI
RtlAllocateHeap(
IN HANDLE   HeapBase,
IN ULONG    Flags,
IN SIZE_T   Bytes
);

NATIVE_API
NTSTATUS
NTAPI
RtlCustomCPToUnicodeN(
IN  PCPTABLEINFO    CPTableInfo,
OUT PWSTR           UnicodeString,
IN  ULONG           MaxBytesInUnicodeString,
OUT PULONG          BytesInUnicodeString OPTIONAL,
IN  PCSTR           CustomCPString,
IN  ULONG           BytesInCustomCPString
);


NATIVE_API
VOID
NTAPI
RtlPushFrame(
PTEB_ACTIVE_FRAME Frame
);


NATIVE_API
PTEB_ACTIVE_FRAME
NTAPI
RtlGetFrame(
VOID
);


NATIVE_API
VOID
NTAPI
RtlPopFrame(
PTEB_ACTIVE_FRAME Frame
);


NATIVE_API
BOOLEAN
NTAPI
RtlFreeHeap(
IN HANDLE   HeapBase,
IN ULONG    Flags,
IN LPVOID   Memory
);


NATIVE_API
NTSTATUS
NTAPI
RtlUnicodeToCustomCPN(
IN  PCPTABLEINFO    CPTableInfo,
OUT PCHAR           CustomCPString,
IN  ULONG           BytesInCustomCPString,
OUT PULONG          BytesInMultiByteString OPTIONAL,
IN  PCWSTR          UnicodeString,
IN  ULONG           BytesInUnicodeString
);


NATIVE_API
NTSTATUS
NTAPI
NtGetContextThread(
IN  HANDLE ThreadHandle,
OUT PCONTEXT Context
);


NATIVE_API
NTSTATUS
NTAPI
NtSetContextThread(
IN HANDLE ThreadHandle,
IN PCONTEXT Context
);

NATIVE_API
NTSTATUS
NTAPI
NtDelayExecution(
IN BOOLEAN          Alertable,
IN PLARGE_INTEGER   Interval
);


NATIVE_API
NTSTATUS
NTAPI
RtlGetVersion(
IN OUT PRTL_OSVERSIONINFOW lpVersionInformation
);


NATIVE_API
NTSTATUS
NTAPI
NtResumeThread(
IN  HANDLE ThreadHandle,
OUT PULONG PreviousSuspendCount OPTIONAL
);


NATIVE_API
HANDLE
NTAPI
RtlCreateHeap(
IN ULONG                Flags,
IN PVOID                HeapBase OPTIONAL,
IN SIZE_T               ReserveSize OPTIONAL,
IN SIZE_T               CommitSize OPTIONAL,
IN PVOID                Lock OPTIONAL,
IN PRTL_HEAP_PARAMETERS Parameters OPTIONAL
);


NATIVE_API
NTSTATUS
NTAPI
NtFlushInstructionCache(
IN HANDLE ProcessHandle,
IN PVOID  BaseAddress,
IN SIZE_T NumberOfBytesToFlush
);


NATIVE_API
PVOID
NTAPI
RtlReAllocateHeap(
IN HANDLE   HeapBase,
IN ULONG    Flags,
IN PVOID    Memory,
IN SIZE_T   Bytes
);


NATIVE_API
NTSTATUS
NTAPI
LdrLoadDll(
IN  PWSTR               PathToFile OPTIONAL,
IN  PULONG              DllCharacteristics OPTIONAL,
IN  PCUNICODE_STRING    ModuleFileName,
OUT PVOID*              DllHandle
);


NATIVE_API
NTSTATUS
NTAPI
NtOpenSection(
OUT PHANDLE             SectionHandle,
IN  ACCESS_MASK         DesiredAccess,
IN  POBJECT_ATTRIBUTES  ObjectAttributes
);


NATIVE_API
NTSTATUS
NTAPI
NtClose(
IN HANDLE Handle
);


NATIVE_API
VOID
NTAPI
RtlInitCodePageTable(
IN  PUSHORT         TableBase,
OUT PCPTABLEINFO    CodePageTable
);


EXTC_IMPORT
BOOL
WINAPI
CreateProcessInternalA(
HANDLE                  Token,
LPCSTR                  ApplicationName,
LPSTR                   CommandLine,
LPSECURITY_ATTRIBUTES   ProcessAttributes,
LPSECURITY_ATTRIBUTES   ThreadAttributes,
BOOL                    InheritHandles,
DWORD                   CreationFlags,
LPVOID                  Environment,
LPCSTR                  CurrentDirectory,
LPSTARTUPINFOA          StartupInfo,
LPPROCESS_INFORMATION   ProcessInformation,
PHANDLE                 NewToken
);



EXTC_IMPORT
BOOL
WINAPI
CreateProcessInternalW(
HANDLE                  Token,
PCWSTR                  ApplicationName,
PWSTR                   CommandLine,
LPSECURITY_ATTRIBUTES   ProcessAttributes,
LPSECURITY_ATTRIBUTES   ThreadAttributes,
BOOL                    InheritHandles,
DWORD                   CreationFlags,
LPVOID                  Environment,
PCWSTR                  CurrentDirectory,
LPSTARTUPINFOW          StartupInfo,
LPPROCESS_INFORMATION   ProcessInformation,
PHANDLE                 NewToken
);


NATIVE_API
NTSTATUS
NTAPI
RtlMultiByteToUnicodeN(
PWSTR   UnicodeString,
ULONG   MaxBytesInUnicodeString,
PULONG  BytesInUnicodeString OPTIONAL,
PCSTR   MultiByteString,
ULONG   BytesInMultiByteString
);

NATIVE_API
NTSTATUS
NTAPI
RtlUnicodeToMultiByteN(
OUT PSTR    MultiByteString,
IN  ULONG   MaxBytesInMultiByteString,
OUT PULONG  BytesInMultiByteString OPTIONAL,
IN  PCWSTR  UnicodeString,
IN  ULONG   BytesInUnicodeString
);


NATIVE_API
NTSTATUS
NTAPI
RtlMultiByteToUnicodeSize(
OUT PULONG  BytesInUnicodeString,
IN  PCSTR   MultiByteString,
IN  ULONG   BytesInMultiByteString
);

NATIVE_API
VOID
NTAPI
RtlSetLastWin32ErrorAndNtStatusFromNtStatus(
IN NTSTATUS NtStatus
);


NATIVE_API
NTSTATUS
NTAPI
RtlUnicodeToMultiByteSize(
PULONG  BytesInMultiByteString,
PCWSTR  UnicodeString,
ULONG   BytesInUnicodeString
);

typedef enum
{
	ViewShare = 1,
	ViewUnmap = 2,

} SECTION_INHERIT;

NATIVE_API
NTSTATUS
NTAPI
NtMapViewOfSection(
IN      HANDLE          SectionHandle,
IN      HANDLE          ProcessHandle,
IN OUT  PVOID          *BaseAddress,
IN      ULONG_PTR       ZeroBits,
IN      SIZE_T          CommitSize,
IN OUT  PLARGE_INTEGER  SectionOffset OPTIONAL,
IN OUT  PSIZE_T         ViewSize,
IN      SECTION_INHERIT InheritDisposition,
IN      ULONG           AllocationType,
IN      ULONG           Win32Protect
);

NATIVE_API
NTSTATUS
NTAPI
LdrGetProcedureAddress(
IN  PVOID           DllHandle,
IN  PANSI_STRING    ProcedureName OPTIONAL,
IN  USHORT          ProcedureNumber OPTIONAL,
OUT PVOID*          ProcedureAddress
);

#ifndef OBJ_INHERIT
#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_KERNEL_HANDLE       0x00000200L
#define OBJ_FORCE_ACCESS_CHECK  0x00000400L
#define OBJ_VALID_ATTRIBUTES    0x000007F2L
#endif // OBJ_INHERIT

#undef InitializeObjectAttributes
inline VOID InitializeObjectAttributes(POBJECT_ATTRIBUTES p, PUNICODE_STRING n, ULONG a, HANDLE r, PVOID s)
{
	p->Length = sizeof(*p);
	p->RootDirectory = r;
	p->Attributes = a;
	p->ObjectName = n;
	p->SecurityDescriptor = s;
	p->SecurityQualityOfService = NULL;

	*(volatile PUNICODE_STRING *)&n = n;
}


inline PLDR_MODULE GetKernel32Ldr()
{
	LDR_MODULE *Ldr, *FirstLdr;

	Ldr = FIELD_BASE(Nt_CurrentPeb()->Ldr->InInitializationOrderModuleList.Flink, LDR_MODULE, InInitializationOrderLinks);
	FirstLdr = Ldr;

	do
	{
		Ldr = FIELD_BASE(Ldr->InInitializationOrderLinks.Flink, LDR_MODULE, InInitializationOrderLinks);
		if (Ldr->BaseDllName.Buffer == NULL)
			continue;

		if (CHAR_UPPER4W(*(PULONG64)(Ldr->BaseDllName.Buffer + 0)) != TAG4W('KERN') ||
			CHAR_UPPER4W(*(PULONG64)(Ldr->BaseDllName.Buffer + 4)) != CHAR_UPPER4W(TAG4W('EL32')) ||
			Ldr->BaseDllName.Buffer[8] != '.')
		{
			continue;
		}

		return Ldr;

	} while (FirstLdr != Ldr);

	return NULL;
}

inline PVOID GetKernel32Handle()
{
	return GetKernel32Ldr()->DllBase;
}

FORCEINLINE PLDR_MODULE GetNtdllLdrModule()
{
	return FIELD_BASE(Nt_CurrentPeb()->Ldr->InInitializationOrderModuleList.Flink, LDR_MODULE, InInitializationOrderLinks);
}

FORCEINLINE PVOID GetNtdllHandle()
{
	return GetNtdllLdrModule()->DllBase;
}

FORCEINLINE PUSHORT Nt_GetDefaultCodePageBase()
{
	return Nt_CurrentPeb()->AnsiCodePageData;
}


#define BaseSetLastNTError RtlSetLastWin32ErrorAndNtStatusFromNtStatus

#define DELAY_ONE_MICROSECOND   (-10)
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND * 1000)
#define DELAY_ONE_SECOND        (DELAY_ONE_MILLISECOND * 1000)
#define DELAY_QUAD_INFINITE     0x8000000000000000ll

inline PLARGE_INTEGER FormatTimeOut(PLARGE_INTEGER TimeOut, ULONG Milliseconds)
{
	if (Milliseconds == INFINITE)
	{
		//TimeOut->LowPart = 0;
		//TimeOut->HighPart = 0x80000000;
		TimeOut->QuadPart = DELAY_QUAD_INFINITE;
	}
	else
	{
		TimeOut->QuadPart = DELAY_ONE_MILLISECOND * (LONG64)Milliseconds;
	}

	return TimeOut;
}


NATIVE_API
NTSTATUS
NTAPI
ZwReadVirtualMemory(
IN    HANDLE  ProcessHandle,
IN    PVOID   BaseAddress,
OUT   PVOID   Buffer,
IN    SIZE_T  NumberOfBytesToRead,
OUT   PSIZE_T NumberOfBytesRead OPTIONAL
);

inline
NTSTATUS
Nt_ReadMemory(
HANDLE      ProcessHandle,
PVOID       BaseAddress,
PVOID       Buffer,
ULONG_PTR   Size,
PULONG_PTR  BytesRead /* = NULL */
)
{
	return ZwReadVirtualMemory(ProcessHandle, BaseAddress, Buffer, Size, BytesRead);
}


NATIVE_API
NTSTATUS
NTAPI
LdrFindEntryForAddress(
IN  PVOID Address,
OUT PLDR_DATA_TABLE_ENTRY *TableEntry
);

PLDR_MODULE
Nt_FindLdrModuleByHandle(
PVOID BaseAddress
);



NATIVE_API
NTSTATUS
NTAPI
ZwWriteVirtualMemory(
IN    HANDLE  ProcessHandle,
IN    PVOID   BaseAddress,
IN    PVOID   Buffer,
IN    SIZE_T  NumberOfBytesToWrite,
OUT   PSIZE_T NumberOfBytesWritten OPTIONAL
);


NTSTATUS
Nt_WriteMemory(
HANDLE      ProcessHandle,
PVOID       BaseAddress,
PVOID       Buffer,
ULONG_PTR   Size,
PULONG_PTR  DEF_VAL(BytesWritten, NULL)
);



NATIVE_API
NTSTATUS
NTAPI
NtFreeVirtualMemory(
IN      HANDLE  ProcessHandle,
IN OUT  PVOID  *BaseAddress,
IN OUT  PSIZE_T RegionSize,
IN      ULONG   FreeType
);

NTSTATUS
Nt_FreeMemory(
HANDLE  ProcessHandle,
PVOID   BaseAddress
);

NATIVE_API
NTSTATUS
NTAPI
NtAllocateVirtualMemory(
IN      HANDLE      ProcessHandle,
IN OUT  PVOID*      BaseAddress,
IN      ULONG_PTR   ZeroBits,
IN OUT  PSIZE_T     RegionSize,
IN      ULONG       AllocationType,
IN      ULONG       Protect
);


NTSTATUS
Nt_AllocateMemory(
HANDLE      ProcessHandle,
PVOID*      BaseAddress,
ULONG_PTR   Size,
ULONG       DEF_VAL(Protect, PAGE_EXECUTE_READWRITE),
ULONG       DEF_VAL(AllocationType, MEM_RESERVE | MEM_COMMIT)
);

NATIVE_API ULONG   NlsAnsiCodePage;
NATIVE_API BOOLEAN NlsMbCodePageTag;

NATIVE_API
NTSTATUS
NTAPI
LdrGetDllHandle(
IN  PUSHORT         PathToFile OPTIONAL,
IN  PULONG          DllCharacteristics OPTIONAL,
IN  PUNICODE_STRING ModuleFileName,
OUT PVOID*          DllHandle
);


NATIVE_API
NTSTATUS
NTAPI
NtAddAtom(
PCWSTR  AtomString,
ULONG   BytesInString,
ATOM   *Atom
);


PTEB_ACTIVE_FRAME
Nt_FindThreadFrameByContext(
ULONG_PTR Context
);


NATIVE_API
NTSTATUS
NTAPI
NtQueryAttributesFile(
IN  POBJECT_ATTRIBUTES      ObjectAttributes,
OUT PFILE_BASIC_INFORMATION FileInformation
);


NATIVE_API
NTSTATUS
NTAPI
NtOpenFile(
PHANDLE             FileHandle,
ACCESS_MASK         DesiredAccess,
POBJECT_ATTRIBUTES  ObjectAttributes,
PIO_STATUS_BLOCK    IoStatusBlock,
ULONG               ShareAccess,
ULONG               OpenOptions
);

NATIVE_API
NTSTATUS
NTAPI
NtCreateSection(
OUT PHANDLE             SectionHandle,
IN  ACCESS_MASK         DesiredAccess,
IN  POBJECT_ATTRIBUTES  ObjectAttributes OPTIONAL,
IN  PLARGE_INTEGER      MaximumSize,
IN  ULONG               SectionPageProtection,
IN  ULONG               AllocationAttributes,
IN  HANDLE              FileHandle OPTIONAL
);


NATIVE_API
NTSTATUS
NTAPI
NtQuerySection(
IN  HANDLE                      SectionHandle,
IN  SECTION_INFORMATION_CLASS   SectionInformationClass,
OUT PVOID                       SectionInformation,
IN  SIZE_T                      Length,
OUT PULONG                      ReturnLength
);

NATIVE_API
BOOLEAN
NTAPI
RtlDosPathNameToNtPathName_U(
IN  PCWSTR                  DosName,
OUT PUNICODE_STRING         NtName,
OUT PCWSTR*                 DosFilePath OPTIONAL,
OUT PRTL_RELATIVE_NAME_U    FileName OPTIONAL
);


NATIVE_API
VOID
NTAPI
RtlFreeUnicodeString(
PUNICODE_STRING UnicodeString
);


NATIVE_API
VOID
NTAPI
RtlInitUnicodeString(
IN OUT  PUNICODE_STRING DestinationString,
IN      LPCWSTR         SourceString
);


NATIVE_API
LONG
NTAPI
RtlCompareUnicodeString(
PCUNICODE_STRING    String1,
PCUNICODE_STRING    String2,
BOOLEAN             CaseInSensitive
);
