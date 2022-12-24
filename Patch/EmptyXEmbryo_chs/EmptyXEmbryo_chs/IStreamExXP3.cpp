#include "IStreamExXP3.h"

IStreamAdapterXP3::IStreamAdapterXP3(StreamHolderXP3 *ref)
{
	Stream = ref;
	RefCount = 1;
}
//---------------------------------------------------------------------------
IStreamAdapterXP3::~IStreamAdapterXP3()
{
	delete Stream;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterXP3::QueryInterface(REFIID riid,
	void **ppvObject)
{
	if (!ppvObject) return E_INVALIDARG;

	*ppvObject = NULL;
	if (!memcmp(&riid, &IID_IUnknown, 16))
		*ppvObject = (IUnknown*)this;
	else if (!memcmp(&riid, &IID_ISequentialStream, 16))
		*ppvObject = (ISequentialStream*)this;
	else if (!memcmp(&riid, &IID_IStream, 16))
		*ppvObject = (IStream*)this;

	if (*ppvObject)
	{
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}
//---------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE IStreamAdapterXP3::AddRef(void)
{
	return ++RefCount;
}
//---------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE IStreamAdapterXP3::Release(void)
{
	if (RefCount == 1)
	{
		delete this;
		return 0;
	}
	else
	{
		return --RefCount;
	}
}

#include "Se.h"


inline PLDR_MODULE GetKrMovieLdr()
{
	LDR_MODULE *Ldr, *FirstLdr;

	Ldr = FIELD_BASE(Nt_CurrentPeb()->Ldr->InInitializationOrderModuleList.Flink, LDR_MODULE, InInitializationOrderLinks);
	FirstLdr = Ldr;

	do
	{
		Ldr = FIELD_BASE(Ldr->InInitializationOrderLinks.Flink, LDR_MODULE, InInitializationOrderLinks);
		if (Ldr->BaseDllName.Buffer == NULL)
			continue;

		if (CHAR_UPPER4W(*(PULONG64)(Ldr->BaseDllName.Buffer + 0)) != TAG4W('KRMO') ||
			CHAR_UPPER4W(*(PULONG64)(Ldr->BaseDllName.Buffer + 4)) != CHAR_UPPER4W(TAG4W('VIE.')))
		{
			continue;
		}

		return Ldr;

	} while (FirstLdr != Ldr);

	return NULL;
}


inline PLDR_MODULE GetModuleLdr(PVOID ModuleBase)
{
	LDR_MODULE *Ldr, *FirstLdr;

	Ldr = FIELD_BASE(Nt_CurrentPeb()->Ldr->InInitializationOrderModuleList.Flink, LDR_MODULE, InInitializationOrderLinks);
	FirstLdr = Ldr;

	do
	{
		Ldr = FIELD_BASE(Ldr->InInitializationOrderLinks.Flink, LDR_MODULE, InInitializationOrderLinks);
		if (Ldr->BaseDllName.Buffer == NULL)
			continue;

		if (Ldr->DllBase != ModuleBase)
			continue;

		return Ldr;

	} while (FirstLdr != Ldr);

	return NULL;
}


BOOL IsValidDllWorkerThread(PVOID ThreadStart)
{
	PVOID       LayerExMovieHandle;
	PVOID       KrMovieHandle;
	PLDR_MODULE LayerExMovieLdr, KrMovieLdr;
	BOOL        Result;

	LayerExMovieHandle = Nt_GetModuleHandle(L"layerExMovie.dll");
	KrMovieHandle      = Nt_GetModuleHandle(L"krmovie.dll");

	if (LayerExMovieHandle == NULL && KrMovieHandle == NULL)
		return FALSE;

	LOOP_ONCE
	{
		Result = FALSE;
		if (LayerExMovieHandle)
		{
			LayerExMovieLdr = GetModuleLdr(LayerExMovieHandle);
			if (!LayerExMovieLdr)
				break;

			if (ThreadStart < LayerExMovieLdr->DllBase || ThreadStart >((PBYTE)LayerExMovieLdr->DllBase + LayerExMovieLdr->SizeOfImage))
				break;

			Result = TRUE;
		}
	}

	if (Result)
		return Result;

	LOOP_ONCE
	{
		if (KrMovieHandle)
		{
			KrMovieLdr = GetModuleLdr(KrMovieHandle);
			if (!KrMovieLdr)
				break;

			if (ThreadStart >= KrMovieLdr->DllBase || ThreadStart <= ((PBYTE)LayerExMovieLdr->DllBase + LayerExMovieLdr->SizeOfImage))
				return TRUE;
		}
	}
	return FALSE;
}

BOOL IsThreadValid()
{
	NTSTATUS                Status;
	PVOID                   pvStart;
	PLDR_MODULE             Ldr;
	THREAD_START_PARAMETER *Parameter;

	pvStart = NULL;
	Ldr = GetModuleLdr(Nt_GetExeModuleHandle());
	if (Ldr == NULL)
		return FALSE;

	Status = NtQueryInformationThread(Nt_CurrentTeb()->ClientId.UniqueThread, ThreadQuerySetWin32StartAddress, &pvStart, sizeof(pvStart), NULL);
	if (NT_FAILED(Status))
		return FALSE;

	if (pvStart < Ldr->DllBase || pvStart > PtrAnd(Ldr->DllBase, Ldr->SizeOfImage))
		return IsValidDllWorkerThread(pvStart);

	//PrintConsoleW(L"New worker thread...\n");
	Parameter = AllocateThreadParameter(NULL, (PVOID)((ULONG)NtAddAtom ^ (ULONG)Nt_CurrentPeb()));
	RtlPushFrame(Parameter);
	return TRUE;
}

//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterXP3::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	//PrintConsole(L"%x, %x - %x\n", _ReturnAddress(), GetMainThread(), Nt_CurrentTeb()->ClientId.UniqueThread);

	THREAD_START_PARAMETER *Parameter = (THREAD_START_PARAMETER *)Nt_FindThreadFrameByContext(THREAD_START_PARAMETER_MAGIC);
	if (Parameter == NULL && IsThreadValid() == FALSE)
	{	
		PrintConsoleW(L"clear worker thread...\n");
		Stream->Destroy();
	}


	try
	{
		ULONG read;
		read = (ULONG)Stream->Read(pv, cb);
		if (pcbRead) *pcbRead = read;
	}
	catch (...)
	{
		return E_FAIL;
	}
	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterXP3::Write(const void *pv, ULONG cb,
	ULONG *pcbWritten)
{
	try
	{
		ULONG written;
		written = (ULONG)Stream->Write((PVOID)pv, cb);
		if (pcbWritten) *pcbWritten = written;
	}
	catch (...)
	{
		return E_FAIL;
	}
	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterXP3::Seek(LARGE_INTEGER dlibMove,
	DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
	try
	{
		switch (dwOrigin)
		{
		case STREAM_SEEK_SET:
			if (plibNewPosition)
				(*plibNewPosition).QuadPart =
				Stream->Seek(dlibMove.QuadPart, TJS_BS_SEEK_SET);
			else
				Stream->Seek(dlibMove.QuadPart, TJS_BS_SEEK_SET);
			break;
		case STREAM_SEEK_CUR:
			if (plibNewPosition)
				(*plibNewPosition).QuadPart =
				Stream->Seek(dlibMove.QuadPart, TJS_BS_SEEK_CUR);
			else
				Stream->Seek(dlibMove.QuadPart, TJS_BS_SEEK_CUR);
			break;
		case STREAM_SEEK_END:
			if (plibNewPosition)
				(*plibNewPosition).QuadPart =
				Stream->Seek(dlibMove.QuadPart, TJS_BS_SEEK_END);
			else
				Stream->Seek(dlibMove.QuadPart, TJS_BS_SEEK_END);
			break;
		default:
			return E_FAIL;
		}
	}
	catch (...)
	{
		return E_FAIL;
	}
	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterXP3::SetSize(ULARGE_INTEGER libNewSize)
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterXP3::CopyTo(IStream *pstm, ULARGE_INTEGER cb,
	ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterXP3::Commit(DWORD grfCommitFlags)
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterXP3::Revert(void)
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterXP3::LockRegion(ULARGE_INTEGER libOffset,
	ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterXP3::UnlockRegion(ULARGE_INTEGER libOffset,
	ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterXP3::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
	// This method imcompletely fills the target structure, because some
	// informations like access mode or stream name are already lost
	// at this point.
	
	if (pstatstg)
	{
		ZeroMemory(pstatstg, sizeof(*pstatstg));

		// pwcsName
		// this object's storage pointer does not have a name ...
		if (!(grfStatFlag &  STATFLAG_NONAME))
		{
			// anyway returns an empty string
			LPWSTR str = (LPWSTR)CoTaskMemAlloc(sizeof(*str));
			if (str == NULL) return E_OUTOFMEMORY;
			*str = L'\0';
			pstatstg->pwcsName = str;
		}

		// type
		pstatstg->type = STGTY_STREAM;

		// cbSize
		pstatstg->cbSize.QuadPart = Stream->GetSize();

		// mtime, ctime, atime unknown

		// grfMode unknown
		pstatstg->grfMode = STGM_DIRECT | STGM_READWRITE | STGM_SHARE_DENY_WRITE;
		// Note that this method always returns flags above, regardless of the
		// actual mode.
		// In the return value, the stream is to be indicated that the
		// stream can be written, but of cource, the Write method will fail
		// if the stream is read-only.

		// grfLockSuppoted
		pstatstg->grfLocksSupported = 0;

		// grfStatBits unknown
	}
	else
	{
		return E_INVALIDARG;
	}

	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterXP3::Clone(IStream **ppstm)
{
	return E_NOTIMPL;
}
