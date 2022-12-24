#pragma once

#include "StreamHolder.h"
#include "tp_stub.h"
#include <Windows.h>
#include <Objidl.h>

class IStreamAdapter : public IStream
{
private:
	StreamHolder *Stream;
	ULONG RefCount;

public:
	IStreamAdapter(StreamHolder *ref);
	~IStreamAdapter();

	// IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
		void **ppvObject);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);

	// ISequentialStream
	HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead);
	HRESULT STDMETHODCALLTYPE Write(const void *pv, ULONG cb,
		ULONG *pcbWritten);

	// IStream
	HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove,
		DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
	HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize);
	HRESULT STDMETHODCALLTYPE CopyTo(IStream *pstm, ULARGE_INTEGER cb,
		ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
	HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags);
	HRESULT STDMETHODCALLTYPE Revert(void);
	HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset,
		ULARGE_INTEGER cb, DWORD dwLockType);
	HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset,
		ULARGE_INTEGER cb, DWORD dwLockType);
	HRESULT STDMETHODCALLTYPE Stat(STATSTG *pstatstg, DWORD grfStatFlag);
	HRESULT STDMETHODCALLTYPE Clone(IStream **ppstm);

	void ClearStream()
	{
		Stream = NULL;
	}
};

