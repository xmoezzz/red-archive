#include "ShinkuHook.h"
#include "ecrypt-sync.h"
#include "py.h"
#include "Init2ndKey.h"
#include "Init1stKey.h"

extern "C"
{
#include "phelix.h"
}

NTSTATUS ShinkuHook::InitAddtion()
{
	NTSTATUS Status;

	LOOP_ONCE
	{
		Status = STATUS_UNSUCCESSFUL;
		ECRYPT_init();
		PhelixInit();
		PyECRYPT_init();
		InitAll1StKey();
		InitAll2ndKey();

		Status = STATUS_SUCCESS;
	}
	return Status;
}

