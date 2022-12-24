#include "SetInternalError.h"


LONG WINAPI CallbackFilter(_EXCEPTION_POINTERS*   excp)
{
	MessageBoxW(0, L"Internal Error", L"X'moe Core Error", MB_OK);
	return   EXCEPTION_EXECUTE_HANDLER;
}

VOID WINAPI SetFilter()
{
	SetUnhandledExceptionFilter(CallbackFilter);
}
