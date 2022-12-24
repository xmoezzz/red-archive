#include "IStreamExFile.h"

IStreamAdapterFile::IStreamAdapterFile(NtFileDisk *ref)
{
	Stream = ref;
	RefCount = 1;
}
//---------------------------------------------------------------------------
IStreamAdapterFile::~IStreamAdapterFile()
{
	delete Stream;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterFile::QueryInterface(REFIID riid,
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
ULONG STDMETHODCALLTYPE IStreamAdapterFile::AddRef(void)
{
	return ++RefCount;
}
//---------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE IStreamAdapterFile::Release(void)
{
	if (RefCount == 1)
	{
		Stream->Close();
		delete this;
		return 0;
	}
	else
	{
		return --RefCount;
	}
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterFile::Read(void *pv, ULONG cb, ULONG *pcbRead)
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
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterFile::Write(const void *pv, ULONG cb,
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
HRESULT STDMETHODCALLTYPE IStreamAdapterFile::Seek(LARGE_INTEGER dlibMove,
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
HRESULT STDMETHODCALLTYPE IStreamAdapterFile::SetSize(ULARGE_INTEGER libNewSize)
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterFile::CopyTo(IStream *pstm, ULARGE_INTEGER cb,
	ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterFile::Commit(DWORD grfCommitFlags)
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterFile::Revert(void)
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterFile::LockRegion(ULARGE_INTEGER libOffset,
	ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterFile::UnlockRegion(ULARGE_INTEGER libOffset,
	ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE IStreamAdapterFile::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
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
		pstatstg->cbSize.QuadPart = Stream->GetSize64();

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
HRESULT STDMETHODCALLTYPE IStreamAdapterFile::Clone(IStream **ppstm)
{
	return E_NOTIMPL;
}
