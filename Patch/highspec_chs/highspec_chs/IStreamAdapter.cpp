#include "IStreamAdapter.h"

IStreamAdapter::IStreamAdapter(StreamHolder *ref)
{
	Stream = ref;
	RefCount = 1;
}

IStreamAdapter::~IStreamAdapter()
{
	delete Stream;
}

HRESULT STDMETHODCALLTYPE IStreamAdapter::QueryInterface(REFIID riid,
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
ULONG STDMETHODCALLTYPE IStreamAdapter::AddRef(void)
{
	return ++RefCount;
}
//---------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE IStreamAdapter::Release(void)
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

HRESULT STDMETHODCALLTYPE IStreamAdapter::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
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


HRESULT STDMETHODCALLTYPE IStreamAdapter::Write(const void *pv, ULONG cb,
	ULONG *pcbWritten)
{
	try
	{
		ULONG written;
		written = (ULONG)Stream->Write(pv, cb);
		if (pcbWritten) *pcbWritten = written;
	}
	catch (...)
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT STDMETHODCALLTYPE IStreamAdapter::Seek(LARGE_INTEGER dlibMove,
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

HRESULT STDMETHODCALLTYPE IStreamAdapter::SetSize(ULARGE_INTEGER libNewSize)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IStreamAdapter::CopyTo(IStream *pstm, ULARGE_INTEGER cb,
	ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IStreamAdapter::Commit(DWORD grfCommitFlags)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE IStreamAdapter::Revert(void)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE IStreamAdapter::LockRegion(ULARGE_INTEGER libOffset,
	ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE IStreamAdapter::UnlockRegion(ULARGE_INTEGER libOffset,
	ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE IStreamAdapter::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
	if (pstatstg)
	{
		RtlZeroMemory(pstatstg, sizeof(*pstatstg));

		if (!(grfStatFlag &  STATFLAG_NONAME))
		{
			LPWSTR str = (LPWSTR)CoTaskMemAlloc(sizeof(*str));

			if (str == NULL) 
				return E_OUTOFMEMORY;

			*str = L'\0';
			pstatstg->pwcsName = str;
		}
		pstatstg->type              = STGTY_STREAM;
		pstatstg->cbSize.QuadPart   = Stream->GetSize();
		pstatstg->grfMode           = STGM_DIRECT | STGM_READWRITE | STGM_SHARE_DENY_WRITE;
		pstatstg->grfLocksSupported = 0;
	}
	else
	{
		return E_INVALIDARG;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE IStreamAdapter::Clone(IStream **ppstm)
{
	return E_NOTIMPL;
}
