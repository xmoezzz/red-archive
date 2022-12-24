#pragma once
#include "NtDefine.h"

#ifndef NOVTABLE
#define NOVTABLE __declspec(novtable)
#endif


#define MM_NOT_FREE     0x80000000

class NOVTABLE MemoryAllocator
{
protected:


	HANDLE             m_hHeap;
	static HANDLE       m_hHeapGlobal;
	static ULONG_PTR    m_ObjectCount;


protected:

	static HANDLE CreateGlobalHeapInternal(DWORD flOptions = 0, SIZE_T dwInitialSize = 0, SIZE_T dwMaximumSize = 0)
	{
		if (m_hHeapGlobal == NULL)
			m_hHeapGlobal = CreateHeapInternal(flOptions, dwInitialSize, dwMaximumSize);

		return m_hHeapGlobal;
	}

	FORCEINLINE static ULONG_PTR AddRef()
	{
		return ++m_ObjectCount;
		//return Interlocked_Increment((PLong)&m_ObjectCount);
	}

	FORCEINLINE static ULONG_PTR Release()
	{
		return --m_ObjectCount;
		//return Interlocked_Decrement((PLong)&m_ObjectCount);
	}

	BOOL DestroyGlobal()
	{
		if (m_hHeap == NULL)
			return FALSE;

		return DestroyGlobalHeap();
	}

	FORCEINLINE BOOL IsHeapPrivate()
	{
		return m_hHeap != NULL && m_hHeap != m_hHeapGlobal;
	}


public:
	MemoryAllocator(HANDLE hHeap = NULL)
	{

		if (hHeap != NULL)
		{
			//            m_hHeapPrivate = hHeap;
		}
		else
		{
			if (AddRef() == 1)
			{
				hHeap = CreateGlobalHeapInternal();
				if (hHeap == NULL)
					Release();
			}
			else
			{
				hHeap = m_hHeapGlobal;
			}

			//            m_hHeapPrivate = NULL;
		}

		m_hHeap = hHeap;
	}

	MemoryAllocator(const MemoryAllocator &mem)
	{
		*this = mem;
	}

	~MemoryAllocator()
	{

		if (IsHeapPrivate())
		{
			if (IsNotProcessHeap())
				DestroyHeapInternal(m_hHeap);
		}
		else
		{
			DestroyGlobal();
		}
	}

	HANDLE GetHeap() const
	{
		return m_hHeap;
	}

	static HANDLE CreateGlobalHeap(ULONG Options = 0)
	{
		HANDLE hHeap = CreateGlobalHeapInternal(Options);
		if (hHeap != NULL)
			AddRef();
		return hHeap;
	}

	static BOOL DestroyGlobalHeap()
	{
		if (m_hHeapGlobal != NULL && Release() == 0)
		{
			if (DestroyHeapInternal(m_hHeapGlobal))
			{
				m_hHeapGlobal = NULL;
				return TRUE;
			}
		}

		return FALSE;
	}

	static HANDLE GetGlobalHeap()
	{
		return m_hHeapGlobal;
	}

	static PVOID GetAddressOfGlobalHeap()
	{
		return &m_hHeapGlobal;
	}

	static PVOID AllocateMemory(ULONG_PTR Size, ULONG Flags = 0)
	{
		return AllocateHeapInternal(GetGlobalHeap(), Flags, Size);
	}

	static PVOID ReAllocateMemory(PVOID Memory, ULONG_PTR Size, ULONG Flags = 0)
	{
		PVOID Block = ReAllocateHeapInternal(GetGlobalHeap(), Flags, Memory, Size);

		if (Block == NULL && Memory != NULL)
		{
			FreeMemory(Memory);
		}

		return Block;
	}

	static BOOL FreeMemory(PVOID Memory, ULONG Flags = 0)
	{
		return FreeHeapInternal(GetGlobalHeap(), Flags, Memory);
	}

	HANDLE CreateHeap(DWORD flOptions = 0, SIZE_T dwInitialSize = 0, SIZE_T dwMaximumSize = 0)
	{
		if (IsHeapPrivate())
			DestroyHeapInternal(m_hHeap);
		else
			DestroyGlobal();

		m_hHeap = CreateHeapInternal(flOptions, dwInitialSize, dwMaximumSize);

		return m_hHeap;
	}

	BOOL DestroyHeap()
	{
		BOOL Result = TRUE;

		if (IsHeapPrivate() && IsNotProcessHeap())
		{
			Result = DestroyHeapInternal(m_hHeap);
			if (Result)
				m_hHeap = NULL;
		}

		return Result;
	}


	PVOID Alloc(SIZE_T Size, ULONG_PTR Flags = 0)
	{
		return AllocateHeapInternal(m_hHeap, Flags, Size);
	}

	PVOID ReAlloc(PVOID pBuffer, SIZE_T Size, ULONG_PTR Flags = 0)
	{
		PVOID pRealloc;

		if (pBuffer == NULL)
			return Alloc(Size, Flags);

		pRealloc = ReAllocateHeapInternal(m_hHeap, Flags, pBuffer, Size);
		if (!FLAG_ON(Flags, MM_NOT_FREE))
		{
			if (pRealloc == NULL)
			{
				Free(pBuffer);
			}
		}

		return pRealloc;
	}

	BOOL Free(PVOID pBuffer, ULONG_PTR Flags = 0)
	{
		return pBuffer == NULL ? FALSE : FreeHeapInternal(m_hHeap, Flags, pBuffer);
	}

	BOOL SafeFree(LPVOID pBuffer, ULONG_PTR Flags = 0)
	{
		LPVOID **pt = (LPVOID **)pBuffer;
		if (*pt == NULL)
			return FALSE;

		BOOL Result = FreeHeapInternal(m_hHeap, Flags, *pt);
		if (Result)
			*pt = NULL;

		return Result;
	}

private:

	BOOL IsNotProcessHeap()
	{
		return m_hHeap != GetProcessHeap();
	}

	static HANDLE CreateHeapInternal(ULONG Flags = 0, SIZE_T CommitSize = 0, SIZE_T ReserveSize = 0)
	{
		return HeapCreate(Flags, CommitSize, ReserveSize);
	}

	static BOOL DestroyHeapInternal(HANDLE hHeap)
	{
		return HeapDestroy(hHeap);
	}

	static LONG_PTR ModifyAllocCount(LONG_PTR Increment)
	{
		static LONG_PTR AllocCount;

		AllocCount += Increment;
		return AllocCount;
	}


	static PVOID AllocateHeapInternal(HANDLE Heap, ULONG_PTR Flags, SIZE_T Size)
	{
		PVOID Memory = RtlAllocateHeap(Heap, (ULONG)Flags, Size);

		return Memory;
	}

	static PVOID ReAllocateHeapInternal(HANDLE Heap, ULONG_PTR Flags, PVOID Memory, SIZE_T Size)
	{
		return Memory == NULL ? AllocateHeapInternal(Heap, Flags, Size) :
			RtlReAllocateHeap(Heap, (ULONG)Flags, Memory, Size);
	}

	static BOOL FreeHeapInternal(HANDLE Heap, ULONG_PTR Flags, PVOID Memory)
	{
		if (Memory == NULL)
			return TRUE;

		return RtlFreeHeap(Heap, (ULONG)Flags, Memory);
	}
};
