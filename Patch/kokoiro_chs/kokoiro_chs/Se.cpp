#include "Se.h"

static HANDLE MainThread = 0;

HANDLE GetMainThread()
{
	return MainThread;
}

VOID SetMainThread(HANDLE Handle)
{
	MainThread = Handle;
}


THREAD_START_PARAMETER*
AllocateThreadParameter(
PVOID StartAddress,
PVOID Parameter
)
{
	THREAD_START_PARAMETER *StartParameter;

	StartParameter = (THREAD_START_PARAMETER *)AllocateMemoryP(sizeof(*StartParameter));

	StartParameter->Context = THREAD_START_PARAMETER_MAGIC;
	StartParameter->Parameter = Parameter;
	StartParameter->ThreadStartRoutine = StartAddress;

	return StartParameter;
}

BOOL
FreeThreadParameter(
THREAD_START_PARAMETER *Parameter
)
{
	return FreeMemoryP(Parameter);
}
