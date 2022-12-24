#pragma once

#include <Windows.h>
#include "NtDefine.h"

#define MP_INLINE FORCEINLINE
#define MP_CALL   FASTCALL

#ifdef WIN32
#define TRAMPOLINE_SIZE     0x40
#elif _AMD64
#define TRAMPOLINE_SIZE     0x100
#endif


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

namespace Mp
{
	enum
	{
		VirtualAddress = 0x00000001,

		// patch
		BackupData = 0x00000002,
		DataIsBuffer = 0x00000004,

		// function
		DoNotDisassemble = 0x00000002,
		NakedTrampoline = 0x00000004,
		KeepRawTrampoline = 0x00000008,
		ExecuteTrampoline = 0x00000010,

		OpMask = 0xF0000000,
		OpJump = 0x00000000,
		OpCall = 0x10000000,
		OpPush = 0x20000000,
		OpJRax = 0x30000000,
		OpJRcx = 0x40000000,
		OpJRdx = 0x50000000,
		OpJRbx = 0x60000000,
		OpJRbp = 0x70000000,
		OpJRsi = 0x80000000,
		OpJRdi = 0x90000000,
		OpJR10 = 0xA0000000,
	};


	namespace PatchMemoryTypes
	{

		enum
		{
			MemoryPatch,
			FunctionPatch,
		};

	}


	typedef struct
	{
		::ULONG_PTR PatchType;

		union
		{
			struct
			{
				union
				{
					ULONG Flags;
					struct
					{
						BOOLEAN VirtualAddress : 1;
						BOOLEAN BackupData : 1;
						BOOLEAN DataIsBuffer : 1;
					};
				} Options;

				ULONG64       Data;
				::ULONG_PTR   Size;
				::ULONG_PTR   Address;
				ULONG64       Backup;

			} Memory;

			struct
			{
				union
				{
					ULONG Flags;
					struct
					{
						BOOLEAN VirtualAddress : 1;
						BOOLEAN DoNotDisassemble : 1;
						BOOLEAN NakedTrampoline : 1;
						BOOLEAN KeepRawTrampoline : 1;
						BOOLEAN ExecuteTrampoline : 1;
					};
				} Options;

				::ULONG_PTR   HookOp;
				::ULONG_PTR   Source;
				PVOID         Target;
				PVOID*        Trampoline;
				::ULONG_PTR   NopBytes;

			} Function;
		};

	} PATCH_MEMORY_DATA, *PPATCH_MEMORY_DATA;

	typedef struct _TRAMPOLINE_DATA
	{
		BYTE                Trampoline[TRAMPOLINE_SIZE];
		BYTE                OriginalCode[TRAMPOLINE_SIZE];
		ULONG               TrampolineSize;
		ULONG               OriginSize;
		PVOID               JumpBackAddress;
		PATCH_MEMORY_DATA   PatchData;

	} TRAMPOLINE_DATA, *PTRAMPOLINE_DATA;

	MP_INLINE PATCH_MEMORY_DATA FunctionPatch(PVOID Source, PVOID Target, PVOID Trampoline, ULONG Flags)
	{
		PATCH_MEMORY_DATA PatchData;

		PatchData.PatchType = PatchMemoryTypes::FunctionPatch;
		PatchData.Function.Options.Flags = Flags;
		PatchData.Function.HookOp = Flags & OpMask;

		PatchData.Function.Source = (ULONG_PTR)Source;
		PatchData.Function.Target = Target;
		PatchData.Function.Trampoline = (PVOID *)Trampoline;
		PatchData.Function.NopBytes = 0;

		return PatchData;
	}

	template<class SOURCE_TYPE, class TARGET_TYPE>
	MP_INLINE PATCH_MEMORY_DATA FunctionJumpVa(SOURCE_TYPE Source, TARGET_TYPE Target, PVOID Trampoline = nullptr, ULONG Flags = OpJump)
	{
		return FunctionPatch(__AnyToPtr__(Source), __AnyToPtr__((SOURCE_TYPE)Target), Trampoline, Flags | VirtualAddress);
	}

	template<class TARGET_TYPE>
	MP_INLINE PATCH_MEMORY_DATA FunctionJumpRva(ULONG_PTR SourceRva, TARGET_TYPE Target, PVOID Trampoline = nullptr, ULONG Flags = OpJump)
	{
		return FunctionPatch((PVOID)SourceRva, __AnyToPtr__(Target), Trampoline, Flags);
	}

	template<class SOURCE_TYPE, class TARGET_TYPE>
	MP_INLINE PATCH_MEMORY_DATA FunctionCallVa(SOURCE_TYPE Source, TARGET_TYPE Target, PVOID Trampoline = nullptr, ULONG Flags = OpCall)
	{
		return FunctionPatch(__AnyToPtr__(Source), __AnyToPtr__((SOURCE_TYPE)Target), Trampoline, Flags | VirtualAddress);
	}

	template<class TARGET_TYPE>
	MP_INLINE PATCH_MEMORY_DATA FunctionCallRva(ULONG_PTR SourceRva, TARGET_TYPE Target, PVOID Trampoline = nullptr, ULONG Flags = OpCall)
	{
		return FunctionPatch((PVOID)SourceRva, __AnyToPtr__(Target), Trampoline, Flags);
	}

	NTSTATUS
		MP_CALL
		PatchMemory(
		PPATCH_MEMORY_DATA  PatchData,
		ULONG_PTR           PatchCount,
		PVOID               BaseAddress = nullptr
		);

	NTSTATUS
		MP_CALL
		RestoreMemory(
		PTRAMPOLINE_DATA TrampolineData
		);

	NTSTATUS RestoreMemory(PPATCH_MEMORY_DATA PatchData, ::ULONG_PTR PatchCount);
}
