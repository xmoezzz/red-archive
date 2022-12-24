#ifndef _NtCore_Xmoe_
#define _NtCore_Xmoe_

#include <Windows.h>

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

#define True  TRUE
#define False FALSE

#if defined(_WIN64)
typedef __int64 Int_Ptr, *PInt_Ptr;
typedef unsigned __int64 UInt_Ptr, *PUInt_Ptr;

typedef __int64 Long_Ptr, *PLong_Ptr, LongPtr, *PLongPtr;
typedef unsigned __int64 ULong_Ptr, *PULong_Ptr, ULongPtr, *PULongPtr;
#else
typedef int _w64 Int_Ptr, *PInt_Ptr;
typedef unsigned int _w64 UInt_Ptr, *PUInt_Ptr;

typedef long _w64 Long_Ptr, *PLong_Ptr, LongPtr, *PLongPtr;
typedef unsigned long _w64 ULong_Ptr, *PULong_Ptr, ULongPtr, *PULongPtr;
#endif

typedef ULong_Ptr SizeT, *PSizeT;
typedef Long_Ptr  SSizeT, *PSSizeT;

#define MAX_SHORT  (Short) (0x7FFF)
#define MAX_USHORT (UShort)(0xFFFF)
#define MAX_INT    (Int)   (0x7FFFFFFF)
#define MAX_UINT   (UInt)  (0xFFFFFFFF)
#define MAX_INT64  (Int64) (0x7FFFFFFFFFFFFFFF)
#define MAX_UINT64 (UInt64)(0xFFFFFFFFFFFFFFFF)
#define MAX_NTPATH  0x220

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

#define SWAP2(v) (u16)(((u32)(v) << 8) | ((u16)(v) >> 8))
#define SWAPCHAR(v) ((u32)SWAP2(v))

Bool IsShiftJISChar(SizeT uChar);
Bool IsShiftJISString(PCChar pString, SizeT Length);

#endif
