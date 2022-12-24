#pragma once

#include <Windows.h>
#include <ntstatus.h>


#define LOOP_ONCE for(ULONG_PTR LoopCount = 0; LoopCount < 1; LoopCount++)

#define _TAG2(s) ((((s) << 8) | ((s) >> 8)) & 0xFFFF)
#define TAG2(s) _TAG2((USHORT)(s))

#define _TAG3(s) ( \
                (((s) >> 16) & 0xFF)       | \
                (((s)        & 0xFF00))    | \
                (((s) << 16) & 0x00FF0000) \
                )
#define TAG3(s) _TAG3((DWORD)(s))

#define _TAG4(s) ( \
                (((s) >> 24) & 0xFF)       | \
                (((s) >> 8 ) & 0xFF00)     | \
                (((s) << 24) & 0xFF000000) | \
                (((s) << 8 ) & 0x00FF0000) \
                )
#define TAG4(s) _TAG4((DWORD)(s))

#define TYPE_OF decltype

#define FUNC_POINTER(__func) TYPE_OF(__func)*
#define API_POINTER(__func) TYPE_OF(&__func)
#define PTR_OF(_var) TYPE_OF(_var)*


FORCEINLINE BOOL IsStatusSuccess(NTSTATUS Status) { return (Status & Status) >= 0; }
#define NT_SUCCESS(Status)  IsStatusSuccess(Status)


