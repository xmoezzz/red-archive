#pragma once

#include "my.h"

#define THREAD_START_PARAMETER_MAGIC    TAG4('TSPM')

struct THREAD_START_PARAMETER : public TEB_ACTIVE_FRAME
{
	PVOID ThreadStartRoutine;
	PVOID Parameter;

	THREAD_START_PARAMETER()
	{
		Context = THREAD_START_PARAMETER_MAGIC;
	}
};

HANDLE GetMainThread();
VOID SetMainThread(HANDLE Handle);

THREAD_START_PARAMETER*
AllocateThreadParameter(
PVOID StartAddress,
PVOID Parameter
);

BOOL
FreeThreadParameter(
THREAD_START_PARAMETER *Parameter
);
