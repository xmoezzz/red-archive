#include <ntstatus.h>
#include "MemoryPatch.h"
#include "GrowableArray.h"
#include "NtDefine.h"
#include "MemoryAllocator.h"

//placement new
#include <new>

#ifdef WIN32
#define TRAMPOLINE_SIZE     0x40
#elif _AMD64
#define TRAMPOLINE_SIZE     0x100
#endif

#define OP_X86_NONE           0x000
#define OP_X86_DATA_I8        0x001
#define OP_X86_DATA_I16       0x002
#define OP_X86_DATA_I32       0x004
#define OP_X86_MODRM          0x008
#define OP_X86_DATA_PRE66_67  0x010
#define OP_X86_PREFIX         0x020
#define OP_X86_REL32          0x040
#define OP_X86_REL8           0x080

/* extended opcode flags (by analyzer) */
#define OP_X86_EXTENDED       0x100


BOOL FreeMemoryP(PVOID Memory, ULONG Flags = 0)
{
	return MemoryAllocator::FreeMemory(Memory, Flags);
}

PVOID ReAllocateMemoryP(PVOID Memory, ULONG_PTR Size, ULONG Flags = 0)
{
	return MemoryAllocator::ReAllocateMemory(Memory, Size, Flags);
}


PVOID AllocateMemory(ULONG_PTR Size, ULONG Flags)
{
	return RtlAllocateHeap(Nt_CurrentPeb()->ProcessHeap, Flags, Size);
}

VOID FreeMemory(PVOID lpMem)
{
	if (!lpMem)
		return;

	RtlFreeHeap(Nt_CurrentPeb()->ProcessHeap, 0, lpMem);
}


LONG_PTR MlInitialize()
{
	if (MemoryAllocator::GetGlobalHeap() == NULL &&
		MemoryAllocator::CreateGlobalHeap() == nullptr)
		return STATUS_NO_MEMORY;

	return STATUS_SUCCESS;
}



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

FORCEINLINE PBYTE GetPackedTable()
{
	static BYTE PackedTable[] =
	{
		0x80, 0x84, 0x80, 0x84, 0x80, 0x84, 0x80, 0x84,
		0x80, 0x88, 0x80, 0x88, 0x80, 0x88, 0x80, 0x88,
		0x8c, 0x8b, 0x8b, 0x8b, 0x8b, 0x8b, 0x8b, 0x8b,
		0x90, 0x94, 0x98, 0x8b, 0x9c, 0x9c, 0x9c, 0x9c,
		0xa0, 0x80, 0x80, 0x80, 0x8b, 0x8b, 0xa4, 0x8b,
		0xa8, 0x8b, 0x84, 0x8b, 0xac, 0xac, 0xa8, 0xa8,
		0xb0, 0xb4, 0xb8, 0xbc, 0x80, 0xc0, 0x80, 0x80,
		0x9c, 0xac, 0xc4, 0x8b, 0xc8, 0x90, 0x8b, 0x90,
		0x80, 0x8b, 0x8b, 0xcc, 0x80, 0x80, 0xd0, 0x8b,
		0x80, 0xd4, 0x80, 0x80, 0x8b, 0x8b, 0x8b, 0x8b,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		0x80, 0x80, 0x80, 0x80, 0xd8, 0xdc, 0x8b, 0x80,
		0xe0, 0xe0, 0xe0, 0xe0, 0x80, 0x80, 0x80, 0x80,
		0x8f, 0xcf, 0x8f, 0xdb, 0x80, 0x80, 0xe4, 0x80,
		0xe8, 0xd9, 0x8b, 0x8b, 0x80, 0x80, 0x80, 0x80,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xdc,
		0x08, 0x08, 0x08, 0x08, 0x01, 0x10, 0x00, 0x00,
		0x01, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x08, 0x08, 0x20, 0x20, 0x20, 0x20,
		0x10, 0x18, 0x01, 0x09, 0x81, 0x81, 0x81, 0x81,
		0x09, 0x18, 0x09, 0x09, 0x00, 0x00, 0x12, 0x00,
		0x10, 0x10, 0x10, 0x10, 0x01, 0x01, 0x01, 0x01,
		0x09, 0x09, 0x02, 0x00, 0x08, 0x08, 0x09, 0x18,
		0x03, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00,
		0x01, 0x01, 0x00, 0x00, 0x50, 0x50, 0x12, 0x81,
		0x20, 0x00, 0x20, 0x20, 0x00, 0x08, 0x00, 0x09,
		0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00,
		0x09, 0x09, 0x09, 0x09, 0x08, 0x08, 0x08, 0x00,
		0x50, 0x50, 0x50, 0x50, 0x00, 0x00, 0x09, 0x08,
		0x08, 0x08, 0x09, 0x08
	};

	return PackedTable;
}

FORCEINLINE ULONG_PTR GetOpCodeFlags32(ULONG_PTR OpCode)
{
	return GetPackedTable()[GetPackedTable()[OpCode / 4] + (OpCode % 4)];
}

ULONG_PTR FASTCALL LdeGetOpCodeSize32(PVOID Code, PVOID *OpCodePtr = NULL)
{
	unsigned char i_mod, i_rm, i_reg;
	ULONG_PTR op1, op2, flags;
	ULONG_PTR pfx66, pfx67;
	ULONG_PTR osize, oflen;
	PBYTE code;

	pfx66 = 0;
	pfx67 = 0;
	osize = 0;
	oflen = 0;

	code = (PBYTE)Code;
	op1 = *code;

	/* skip preffixes */
	while (GetOpCodeFlags32(op1) & OP_X86_PREFIX)
	{
		switch (op1)
		{
		case 0x66:
			pfx66 = 1;
			break;

		case 0x67:
			pfx67 = 1;
			break;
		}

		osize++;
		op1 = *++code;
	}

	/* get opcode size and flags */
	if (OpCodePtr != NULL)
		*OpCodePtr = code;

	++code;
	osize++;

	if (op1 == 0x0F)
	{
		op2 = (*code | OP_X86_EXTENDED);
		code++;
		osize++;
	}
	else
	{
		op2 = op1;

		/* pfx66 = pfx67 for opcodes A0 - A3 */
		if (op2 >= 0xA0 && op2 <= 0xA3)
		{
			pfx66 = pfx67;
		}
	}

	flags = GetOpCodeFlags32(op2);

	/* process MODRM byte */
	if (flags & OP_X86_MODRM)
	{
		i_mod = (*code >> 6);
		i_reg = (*code & 0x38) >> 3;
		i_rm = (*code & 7);
		code++;
		osize++;

		/* in F6 and F7 opcodes, immediate value present if i_reg == 0 */
		if (op1 == 0xF6 && i_reg == 0)
		{
			flags |= OP_X86_DATA_I8;
		}
		if (op1 == 0xF7 && i_reg == 0)
		{
			flags |= OP_X86_DATA_PRE66_67;
		}

		switch (i_mod)
		{
		case 0:
			if (pfx67)
			{
				if (i_rm == 6)
					oflen = 2;
			}
			else
			{
				if (i_rm == 5)
					oflen = 4;
			}
			break;

		case 1:
			oflen = 1;
			break;

		case 2:
			if (pfx67)
				oflen = 2;
			else
				oflen = 4;
			break;
		}

		/* process SIB byte */
		if (pfx67 == 0 && i_rm == 4 && i_mod != 3)
		{
			if ((*code & 7) == 5 && (i_mod != 1))
			{
				oflen = 4;
			}

			oflen++;
		}

		osize += oflen;
	}

	/* process offset */
	if (flags & OP_X86_DATA_PRE66_67)
	{
		osize += 4 - (pfx66 << 1);
	}
	/* process immediate value */
	osize += (flags & 7);

	return osize;
}


#define OP_X64_NONE           0x00
#define OP_X64_MODRM          0x01
#define OP_X64_DATA_I8        0x02
#define OP_X64_DATA_I16       0x04
#define OP_X64_DATA_I32       0x08
#define OP_X64_DATA_PRE66_67  0x10
#define OP_X64_WORD           0x20
#define OP_X64_REL32          0x40

FORCEINLINE PUCHAR GetOpCodeFlags()
{
	static UCHAR OpcodeFlags[256] =
	{
		OP_X64_MODRM,                      // 00
		OP_X64_MODRM,                      // 01
		OP_X64_MODRM,                      // 02
		OP_X64_MODRM,                      // 03
		OP_X64_DATA_I8,                    // 04
		OP_X64_DATA_PRE66_67,              // 05
		OP_X64_NONE,                       // 06
		OP_X64_NONE,                       // 07
		OP_X64_MODRM,                      // 08
		OP_X64_MODRM,                      // 09
		OP_X64_MODRM,                      // 0A
		OP_X64_MODRM,                      // 0B
		OP_X64_DATA_I8,                    // 0C
		OP_X64_DATA_PRE66_67,              // 0D
		OP_X64_NONE,                       // 0E
		OP_X64_NONE,                       // 0F
		OP_X64_MODRM,                      // 10
		OP_X64_MODRM,                      // 11
		OP_X64_MODRM,                      // 12
		OP_X64_MODRM,                      // 13
		OP_X64_DATA_I8,                    // 14
		OP_X64_DATA_PRE66_67,              // 15
		OP_X64_NONE,                       // 16
		OP_X64_NONE,                       // 17
		OP_X64_MODRM,                      // 18
		OP_X64_MODRM,                      // 19
		OP_X64_MODRM,                      // 1A
		OP_X64_MODRM,                      // 1B
		OP_X64_DATA_I8,                    // 1C
		OP_X64_DATA_PRE66_67,              // 1D
		OP_X64_NONE,                       // 1E
		OP_X64_NONE,                       // 1F
		OP_X64_MODRM,                      // 20
		OP_X64_MODRM,                      // 21
		OP_X64_MODRM,                      // 22
		OP_X64_MODRM,                      // 23
		OP_X64_DATA_I8,                    // 24
		OP_X64_DATA_PRE66_67,              // 25
		OP_X64_NONE,                       // 26
		OP_X64_NONE,                       // 27
		OP_X64_MODRM,                      // 28
		OP_X64_MODRM,                      // 29
		OP_X64_MODRM,                      // 2A
		OP_X64_MODRM,                      // 2B
		OP_X64_DATA_I8,                    // 2C
		OP_X64_DATA_PRE66_67,              // 2D
		OP_X64_NONE,                       // 2E
		OP_X64_NONE,                       // 2F
		OP_X64_MODRM,                      // 30
		OP_X64_MODRM,                      // 31
		OP_X64_MODRM,                      // 32
		OP_X64_MODRM,                      // 33
		OP_X64_DATA_I8,                    // 34
		OP_X64_DATA_PRE66_67,              // 35
		OP_X64_NONE,                       // 36
		OP_X64_NONE,                       // 37
		OP_X64_MODRM,                      // 38
		OP_X64_MODRM,                      // 39
		OP_X64_MODRM,                      // 3A
		OP_X64_MODRM,                      // 3B
		OP_X64_DATA_I8,                    // 3C
		OP_X64_DATA_PRE66_67,              // 3D
		OP_X64_NONE,                       // 3E
		OP_X64_NONE,                       // 3F
		OP_X64_NONE,                       // 40
		OP_X64_NONE,                       // 41
		OP_X64_NONE,                       // 42
		OP_X64_NONE,                       // 43
		OP_X64_NONE,                       // 44
		OP_X64_NONE,                       // 45
		OP_X64_NONE,                       // 46
		OP_X64_NONE,                       // 47
		OP_X64_NONE,                       // 48
		OP_X64_NONE,                       // 49
		OP_X64_NONE,                       // 4A
		OP_X64_NONE,                       // 4B
		OP_X64_NONE,                       // 4C
		OP_X64_NONE,                       // 4D
		OP_X64_NONE,                       // 4E
		OP_X64_NONE,                       // 4F
		OP_X64_NONE,                       // 50
		OP_X64_NONE,                       // 51
		OP_X64_NONE,                       // 52
		OP_X64_NONE,                       // 53
		OP_X64_NONE,                       // 54
		OP_X64_NONE,                       // 55
		OP_X64_NONE,                       // 56
		OP_X64_NONE,                       // 57
		OP_X64_NONE,                       // 58
		OP_X64_NONE,                       // 59
		OP_X64_NONE,                       // 5A
		OP_X64_NONE,                       // 5B
		OP_X64_NONE,                       // 5C
		OP_X64_NONE,                       // 5D
		OP_X64_NONE,                       // 5E
		OP_X64_NONE,                       // 5F
		OP_X64_NONE,                       // 60
		OP_X64_NONE,                       // 61
		OP_X64_MODRM,                      // 62
		OP_X64_MODRM,                      // 63
		OP_X64_NONE,                       // 64
		OP_X64_NONE,                       // 65
		OP_X64_NONE,                       // 66
		OP_X64_NONE,                       // 67
		OP_X64_DATA_PRE66_67,              // 68
		OP_X64_MODRM | OP_X64_DATA_PRE66_67,   // 69
		OP_X64_DATA_I8,                    // 6A
		OP_X64_MODRM | OP_X64_DATA_I8,         // 6B
		OP_X64_NONE,                       // 6C
		OP_X64_NONE,                       // 6D
		OP_X64_NONE,                       // 6E
		OP_X64_NONE,                       // 6F
		OP_X64_DATA_I8,                    // 70
		OP_X64_DATA_I8,                    // 71
		OP_X64_DATA_I8,                    // 72
		OP_X64_DATA_I8,                    // 73
		OP_X64_DATA_I8,                    // 74
		OP_X64_DATA_I8,                    // 75
		OP_X64_DATA_I8,                    // 76
		OP_X64_DATA_I8,                    // 77
		OP_X64_DATA_I8,                    // 78
		OP_X64_DATA_I8,                    // 79
		OP_X64_DATA_I8,                    // 7A
		OP_X64_DATA_I8,                    // 7B
		OP_X64_DATA_I8,                    // 7C
		OP_X64_DATA_I8,                    // 7D
		OP_X64_DATA_I8,                    // 7E
		OP_X64_DATA_I8,                    // 7F
		OP_X64_MODRM | OP_X64_DATA_I8,         // 80
		OP_X64_MODRM | OP_X64_DATA_PRE66_67,   // 81
		OP_X64_MODRM | OP_X64_DATA_I8,         // 82
		OP_X64_MODRM | OP_X64_DATA_I8,         // 83
		OP_X64_MODRM,                      // 84
		OP_X64_MODRM,                      // 85
		OP_X64_MODRM,                      // 86
		OP_X64_MODRM,                      // 87
		OP_X64_MODRM,                      // 88
		OP_X64_MODRM,                      // 89
		OP_X64_MODRM,                      // 8A
		OP_X64_MODRM,                      // 8B
		OP_X64_MODRM,                      // 8C
		OP_X64_MODRM,                      // 8D
		OP_X64_MODRM,                      // 8E
		OP_X64_MODRM,                      // 8F
		OP_X64_NONE,                       // 90
		OP_X64_NONE,                       // 91
		OP_X64_NONE,                       // 92
		OP_X64_NONE,                       // 93
		OP_X64_NONE,                       // 94
		OP_X64_NONE,                       // 95
		OP_X64_NONE,                       // 96
		OP_X64_NONE,                       // 97
		OP_X64_NONE,                       // 98
		OP_X64_NONE,                       // 99
		OP_X64_DATA_I16 | OP_X64_DATA_PRE66_67,// 9A
		OP_X64_NONE,                       // 9B
		OP_X64_NONE,                       // 9C
		OP_X64_NONE,                       // 9D
		OP_X64_NONE,                       // 9E
		OP_X64_NONE,                       // 9F
		OP_X64_DATA_PRE66_67,              // A0
		OP_X64_DATA_PRE66_67,              // A1
		OP_X64_DATA_PRE66_67,              // A2
		OP_X64_DATA_PRE66_67,              // A3
		OP_X64_NONE,                       // A4
		OP_X64_NONE,                       // A5
		OP_X64_NONE,                       // A6
		OP_X64_NONE,                       // A7
		OP_X64_DATA_I8,                    // A8
		OP_X64_DATA_PRE66_67,              // A9
		OP_X64_NONE,                       // AA
		OP_X64_NONE,                       // AB
		OP_X64_NONE,                       // AC
		OP_X64_NONE,                       // AD
		OP_X64_NONE,                       // AE
		OP_X64_NONE,                       // AF
		OP_X64_DATA_I8,                    // B0
		OP_X64_DATA_I8,                    // B1
		OP_X64_DATA_I8,                    // B2
		OP_X64_DATA_I8,                    // B3
		OP_X64_DATA_I8,                    // B4
		OP_X64_DATA_I8,                    // B5
		OP_X64_DATA_I8,                    // B6
		OP_X64_DATA_I8,                    // B7
		OP_X64_DATA_PRE66_67,              // B8
		OP_X64_DATA_PRE66_67,              // B9
		OP_X64_DATA_PRE66_67,              // BA
		OP_X64_DATA_PRE66_67,              // BB
		OP_X64_DATA_PRE66_67,              // BC
		OP_X64_DATA_PRE66_67,              // BD
		OP_X64_DATA_PRE66_67,              // BE
		OP_X64_DATA_PRE66_67,              // BF
		OP_X64_MODRM | OP_X64_DATA_I8,         // C0
		OP_X64_MODRM | OP_X64_DATA_I8,         // C1
		OP_X64_DATA_I16,                   // C2
		OP_X64_NONE,                       // C3
		OP_X64_MODRM,                      // C4
		OP_X64_MODRM,                      // C5
		OP_X64_MODRM | OP_X64_DATA_I8,       // C6
		OP_X64_MODRM | OP_X64_DATA_PRE66_67, // C7
		OP_X64_DATA_I8 | OP_X64_DATA_I16,      // C8
		OP_X64_NONE,                       // C9
		OP_X64_DATA_I16,                   // CA
		OP_X64_NONE,                       // CB
		OP_X64_NONE,                       // CC
		OP_X64_DATA_I8,                    // CD
		OP_X64_NONE,                       // CE
		OP_X64_NONE,                       // CF
		OP_X64_MODRM,                      // D0
		OP_X64_MODRM,                      // D1
		OP_X64_MODRM,                      // D2
		OP_X64_MODRM,                      // D3
		OP_X64_DATA_I8,                    // D4
		OP_X64_DATA_I8,                    // D5
		OP_X64_NONE,                       // D6
		OP_X64_NONE,                       // D7
		OP_X64_WORD,                       // D8
		OP_X64_WORD,                       // D9
		OP_X64_WORD,                       // DA
		OP_X64_WORD,                       // DB
		OP_X64_WORD,                       // DC
		OP_X64_WORD,                       // DD
		OP_X64_WORD,                       // DE
		OP_X64_WORD,                       // DF
		OP_X64_DATA_I8,                    // E0
		OP_X64_DATA_I8,                    // E1
		OP_X64_DATA_I8,                    // E2
		OP_X64_DATA_I8,                    // E3
		OP_X64_DATA_I8,                    // E4
		OP_X64_DATA_I8,                    // E5
		OP_X64_DATA_I8,                    // E6
		OP_X64_DATA_I8,                    // E7
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // E8
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // E9
		OP_X64_DATA_I16 | OP_X64_DATA_PRE66_67,// EA
		OP_X64_DATA_I8,                    // EB
		OP_X64_NONE,                       // EC
		OP_X64_NONE,                       // ED
		OP_X64_NONE,                       // EE
		OP_X64_NONE,                       // EF
		OP_X64_NONE,                       // F0
		OP_X64_NONE,                       // F1
		OP_X64_NONE,                       // F2
		OP_X64_NONE,                       // F3
		OP_X64_NONE,                       // F4
		OP_X64_NONE,                       // F5
		OP_X64_MODRM,                      // F6
		OP_X64_MODRM,                      // F7
		OP_X64_NONE,                       // F8
		OP_X64_NONE,                       // F9
		OP_X64_NONE,                       // FA
		OP_X64_NONE,                       // FB
		OP_X64_NONE,                       // FC
		OP_X64_NONE,                       // FD
		OP_X64_MODRM,                      // FE
		OP_X64_MODRM | OP_X64_REL32            // FF
	};

	return OpcodeFlags;
}


FORCEINLINE PUCHAR GetOpCodeFlagsExt()
{
	static UCHAR OpcodeFlagsExt[256] =
	{
		OP_X64_MODRM,                      // 00
		OP_X64_MODRM,                      // 01
		OP_X64_MODRM,                      // 02
		OP_X64_MODRM,                      // 03
		OP_X64_NONE,                       // 04
		OP_X64_NONE,                       // 05
		OP_X64_NONE,                       // 06
		OP_X64_NONE,                       // 07
		OP_X64_NONE,                       // 08
		OP_X64_NONE,                       // 09
		OP_X64_NONE,                       // 0A
		OP_X64_NONE,                       // 0B
		OP_X64_NONE,                       // 0C
		OP_X64_MODRM,                      // 0D
		OP_X64_NONE,                       // 0E
		OP_X64_MODRM | OP_X64_DATA_I8,         // 0F
		OP_X64_MODRM,                      // 10
		OP_X64_MODRM,                      // 11
		OP_X64_MODRM,                      // 12
		OP_X64_MODRM,                      // 13
		OP_X64_MODRM,                      // 14
		OP_X64_MODRM,                      // 15
		OP_X64_MODRM,                      // 16
		OP_X64_MODRM,                      // 17
		OP_X64_MODRM,                      // 18
		OP_X64_NONE,                       // 19
		OP_X64_NONE,                       // 1A
		OP_X64_NONE,                       // 1B
		OP_X64_NONE,                       // 1C
		OP_X64_NONE,                       // 1D
		OP_X64_NONE,                       // 1E
		OP_X64_NONE,                       // 1F
		OP_X64_MODRM,                      // 20
		OP_X64_MODRM,                      // 21
		OP_X64_MODRM,                      // 22
		OP_X64_MODRM,                      // 23
		OP_X64_MODRM,                      // 24
		OP_X64_NONE,                       // 25
		OP_X64_MODRM,                      // 26
		OP_X64_NONE,                       // 27
		OP_X64_MODRM,                      // 28
		OP_X64_MODRM,                      // 29
		OP_X64_MODRM,                      // 2A
		OP_X64_MODRM,                      // 2B
		OP_X64_MODRM,                      // 2C
		OP_X64_MODRM,                      // 2D
		OP_X64_MODRM,                      // 2E
		OP_X64_MODRM,                      // 2F
		OP_X64_NONE,                       // 30
		OP_X64_NONE,                       // 31
		OP_X64_NONE,                       // 32
		OP_X64_NONE,                       // 33
		OP_X64_NONE,                       // 34
		OP_X64_NONE,                       // 35
		OP_X64_NONE,                       // 36
		OP_X64_NONE,                       // 37
		OP_X64_NONE,                       // 38
		OP_X64_NONE,                       // 39
		OP_X64_NONE,                       // 3A
		OP_X64_NONE,                       // 3B
		OP_X64_NONE,                       // 3C
		OP_X64_NONE,                       // 3D
		OP_X64_NONE,                       // 3E
		OP_X64_NONE,                       // 3F
		OP_X64_MODRM,                      // 40
		OP_X64_MODRM,                      // 41
		OP_X64_MODRM,                      // 42
		OP_X64_MODRM,                      // 43
		OP_X64_MODRM,                      // 44
		OP_X64_MODRM,                      // 45
		OP_X64_MODRM,                      // 46
		OP_X64_MODRM,                      // 47
		OP_X64_MODRM,                      // 48
		OP_X64_MODRM,                      // 49
		OP_X64_MODRM,                      // 4A
		OP_X64_MODRM,                      // 4B
		OP_X64_MODRM,                      // 4C
		OP_X64_MODRM,                      // 4D
		OP_X64_MODRM,                      // 4E
		OP_X64_MODRM,                      // 4F
		OP_X64_MODRM,                      // 50
		OP_X64_MODRM,                      // 51
		OP_X64_MODRM,                      // 52
		OP_X64_MODRM,                      // 53
		OP_X64_MODRM,                      // 54
		OP_X64_MODRM,                      // 55
		OP_X64_MODRM,                      // 56
		OP_X64_MODRM,                      // 57
		OP_X64_MODRM,                      // 58
		OP_X64_MODRM,                      // 59
		OP_X64_MODRM,                      // 5A
		OP_X64_MODRM,                      // 5B
		OP_X64_MODRM,                      // 5C
		OP_X64_MODRM,                      // 5D
		OP_X64_MODRM,                      // 5E
		OP_X64_MODRM,                      // 5F
		OP_X64_MODRM,                      // 60
		OP_X64_MODRM,                      // 61
		OP_X64_MODRM,                      // 62
		OP_X64_MODRM,                      // 63
		OP_X64_MODRM,                      // 64
		OP_X64_MODRM,                      // 65
		OP_X64_MODRM,                      // 66
		OP_X64_MODRM,                      // 67
		OP_X64_MODRM,                      // 68
		OP_X64_MODRM,                      // 69
		OP_X64_MODRM,                      // 6A
		OP_X64_MODRM,                      // 6B
		OP_X64_MODRM,                      // 6C
		OP_X64_MODRM,                      // 6D
		OP_X64_MODRM,                      // 6E
		OP_X64_MODRM,                      // 6F
		OP_X64_MODRM | OP_X64_DATA_I8,         // 70
		OP_X64_MODRM | OP_X64_DATA_I8,         // 71
		OP_X64_MODRM | OP_X64_DATA_I8,         // 72
		OP_X64_MODRM | OP_X64_DATA_I8,         // 73
		OP_X64_MODRM,                      // 74
		OP_X64_MODRM,                      // 75
		OP_X64_MODRM,                      // 76
		OP_X64_NONE,                       // 77
		OP_X64_NONE,                       // 78
		OP_X64_NONE,                       // 79
		OP_X64_NONE,                       // 7A
		OP_X64_NONE,                       // 7B
		OP_X64_MODRM,                      // 7C
		OP_X64_MODRM,                      // 7D
		OP_X64_MODRM,                      // 7E
		OP_X64_MODRM,                      // 7F
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 80
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 81
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 82
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 83
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 84
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 85
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 86
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 87
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 88
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 89
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 8A
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 8B
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 8C
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 8D
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 8E
		OP_X64_DATA_PRE66_67 | OP_X64_REL32,   // 8F
		OP_X64_MODRM,                      // 90
		OP_X64_MODRM,                      // 91
		OP_X64_MODRM,                      // 92
		OP_X64_MODRM,                      // 93
		OP_X64_MODRM,                      // 94
		OP_X64_MODRM,                      // 95
		OP_X64_MODRM,                      // 96
		OP_X64_MODRM,                      // 97
		OP_X64_MODRM,                      // 98
		OP_X64_MODRM,                      // 99
		OP_X64_MODRM,                      // 9A
		OP_X64_MODRM,                      // 9B
		OP_X64_MODRM,                      // 9C
		OP_X64_MODRM,                      // 9D
		OP_X64_MODRM,                      // 9E
		OP_X64_MODRM,                      // 9F
		OP_X64_NONE,                       // A0
		OP_X64_NONE,                       // A1
		OP_X64_NONE,                       // A2
		OP_X64_MODRM,                      // A3
		OP_X64_MODRM | OP_X64_DATA_I8,         // A4
		OP_X64_MODRM,                      // A5
		OP_X64_NONE,                       // A6
		OP_X64_NONE,                       // A7
		OP_X64_NONE,                       // A8
		OP_X64_NONE,                       // A9
		OP_X64_NONE,                       // AA
		OP_X64_MODRM,                      // AB
		OP_X64_MODRM | OP_X64_DATA_I8,         // AC
		OP_X64_MODRM,                      // AD
		OP_X64_MODRM,                      // AE
		OP_X64_MODRM,                      // AF
		OP_X64_MODRM,                      // B0
		OP_X64_MODRM,                      // B1
		OP_X64_MODRM,                      // B2
		OP_X64_MODRM,                      // B3
		OP_X64_MODRM,                      // B4
		OP_X64_MODRM,                      // B5
		OP_X64_MODRM,                      // B6
		OP_X64_MODRM,                      // B7
		OP_X64_NONE,                       // B8
		OP_X64_NONE,                       // B9
		OP_X64_MODRM | OP_X64_DATA_I8,         // BA
		OP_X64_MODRM,                      // BB
		OP_X64_MODRM,                      // BC
		OP_X64_MODRM,                      // BD
		OP_X64_MODRM,                      // BE
		OP_X64_MODRM,                      // BF
		OP_X64_MODRM,                      // C0
		OP_X64_MODRM,                      // C1
		OP_X64_MODRM | OP_X64_DATA_I8,         // C2
		OP_X64_MODRM,                      // C3
		OP_X64_MODRM | OP_X64_DATA_I8,         // C4
		OP_X64_MODRM | OP_X64_DATA_I8,         // C5
		OP_X64_MODRM | OP_X64_DATA_I8,         // C6
		OP_X64_MODRM,                      // C7
		OP_X64_NONE,                       // C8
		OP_X64_NONE,                       // C9
		OP_X64_NONE,                       // CA
		OP_X64_NONE,                       // CB
		OP_X64_NONE,                       // CC
		OP_X64_NONE,                       // CD
		OP_X64_NONE,                       // CE
		OP_X64_NONE,                       // CF
		OP_X64_MODRM,                      // D0
		OP_X64_MODRM,                      // D1
		OP_X64_MODRM,                      // D2
		OP_X64_MODRM,                      // D3
		OP_X64_MODRM,                      // D4
		OP_X64_MODRM,                      // D5
		OP_X64_MODRM,                      // D6
		OP_X64_MODRM,                      // D7
		OP_X64_MODRM,                      // D8
		OP_X64_MODRM,                      // D9
		OP_X64_MODRM,                      // DA
		OP_X64_MODRM,                      // DB
		OP_X64_MODRM,                      // DC
		OP_X64_MODRM,                      // DD
		OP_X64_MODRM,                      // DE
		OP_X64_MODRM,                      // DF
		OP_X64_MODRM,                      // E0
		OP_X64_MODRM,                      // E1
		OP_X64_MODRM,                      // E2
		OP_X64_MODRM,                      // E3
		OP_X64_MODRM,                      // E4
		OP_X64_MODRM,                      // E5
		OP_X64_MODRM,                      // E6
		OP_X64_MODRM,                      // E7
		OP_X64_MODRM,                      // E8
		OP_X64_MODRM,                      // E9
		OP_X64_MODRM,                      // EA
		OP_X64_MODRM,                      // EB
		OP_X64_MODRM,                      // EC
		OP_X64_MODRM,                      // ED
		OP_X64_MODRM,                      // EE
		OP_X64_MODRM,                      // EF
		OP_X64_MODRM,                      // F0
		OP_X64_MODRM,                      // F1
		OP_X64_MODRM,                      // F2
		OP_X64_MODRM,                      // F3
		OP_X64_MODRM,                      // F4
		OP_X64_MODRM,                      // F5
		OP_X64_MODRM,                      // F6
		OP_X64_MODRM,                      // F7
		OP_X64_MODRM,                      // F8
		OP_X64_MODRM,                      // F9
		OP_X64_MODRM,                      // FA
		OP_X64_MODRM,                      // FB
		OP_X64_MODRM,                      // FC
		OP_X64_MODRM,                      // FD
		OP_X64_MODRM,                      // FE
		OP_X64_NONE                        // FF
	};

	return OpcodeFlagsExt;
}



ULONG_PTR FASTCALL LdeGetOpCodeSize64(PVOID Code, PVOID *OpCodePtr = NULL)
{
	// OpCode High 5 bits as index, (1 << low 3bits) as flag

	static UCHAR PrefixTable[0x20] =
	{
		0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00
	};

	PUCHAR      Ptr;
	BOOL        PFX66, PFX67;
	BOOL        SibPresent;
	BOOL        Rex;
	ULONG_PTR   Flags;
	ULONG_PTR   Mod, RM, Reg;
	ULONG_PTR   OffsetSize, Imm64;
	ULONG_PTR   Opcode;

	Imm64 = 0;
	OffsetSize = 0;
	PFX66 = FALSE;
	PFX67 = FALSE;
	Rex = FALSE;
	Ptr = (PUCHAR)Code;

	//    while ( (*cPtr == 0x2E) || (*cPtr == 0x3E) || (*cPtr == 0x36) ||
	//            (*cPtr == 0x26) || (*cPtr == 0x64) || (*cPtr == 0x65) ||
	//            (*cPtr == 0xF0) || (*cPtr == 0xF2) || (*cPtr == 0xF3) ||
	//            (*cPtr == 0x66) || (*cPtr == 0x67) )
	while (PrefixTable[*Ptr >> 3] & (1 << (*Ptr & 7)))
	{
		PFX66 = *Ptr == 0x66;
		PFX67 = *Ptr == 0x67;
		Ptr++;
		if (Ptr > (PUCHAR)Code + 16)
			return 0;
	}

	// 0x40 ~ 0x4F
	if (((*Ptr) >> 4) == 0x4)
	{
		Rex = *Ptr & 0x0F;
		if (Rex)
			++Ptr;
	}

	Opcode = *Ptr;
	if (OpCodePtr)
		*OpCodePtr = Ptr;

	if (*Ptr == 0x0F)
	{
		Ptr++;
		Flags = GetOpCodeFlagsExt()[*Ptr];
	}
	else
	{
		ULONG_PTR tmp;

		Flags = GetOpCodeFlags()[Opcode];

		// if (Opcode >= 0xA0 && Opcode <= 0xA3)
		tmp = Opcode >> 2;
		if (tmp == 0x28)
		{
			PFX66 = PFX67;
		}
		else if (Rex && (tmp >> 1) == 0x17)     // 0xB8 ~ 0xBF  mov r64, imm64
		{
			Imm64 = 4;
		}
	}

	Ptr++;
	Ptr += FLAG_ON(Flags, OP_X64_WORD);

	if (Flags & OP_X64_MODRM)
	{
		Mod = *Ptr >> 6;
		Reg = (*Ptr & 0x38) >> 3;
		RM = *Ptr & 7;
		Ptr++;

		if ((Opcode == 0xF6) && !Reg)
			SET_FLAG(Flags, OP_X64_DATA_I8);

		if ((Opcode == 0xF7) && !Reg)
			SET_FLAG(Flags, OP_X64_DATA_PRE66_67);

		SibPresent = !PFX67 & (RM == 4);
		switch (Mod)
		{
		case 0:
			if (PFX67 && (RM == 6))
				OffsetSize = 2;
			if (!PFX67 && (RM == 5))
				OffsetSize = 4;
			break;

		case 1:
			OffsetSize = 1;
			break;

		case 2:
			OffsetSize = PFX67 ? 2 : 4;
			break;

		case 3:
			SibPresent = FALSE;
			break;
		}

		if (SibPresent)
		{
			if (((*Ptr & 7) == 5) && ((!Mod) || (Mod == 2)))
				OffsetSize = 4;

			Ptr++;
		}

		Ptr += OffsetSize;
	}

	Ptr += FLAG_ON(Flags, OP_X64_DATA_I8);
	Ptr += FLAG_ON(Flags, OP_X64_DATA_I16) ? 2 : 0;
	Ptr += FLAG_ON(Flags, OP_X64_DATA_I32) ? 4 : 0;
	Ptr += FLAG_ON(Flags, OP_X64_DATA_PRE66_67) ? (PFX66 ? 2 : 4) : 0;
	Ptr += (Rex & 9) ? Imm64 : 0;   // 0x48 || 0x49

	return PtrOffset(Ptr, Code);
}


#define NOP  0x90
#define CALL 0xE8
#define JUMP 0xE9
#define PUSH 0x68
#define REXW 0x49

NATIVE_API
NTSTATUS
NTAPI
NtProtectVirtualMemory(
IN      HANDLE      ProcessHandle,
IN OUT  PVOID*      BaseAddress,
IN OUT  PULONG_PTR  ProtectSize,
IN      ULONG       NewProtect,
OUT     PULONG      OldProtect
);

namespace Mm
{
	NTSTATUS
		ProtectVirtualMemory(
		PVOID       BaseAddress,
		ULONG_PTR   Size,
		ULONG       NewProtect,
		PULONG      OldProtect,
		HANDLE      ProcessHandle   /* = Ps::CurrentProcess */
		)
	{
		ULONG _OldProtect;
		NTSTATUS Status;

		Status = NtProtectVirtualMemory(ProcessHandle, &BaseAddress,
			&Size, NewProtect, &_OldProtect);
		if (NT_SUCCESS(Status) && OldProtect != NULL)
			*OldProtect = _OldProtect;

		return Status;
	}

	NTSTATUS
		ProtectMemory(
		HANDLE      ProcessHandle,
		PVOID       BaseAddress,
		ULONG_PTR   Size,
		ULONG       NewProtect,
		PULONG      OldProtect = nullptr
		)
	{
		return ProtectVirtualMemory(BaseAddress, Size, NewProtect, OldProtect, ProcessHandle);
	}
}


namespace Mp
{

	typedef struct
	{

#ifdef WIN32
		::ULONG_PTR EFlags;
		::ULONG_PTR Rdi;
		::ULONG_PTR Rsi;
		::ULONG_PTR Rbp;
		::ULONG_PTR Rbx;
		::ULONG_PTR Rdx;
		::ULONG_PTR Rcx;
		::ULONG_PTR Rax;
		::ULONG_PTR Rsp;
#elif _AMD64

		ULONG_PTR EFlags;
		ULONG_PTR Rax;
		ULONG_PTR Rcx;
		ULONG_PTR Rdx;
		ULONG_PTR Rbx;
		//ULONG_PTR Rsp;
		ULONG_PTR Rbp;
		ULONG_PTR Rsi;
		ULONG_PTR Rdi;

		ULONG_PTR R8;
		ULONG_PTR R9;
		ULONG_PTR R10;
		ULONG_PTR R11;
		ULONG_PTR R12;
		ULONG_PTR R13;
		ULONG_PTR R14;
		ULONG_PTR R15;

#endif // arch

		::ULONG_PTR ReturnAddress;

#ifdef WIN32

		void* GetArgument(LONG_PTR Index)
		{
			return *(void**)(this->Rsp + (Index + 1) * sizeof(this->Rsp));
		}

		template<class T>
		VOID SetArgument(LONG_PTR Index, T Value)
		{
			*(T *)(this->Rsp + (Index + 1) * sizeof(this->Rsp)) = Value;;
		}

#endif

	} TRAMPOLINE_NAKED_CONTEXT, *PTRAMPOLINE_NAKED_CONTEXT;





	template<class TYPE>
	MP_INLINE PVOID __AnyToPtr__(const TYPE &Val)
	{
		union
		{
			TYPE Val;
			PVOID Ptr;
		} u;

		u.Ptr = nullptr;
		u.Val = Val;

		return u.Ptr;
	}

	/************************************************************************
	MemoryPatch
	************************************************************************/

	MP_INLINE PATCH_MEMORY_DATA MemoryPatch(ULONG64 Data, ULONG_PTR Size, ULONG_PTR Address, ULONG Flags)
	{
		PATCH_MEMORY_DATA PatchData;

		PatchData.PatchType = PatchMemoryTypes::MemoryPatch;
		PatchData.Memory.Options.Flags = Flags;

		PatchData.Memory.Data = Data;
		PatchData.Memory.Size = Size;
		PatchData.Memory.Address = Address;

		PatchData.Memory.Backup = 0;

		return PatchData;
	}

	MP_INLINE PATCH_MEMORY_DATA MemoryPatchRva(ULONG64 Data, ULONG_PTR Size, ULONG_PTR RVA, ULONG Flags = 0)
	{
		return MemoryPatch((ULONG64)Data, Size, (ULONG_PTR)RVA, Flags);
	}

	MP_INLINE PATCH_MEMORY_DATA MemoryPatchRva(PVOID Data, ULONG_PTR Size, ULONG_PTR RVA, ULONG Flags = 0)
	{
		return MemoryPatch((ULONG64)Data, Size, (ULONG_PTR)RVA, Flags | DataIsBuffer);
	}

	template<class VA_TYPE>
	MP_INLINE PATCH_MEMORY_DATA MemoryPatchVa(ULONG64 Data, ULONG_PTR Size, VA_TYPE Address, ULONG Flags = 0)
	{
		return MemoryPatch((ULONG64)Data, Size, (ULONG_PTR)__AnyToPtr__(Address), Flags | VirtualAddress);
	}

	template<class VA_TYPE>
	MP_INLINE PATCH_MEMORY_DATA MemoryPatchVa(PVOID Data, ULONG_PTR Size, VA_TYPE Address, ULONG Flags = 0)
	{
		return MemoryPatchVa((ULONG64)Data, Size, Address, Flags | VirtualAddress | DataIsBuffer);
	}

	/************************************************************************
	FunctionPatch
	************************************************************************/

	template<class TRAMPOLINE_PTR> inline NTSTATUS RestoreMemory(TRAMPOLINE_PTR& Trampoline)
	{
		NTSTATUS Status;

		if (Trampoline == nullptr)
			return STATUS_SUCCESS;

		Status = RestoreMemory(FIELD_BASE(Trampoline, TRAMPOLINE_DATA, Trampoline));
		if (NT_SUCCESS(Status))
			Trampoline = nullptr;

		return Status;
	}


	/************************************************************************
	helper function
	************************************************************************/

	template<class PtrType>
	MP_INLINE
		PtrType GetCallDestination(PtrType Buffer)
	{
		union
		{
			PtrType     Pointer;
			ULONG_PTR   Value;
		};

		Pointer = Buffer;
		Value = *(PLONG)(Value + 1) + Value + sizeof(ULONG) + 1;

		return Pointer;
	}

	NTSTATUS
		CopyOneOpCode(
		PVOID       Target,
		PVOID       Source,
		PULONG_PTR  DestinationOpLength,
		PULONG_PTR  SourceOpLength,
		ULONG_PTR   ForwardSize,
		ULONG_PTR   BackwardSize,
		PVOID       TargetIp = IMAGE_INVALID_VA
		);

	MP_INLINE::ULONG_PTR GetOpCodeSize32(PVOID Buffer)
	{
		return LdeGetOpCodeSize32(Buffer);
	}

	MP_INLINE::ULONG_PTR GetOpCodeSize64(PVOID Buffer)
	{
		return LdeGetOpCodeSize64(Buffer);
	}

	MP_INLINE::ULONG_PTR GetOpCodeSize(PVOID Buffer)
	{
#ifdef WIN32
		return GetOpCodeSize32(Buffer);
#else
		return GetOpCodeSize64(Buffer);
#endif
	}


#ifdef WIN32

#define SIZE_OF_JUMP_CODE   5

#elif _AMD64


#endif // arch

	VOID PatchNop(PVOID Address, ULONG_PTR BytesToPatch)
	{
		PBYTE Buffer = (PBYTE)Address;

		switch (BytesToPatch)
		{
		case 0:
			return;

		case 1:
			//
			// nop
			//
			Buffer[0] = 0x90;
			break;

		case 2:
			//
			// mov eax, eax
			//
			*(PUSHORT)Buffer = 0xC08B;
			break;

		case 3:
			//
			// lea eax, [eax+0]
			//
			*(PUSHORT)Buffer = 0x408D;
			Buffer[2] = 0x00;
			break;

		case 4:
			//
			// lea esi, [esi]
			//
			*(PULONG)Buffer = 0x0026748D;
			break;

		case 5:
			// 2 + 3
			*Buffer = 0x8B;
			*(PULONG)(Buffer + 1) = 0x408DC0;
			break;

		case 6:
			// lea eax, [eax+0]
			*(PULONG)Buffer = 0x808D;
			*(PUSHORT)(Buffer + 4) = 0;
			break;

		case 7:
			// lea esi, [esi]
			*(PULONG)Buffer = 0x0026B48D;
			*(PULONG)&Buffer[3] = 0;
			break;
		}
	}

	class MemoryPatchManager
	{
	protected:
		GrowableArray<PTRAMPOLINE_DATA> TrampolineList;
		HANDLE ExecutableHeap;

	public:
		MemoryPatchManager()
		{
			this->ExecutableHeap = RtlCreateHeap(HEAP_CREATE_ENABLE_EXECUTE | HEAP_GROWABLE | HEAP_CREATE_ALIGN_16,
				nullptr, 0, 0, nullptr, nullptr);
		}

	protected:
		NTSTATUS HandleMemoryPatch(PPATCH_MEMORY_DATA PatchData, void* BaseAddress)
		{
			auto&       Patch = PatchData->Memory;
			void*       Address;
			ULONG       Protect;
			::NTSTATUS  Status;

			if (Patch.Size == 0)
				return STATUS_SUCCESS;

			if (Patch.Address == IMAGE_INVALID_RVA)
				return STATUS_SUCCESS;

			Address = PtrAdd(Patch.Options.VirtualAddress ? nullptr : BaseAddress, Patch.Address);
			Status = ProtectMemory(Address, Patch.Size, PAGE_EXECUTE_READWRITE, &Protect);
			FAIL_RETURN(Status);

			if (Patch.Options.DataIsBuffer)
			{
				CopyMemory(Address, (PVOID)Patch.Data, Patch.Size);
			}
			else
			{
				if (Patch.Options.BackupData)
				{
					PTRAMPOLINE_DATA TrampolineData;

					TrampolineData = AllocateTrampolineData();
					if (TrampolineData == nullptr)
						return STATUS_NO_MEMORY;

					TrampolineData->PatchData = *PatchData;
					TrampolineData->PatchData.Memory.Backup = 0;
					TrampolineData->PatchData.Memory.Options.VirtualAddress = TRUE;
					TrampolineData->PatchData.Memory.Address = (ULONG_PTR)Address;
					CopyMemory(&TrampolineData->PatchData.Memory.Backup, Address, Patch.Size);

					*PatchData = TrampolineData->PatchData;

					this->TrampolineList.Add(TrampolineData);
				}

				CopyMemory(&Patch.Backup, Address, Patch.Size);
				CopyMemory(Address, &Patch.Data, Patch.Size);
			}

			Status = ProtectMemory(Address, Patch.Size, Protect, nullptr);

			return STATUS_SUCCESS;
		}

		NTSTATUS HandleFunctionPatch(PPATCH_MEMORY_DATA PatchData, void* BaseAddress)
		{
			auto&               Function = PatchData->Function;
			BYTE                LocalHookBuffer[TRAMPOLINE_SIZE];
			void*               Address;
			PBYTE               Trampoline;
			ULONG               Protect;
			ULONG_PTR           HookOpSize, CopyOpSize;
			NTSTATUS            Status;
			PTRAMPOLINE_DATA    TrampolineData;

			if (Function.Target == nullptr)
				return STATUS_SUCCESS;

			if (Function.Source == IMAGE_INVALID_RVA)
				return STATUS_SUCCESS;

			Address = PtrAdd(Function.Options.VirtualAddress ? nullptr : BaseAddress, Function.Source);

			HookOpSize = GetSizeOfHookOpSize(Function.HookOp);
			if (HookOpSize == ULONG_PTR_MAX)
				return STATUS_BUFFER_TOO_SMALL;

			Status = GetHookAddressAndSize(Address, HookOpSize, &Address, &CopyOpSize);
			FAIL_RETURN(Status);

			if (CopyOpSize > TRAMPOLINE_SIZE)
				RaiseDebugBreak();

			if (Function.Options.NakedTrampoline == FALSE)
			{
				Status = GenerateHookCode(LocalHookBuffer, Address, Function.Target, Function.HookOp, HookOpSize);
				FAIL_RETURN(Status);

				PatchNop(&LocalHookBuffer[HookOpSize], CopyOpSize - HookOpSize);
			}

			TrampolineData = nullptr;
			if (Function.Options.NakedTrampoline != FALSE || (Function.Trampoline != nullptr && Function.Options.DoNotDisassemble == FALSE))
			{
				TrampolineData = AllocateTrampolineData();
				if (TrampolineData == nullptr)
					return STATUS_NO_MEMORY;

				TrampolineData->PatchData = *PatchData;
				TrampolineData->PatchData.Function.NopBytes = CopyOpSize - HookOpSize;
				TrampolineData->PatchData.Function.Source = (ULONG_PTR)Address;
				TrampolineData->PatchData.Function.Options.VirtualAddress = TRUE;

				TrampolineData->OriginSize = CopyOpSize;
				CopyMemory(TrampolineData->OriginalCode, Address, CopyOpSize);

				TrampolineData->JumpBackAddress = PtrAdd(Address, CopyOpSize);
				Trampoline = TrampolineData->Trampoline;

				if (Function.Options.NakedTrampoline == FALSE)
				{
					CopyTrampolineStub(Trampoline, Address, CopyOpSize);
					if (CopyOpSize == HookOpSize && Function.HookOp == OpCall && Function.Options.KeepRawTrampoline == FALSE)
					{
						TrampolineData->Trampoline[0] = 0xE9;
					}
					else
					{
						GenerateJumpBack(Trampoline, &TrampolineData->JumpBackAddress);
					}
				}
				else
				{
					GenerateNakedTrampoline(Trampoline, Address, CopyOpSize, TrampolineData);

					Status = GenerateHookCode(LocalHookBuffer, Address, TrampolineData->Trampoline, Function.HookOp, HookOpSize);
					if (!NT_SUCCESS(Status))
					{
						FreeTrampolineData(TrampolineData);
						return Status;
					}

					PatchNop(&LocalHookBuffer[HookOpSize], CopyOpSize - HookOpSize);
				}

				if (Trampoline - TrampolineData->Trampoline > TRAMPOLINE_SIZE)
					RaiseDebugBreak();

				FlushInstructionCache(TrampolineData->Trampoline, TRAMPOLINE_SIZE);

				if (Function.Trampoline != nullptr)
					*Function.Trampoline = TrampolineData->Trampoline;

				this->TrampolineList.Add(TrampolineData);
			}

			Status = ProtectMemory(Address, CopyOpSize, PAGE_EXECUTE_READWRITE, &Protect);
			if (!NT_SUCCESS(Status))
			{
				FreeTrampolineData(TrampolineData);
				return Status;
			}

			CopyMemory(Address, LocalHookBuffer, CopyOpSize);
			FlushInstructionCache(Address, CopyOpSize);

			Status = ProtectMemory(Address, CopyOpSize, Protect, &Protect);

			return STATUS_SUCCESS;
		}

		NTSTATUS GenerateHookCode(PBYTE Buffer, PVOID SourceIp, PVOID Target, ULONG_PTR HookOp, ULONG_PTR HookOpSize)
		{
			::ULONG_PTR RegisterIndex;

#ifdef WIN32

			switch (HookOp)
			{
			case OpCall:
			case OpJump:
				//
				// jmp imm
				//
				*Buffer++ = HookOp == OpCall ? 0xE8 : 0xE9;
				*(PULONG)Buffer = PtrOffset(Target, PtrAdd(SourceIp, HookOpSize));
				break;

			case OpPush:
				//
				// push imm
				// ret
				//
				*Buffer++ = 0x68;
				*(PULONG)Buffer = (ULONG)Target;
				Buffer += POINTER_SIZE;
				*Buffer = 0xC3;
				break;

			default:
				if (HookOp < OpJRax || HookOp > OpJRdi)
					return STATUS_INVALID_PARAMETER;

				//
				// mov r32, imm
				// jmp r32
				//
				RegisterIndex = HookOp - OpJRax;
				*Buffer++ = 0xB8 + (BYTE)RegisterIndex;
				*(PVOID *)Buffer = Target;
				Buffer += POINTER_SIZE;
				*(PUSHORT)Buffer = (USHORT)(0xE0FF + RegisterIndex);
				break;
			}

#else

			switch (HookOp)
			{
			case OpPush:
				//
				// push imm.low
				// mov dword ptr [rsp + 4], imm.high
				// ret
				//
				*Buffer++ = 0x68;
				*(PULONG)Buffer = (ULONG)((ULONG_PTR)Target >> 32);
				Buffer += POINTER_SIZE;

				*Buffer++ = 0xC7;
				*Buffer++ = 0x04;
				*Buffer++ = 0x24;
				*(PULONG)Buffer = (ULONG)(ULONG_PTR)Target;

				*Buffer = 0xC3;
				break;

			default:
				if (HookOp < OpJRax || HookOp > OpJRdi)
					return STATUS_INVALID_PARAMETER;

				//
				// mov r32, imm
				// jmp r32
				//
				RegisterIndex = HookOp - OpJRax;
				*Buffer++ = 0x48;
				*Buffer++ = 0xB8 + (BYTE)RegisterIndex;
				*(PVOID *)Buffer = Target;
				Buffer += POINTER_SIZE;
				*(PUSHORT)Buffer = (USHORT)(0xE0FF + RegisterIndex);
				break;
			}
#endif
			return STATUS_SUCCESS;
		}

		NTSTATUS GenerateNakedTrampoline(PBYTE& Trampoline, PVOID Address, ULONG_PTR CopyOpSize, PTRAMPOLINE_DATA TrampolineData)
		{
			::NTSTATUS    Status;
			PVOID*      AddressOfReturnAddress;
			auto&       Function = TrampolineData->PatchData.Function;

			if (Function.Options.ExecuteTrampoline != FALSE)
			{
				Status = CopyTrampolineStub(Trampoline, Address, CopyOpSize);
				FAIL_RETURN(Status);
			}

#ifdef WIN32

			if (Function.HookOp != OpCall)
			{
				// push eax
				*Trampoline++ = 0x50;
			}

			*Trampoline++ = 0x54;       // push    esp
			*Trampoline++ = 0x50;       // push    eax
			*Trampoline++ = 0x51;       // push    ecx
			*Trampoline++ = 0x52;       // push    edx
			*Trampoline++ = 0x53;       // push    ebx
			*Trampoline++ = 0x55;       // push    ebp
			*Trampoline++ = 0x56;       // push    esi
			*Trampoline++ = 0x57;       // push    edi
			*Trampoline++ = 0x9C;       // pushfd

			// mov eax, imm
			*Trampoline++ = 0xB8;
			*(PVOID *)Trampoline = PtrAdd(Address, CopyOpSize);
			AddressOfReturnAddress = (PVOID *)Trampoline;
			Trampoline += POINTER_SIZE;

			// mov [esp + 24h], eax
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x44;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x24;

			// mov eax, CallBack
			*Trampoline++ = 0xB8;
			*(PVOID *)Trampoline = Function.Target;
			Trampoline += POINTER_SIZE;

			// lea ecx, [esp]
			*Trampoline++ = 0x8D;
			*Trampoline++ = 0x0C;
			*Trampoline++ = 0x24;

			// call eax
			*Trampoline++ = 0xFF;
			*Trampoline++ = 0xD0;

			*Trampoline++ = 0x9D;       // popfd
			*Trampoline++ = 0x5F;       // pop     edi
			*Trampoline++ = 0x5E;       // pop     esi
			*Trampoline++ = 0x5D;       // pop     ebp
			*Trampoline++ = 0x5B;       // pop     ebx
			*Trampoline++ = 0x5A;       // pop     edx
			*Trampoline++ = 0x59;       // pop     ecx
			*Trampoline++ = 0x58;       // pop     eax
			*Trampoline++ = 0x5C;       // pop     esp

			// ret
			*Trampoline++ = 0xC3;

#else

			if (Function.HookOp != OpCall)
			{
				// push rax
				*Trampoline++ = 0x50;
			}

			// pushfq
			*Trampoline++ = 0x9C;

			// lea     rsp, [rsp-78h]
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x8D;
			*Trampoline++ = 0x64;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x88;

			// mov     [rsp + 00h], rax
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x04;
			*Trampoline++ = 0x24;

			// mov     [rsp + 08h], rcx
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x08;

			// mov     [rsp + 10h], rdx
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x54;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x10;

			// mov     [rsp + 18h], rbx
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x5C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x18;

			// mov     [rsp + 20h], rbp
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x6C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x20;

			// mov     [rsp + 28h], rsi
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x74;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x28;

			// mov     [rsp + 30h], rdi
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x7C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x30;

			// mov     [rsp + 38h], r8
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x44;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x38;

			// mov     [rsp + 40h], r9
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x40;

			// mov     [rsp + 48h], r10
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x54;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x48;

			// mov     [rsp + 50h], r11
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x5C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x50;

			// mov     [rsp + 58h], r12
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x64;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x58;

			// mov     [rsp + 60h], r13
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x6C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x60;

			// mov     [rsp + 68h], r14
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x74;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x68;

			// mov     [rsp + 70h], r15
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x7C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x70;

			// mov     rax, ReturnAddress
			*Trampoline++ = 0x48;
			*Trampoline++ = 0xB8;
			*(PVOID *)Trampoline = PtrAdd(Address, CopyOpSize);
			AddressOfReturnAddress = (PVOID *)Trampoline;
			Trampoline += POINTER_SIZE;

			// mov     [rsp + SIZE_OF_CONTEXT + 10], rax
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x89;
			*Trampoline++ = 0x84;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x80;
			*Trampoline++ = 0x00;
			*Trampoline++ = 0x00;
			*Trampoline++ = 0x00;

			// mov     rax, CallBack
			*Trampoline++ = 0x48;
			*Trampoline++ = 0xB8;
			*(PVOID *)Trampoline = Function.Target;
			Trampoline += POINTER_SIZE;

			// lea     rcx, [rsp];      rcx = PTRAMPOLINE_NAKED_CONTEXT
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x8D;
			*Trampoline++ = 0x0C;
			*Trampoline++ = 0x24;

			// lea     rsp, [rsp - 20h]
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x8D;
			*Trampoline++ = 0x64;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0xE0;

			// call    rax
			*Trampoline++ = 0xFF;
			*Trampoline++ = 0xD0;

			// lea     rsp, [rsp + 20h]
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x8D;
			*Trampoline++ = 0x64;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x20;

			// mov     rax, [rsp + 00h]
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x04;
			*Trampoline++ = 0x24;

			// mov     rcx, [rsp + 08h]
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x08;

			// mov     rdx, [rsp + 10h]
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x54;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x10;

			// mov     rbx, [rsp + 18h]
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x5C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x18;

			// mov     rbp, [rsp + 20h]
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x6C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x20;

			// mov     rsi, [rsp + 28h]
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x74;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x28;

			// mov     rdi, [rsp + 30h]
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x7C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x30;

			// mov     r8 , [rsp + 38h]
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x44;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x38;

			// mov     r9 , [rsp + 40h]
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x40;

			// mov     r10, [rsp + 48h]
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x54;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x48;

			// mov     r11, [rsp + 50h]
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x5C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x50;

			// mov     r12, [rsp + 58h]
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x64;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x58;

			// mov     r13, [rsp + 60h]
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x6C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x60;

			// mov     r14, [rsp + 68h]
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x74;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x68;

			// mov     r15, [rsp + 70h]
			*Trampoline++ = 0x4C;
			*Trampoline++ = 0x8B;
			*Trampoline++ = 0x7C;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x70;

			// lea     rsp, [rsp + SIZE_OF_CONTEXT]
			*Trampoline++ = 0x48;
			*Trampoline++ = 0x8D;
			*Trampoline++ = 0x64;
			*Trampoline++ = 0x24;
			*Trampoline++ = 0x78;

			// popfq
			*Trampoline++ = 0x9D;

			// ret
			*Trampoline++ = 0xC3;

			//#error NOT COMPLETE

#endif // arch

			if (Function.Options.ExecuteTrampoline == FALSE)
			{
				*AddressOfReturnAddress = Trampoline;

				Status = CopyTrampolineStub(Trampoline, Address, CopyOpSize);
				FAIL_RETURN(Status);

				Status = GenerateJumpBack(Trampoline, &TrampolineData->JumpBackAddress);
				FAIL_RETURN(Status);
			}

			return STATUS_SUCCESS;
		}

		NTSTATUS GenerateJumpBack(PBYTE& Trampoline, PVOID AddressOfJumpAddress)
		{
			// jmp [imm32]
			*Trampoline++ = 0xFF;
			*Trampoline++ = 0x25;

			*(PVOID *)Trampoline = AddressOfJumpAddress;
			Trampoline += 2 + 4;

			return STATUS_SUCCESS;
		}

		NTSTATUS CopyTrampolineStub(PBYTE& Trampoline, PVOID Address, ULONG_PTR CopyOpSize)
		{
			ULONG_PTR SourceOpLength, ForwardSize, BackwardSize;
			PBYTE Source = (PBYTE)Address;

			ForwardSize = CopyOpSize;
			BackwardSize = 0;

			for (LONG_PTR Bytes = CopyOpSize; Bytes > 0; Bytes -= SourceOpLength)
			{
				ULONG_PTR TargetOpLength;

				if (Source[0] == 0xC2 || Source[0] == 0xC3)
					return STATUS_BUFFER_TOO_SMALL;

				CopyOneOpCode(Trampoline, Source, &TargetOpLength, &SourceOpLength, ForwardSize, BackwardSize);

				BackwardSize += SourceOpLength;
				ForwardSize -= SourceOpLength;
				Trampoline += TargetOpLength;
				Source += SourceOpLength;
			}

			return STATUS_SUCCESS;
		}

		NTSTATUS GetHookAddressAndSize(PVOID Address, ULONG_PTR HookOpSize, PVOID *FinalAddress, PULONG_PTR Size)
		{
			::ULONG_PTR Length, OpSize;
			PBYTE       Buffer;

			OpSize = 0;
			Buffer = (PBYTE)Address;
			*FinalAddress = Address;

#if  defined(WIN32) || defined(_AMD64)

			while (OpSize < HookOpSize)
			{
				if (Buffer[0] == 0xC2 || Buffer[0] == 0xC3)
					return STATUS_BUFFER_TOO_SMALL;

				Length = GetOpCodeSize(Buffer);

				// jmp short const
				if (Buffer[0] == 0xEB)
				{
					if (OpSize == 0)
					{
						if (Buffer[1] != 0)
						{
							Buffer += *(PCHAR)&Buffer[1] + Length;
							*FinalAddress = Buffer;
							continue;
						}
					}
					else if (OpSize < HookOpSize - Length)
					{
						return STATUS_BUFFER_TOO_SMALL;
					}
				}
				else if (Buffer[0] == 0xFF && Buffer[1] == 0x25)
				{
					// jmp [rimm]

#ifdef WIN32
					if (OpSize != 0 && OpSize + Length < HookOpSize)
						return STATUS_BUFFER_TOO_SMALL;

					if (OpSize == 0 && HookOpSize > Length)
					{
						Buffer = **(PBYTE **)&Buffer[2];
						*FinalAddress = Buffer;
						continue;
					}
#else
					if (OpSize != 0)
						return STATUS_BUFFER_TOO_SMALL;

					Buffer = *(PBYTE *)(Buffer + Length + *(PLONG)&Buffer[2]);
					*FinalAddress = Buffer;
					continue;
#endif // arch

				}

				Buffer += Length;
				OpSize += Length;
			}

#else

#error "not support"

#endif // arch

			*Size = OpSize;

			return STATUS_SUCCESS;
		}

		ULONG_PTR GetSizeOfHookOpSize(ULONG_PTR HookOp)
		{

#ifdef _AMD64

			switch (HookOp)
			{
			case OpPush:
				// 68 00 00 00 80       push    80000000h
				// C7 04 24 00 00 00 80 mov     dword ptr [rsp], 80000000h
				// C3                   ret
				return 0xD;

				//case OpJumpIndirect:
				//    //
				//    // FF 25 00 00 00 00 jmp [rimm]
				//    // 00 00 00 00 00 00 00 00
				//    //
				//    break;
				//    //return 0xE;

			default:
				if (HookOp >= OpJRax && HookOp <= OpJRdi)
				{
					//
					// 48 B8 00 00 00 80 00 00 00 80    mov     rax, 8000000080000000h
					// FF E0                            jmp     rax
					//
					return 0xC;
				}
			}

#else

			switch (HookOp)
			{
			case OpPush:
				//
				// 68 00000000  push const
				// C3           ret
				//
				return 1 + POINTER_SIZE + 1;

			case OpCall:
			case OpJump:
				//
				// E8 00000000  call    const
				// E9 00000000  jmp     const
				//
				return 1 + POINTER_SIZE;

			default:
				if (HookOp >= OpJRax && HookOp <= OpJRdi)
				{
					//
					// b8 88888888      mov     r32, const
					// ffe0             jmp     r32
					//
					return 1 + POINTER_SIZE + 2;
				}
			}

#endif

			return ULONG_PTR_MAX;
		}

		FORCEINLINE
			NTSTATUS
			ProtectMemory(
			PVOID       BaseAddress,
			ULONG_PTR   Size,
			ULONG       NewProtect,
			PULONG      OldProtect
			)
		{
			return Mm::ProtectMemory(CurrentProcess, BaseAddress, Size, NewProtect, OldProtect);
		}

		FORCEINLINE
			NTSTATUS
			FlushInstructionCache(
			PVOID       BaseAddress,
			ULONG_PTR   NumberOfBytesToFlush
			)
		{
			return NtFlushInstructionCache(CurrentProcess, BaseAddress,
				NumberOfBytesToFlush);
		}

		PTRAMPOLINE_DATA AllocateTrampolineData()
		{
			return (PTRAMPOLINE_DATA)RtlAllocateHeap(this->ExecutableHeap,
				HEAP_ZERO_MEMORY, sizeof(TRAMPOLINE_DATA));
		}

		VOID FreeTrampolineData(PTRAMPOLINE_DATA TrampolineData)
		{
			if (TrampolineData != nullptr)
				RtlFreeHeap(this->ExecutableHeap, 0, TrampolineData);
		}

		NTSTATUS RemoveTrampolineData(PTRAMPOLINE_DATA TrampolineData)
		{
			NTSTATUS Status;
			ULONG_PTR Index;

			Index = this->TrampolineList.IndexOf(TrampolineData);
			if (Index == this->TrampolineList.kInvalidIndex)
				return STATUS_NOT_FOUND;

			this->TrampolineList.Remove(Index);
			FreeTrampolineData(TrampolineData);

			return STATUS_SUCCESS;
		}

		VOID RaiseDebugBreak()
		{
			__debugbreak();
		}

	public:
		NTSTATUS PatchMemory(PPATCH_MEMORY_DATA PatchData, ULONG_PTR PatchCount, PVOID BaseAddress)
		{
			NTSTATUS Status;

			FOR_EACH(PatchData, PatchData, PatchCount)
			{
				switch (PatchData->PatchType)
				{
				case PatchMemoryTypes::MemoryPatch:
					Status = HandleMemoryPatch(PatchData, BaseAddress);
					break;

				case PatchMemoryTypes::FunctionPatch:
					Status = HandleFunctionPatch(PatchData, BaseAddress);
					break;

				default:
					Status = STATUS_NOT_IMPLEMENTED;
					break;
				}

				FAIL_RETURN(Status);
			}

			return STATUS_SUCCESS;
		}

		template<class TRAMPOLINE_PTR> inline NTSTATUS RestoreMemory(TRAMPOLINE_PTR& Trampoline)
		{
			NTSTATUS Status;

			if (Trampoline == nullptr)
				return STATUS_SUCCESS;

			Status = RestoreMemory(FIELD_BASE(Trampoline, TRAMPOLINE_DATA, Trampoline));
			if (NT_SUCCESS(Status))
				Trampoline = nullptr;

			return Status;
		}

		NTSTATUS RestoreMemory(PTRAMPOLINE_DATA TrampolineData)
		{
			NTSTATUS Status;
			PATCH_MEMORY_DATA PatchData = MemoryPatchVa(&TrampolineData->OriginalCode, TrampolineData->OriginSize, TrampolineData->PatchData.Function.Source);
			Status = HandleMemoryPatch(&PatchData, nullptr);
			NT_SUCCESS(Status) && RemoveTrampolineData(TrampolineData);

			return Status;
		}

	public:
		static NTSTATUS CreateInstance(MemoryPatchManager **Manager)
		{
			MlInitialize();

			*Manager = (MemoryPatchManager *)AllocateMemory(sizeof(**Manager), HEAP_ZERO_MEMORY);
			if (*Manager == nullptr)
				return STATUS_NO_MEMORY;

			ZeroMemory(*Manager, sizeof(**Manager));
			new (*Manager) MemoryPatchManager;

			return STATUS_SUCCESS;
		}

		static NTSTATUS DestroyInstance(MemoryPatchManager *Manager)
		{
			if (Manager != nullptr)
			{
				Manager->~MemoryPatchManager();
				FreeMemory(Manager);
			}

			return STATUS_SUCCESS;
		}
	};

	FORCEINLINE MemoryPatchManager*& MemoryPatchManagerInstance()
	{
		static MemoryPatchManager *Manager;
		return Manager;
	}

	NTSTATUS MP_CALL PatchMemory(PPATCH_MEMORY_DATA PatchData, ULONG_PTR PatchCount, PVOID BaseAddress)
	{
		NTSTATUS Status;

		if (MemoryPatchManagerInstance() == nullptr)
		{
			Status = MemoryPatchManager::CreateInstance(&MemoryPatchManagerInstance());
			FAIL_RETURN(Status);
		}

		return MemoryPatchManagerInstance()->PatchMemory(PatchData, PatchCount, BaseAddress);
	}

	NTSTATUS RestoreMemory(PPATCH_MEMORY_DATA PatchData, ::ULONG_PTR PatchCount)
	{
		FOR_EACH(PatchData, PatchData, PatchCount)
		{
			switch (PatchData->PatchType)
			{
			case PatchMemoryTypes::MemoryPatch:
				break;

			case PatchMemoryTypes::FunctionPatch:
				if (PatchData->Function.Trampoline != nullptr)
					RestoreMemory(*PatchData->Function.Trampoline);
				break;
			}
		}

		return STATUS_SUCCESS;
	}

	NTSTATUS MP_CALL RestoreMemory(PTRAMPOLINE_DATA TrampolineData)
	{
		if (MemoryPatchManagerInstance() == nullptr)
			return STATUS_FLT_NOT_INITIALIZED;

		return MemoryPatchManagerInstance()->RestoreMemory(TrampolineData);
	}
}

namespace Mp
{

	NTSTATUS
		CopyOneOpCode(
		PVOID       Target,
		PVOID       Source,
		PULONG_PTR  TargetOpLength,
		PULONG_PTR  SourceOpLength,
		ULONG_PTR   ForwardSize,
		ULONG_PTR   BackwardSize,
		PVOID       TargetIp
		)
	{
		ULONG_PTR   Length, OpCode, OpCodeLength, OpOffsetLength, Selector;
		LONG_PTR    OpOffset, *pOpOffset;
		PBYTE       Func, p;
		ULONG_PTR   NextOpAddress;

		enum { OpCodeOffsetShort = 1, OpCodeOffsetLong = POINTER_SIZE };

		TargetIp = TargetIp == IMAGE_INVALID_VA ? Target : TargetIp;

		Func = (PBYTE)Source;
		p = (PBYTE)Target;

		Length = GetOpCodeSize(Func);

		OpCode = *Func;
		pOpOffset = (PLONG_PTR)(Func + 1);
		OpOffsetLength = OpCodeOffsetShort;
		OpCodeLength = 2;

#ifdef WIN32
		Selector = 0xFFFFFFFFu;
#else
		Selector = 0xFFFFFFFFFFFFFFFFu;
#endif

		if (((OpCode & 0xF0) ^ 0x70) == 0)
			//    if (OpCode >= 0x70 && OpCode <= 0x7F)
		{
			OpCode = 0x800F | ((OpCode - 0x70) << 8);
		}
		else if (OpCode == 0xEB)
		{
			OpCode = 0xE9;
			OpCodeLength = 1;
		}
		else if (OpCode == 0xE8 || OpCode == 0xE9)
		{
			OpOffsetLength = OpCodeOffsetLong;
			OpCodeLength = 1;
		}
		else
		{
			ULONG _Op;

			OpCode = *(PUSHORT)Func;
			_Op = SWAP2(OpCode);

			if ((_Op & ~0xF) == 0x0F80)
				// if (_Op >= 0x0F80 && _Op <= 0x0F8F)
			{
				OpOffsetLength = OpCodeOffsetLong;
				pOpOffset = (PLONG_PTR)(Func + 2);
			}
			else
			{
			DEFAULT_OP_CODE:
				CopyMemory(p, Func, Length);
				p += Length;

				goto EXIT_PROC;
			}
		}

		OpOffset = 0;
		switch (OpOffsetLength)
		{
		case OpCodeOffsetShort:
			OpOffset = *(PCHAR)pOpOffset;
			break;

		case OpCodeOffsetLong:
		default:
			OpOffset = *(PLONG_PTR)pOpOffset;
			break;
		}


		LONG_PTR NewOffset = Length + OpOffset;

		if (NewOffset < 0)
		{
			if ((ULONG_PTR)-NewOffset < BackwardSize)
				goto DEFAULT_OP_CODE;
		}
		else if ((ULONG_PTR)NewOffset < ForwardSize)
		{
			goto DEFAULT_OP_CODE;
		}

		NextOpAddress = (ULONG_PTR)Func + Length;
		OpOffset = (NextOpAddress + OpOffset) - ((ULONG_PTR)TargetIp + OpCodeLength + OpCodeOffsetLong + (Selector == -1 ? 0 : 2));
		switch (OpCodeLength)
		{
		case 1:
			*p++ = (BYTE)OpCode;
			break;

		case 2:
		default:
			*(PUSHORT)p = (USHORT)OpCode;
			p += 2;
			break;
		}
		/*
		if (Selector != -1)
		{
		*(PUSHORT)p = Selector;
		p += 2;
		}
		*/
		*(PLONG_PTR)p = OpOffset;
		p += POINTER_SIZE;

		//    Length = PtrOffset(p, Destination);

	EXIT_PROC:
		if (SourceOpLength != NULL)
			*SourceOpLength = Length;

		if (TargetOpLength != NULL)
			*TargetOpLength = PtrOffset(p, Target);

		return STATUS_SUCCESS;
	}
}
