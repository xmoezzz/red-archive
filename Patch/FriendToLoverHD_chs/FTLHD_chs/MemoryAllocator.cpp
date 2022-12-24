#include "MemoryAllocator.h"

HANDLE       MemoryAllocator::m_hHeapGlobal = NULL;
ULONG_PTR    MemoryAllocator::m_ObjectCount = 0;

