#pragma once

#include "my.h"

template<class PtrType> ForceInline PtrType Nt_EncodePointer(PtrType Pointer, ULONG_PTR Cookie)
{
	return (PtrType)_rotr((ULONG_PTR)PtrXor(Pointer, Cookie), Cookie & 0x1F);
}

template<class PtrType> ForceInline PtrType Nt_DecodePointer(PtrType Pointer, ULONG_PTR Cookie)
{
	return (PtrType)PtrXor(_rotl((ULONG_PTR)Pointer, Cookie & 0x1F), Cookie);
}

