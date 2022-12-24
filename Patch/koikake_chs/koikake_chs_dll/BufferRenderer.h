/****************************************************************************/
/*! @file
@brief DirectShow

�o�b�t�@�w�����_�����O����
-----------------------------------------------------------------------------
Copyright (C) 2004 T.Imoto
-----------------------------------------------------------------------------
@author		T.Imoto
@date		2004/08/05
@note
2004/08/05	T.Imoto
*****************************************************************************/

#ifndef __VIDEO_BUFFER_RENDER_H__
#define __VIDEO_BUFFER_RENDER_H__

#include <streams.h>
#include "IBufferRenderer.h"
#include "IRendererBufferAccess.h"
#include "IRendererBufferVideo.h"

#define EC_UPDATE		(EC_USER+1)

class TBufferRendererAllocator;
class TBufferRendererInputPin;
class TBufferRenderer;

//----------------------------------------------------------------------------
//! @brief Buffer Renderer�̃A���P�[�^�[
//----------------------------------------------------------------------------
class TBufferRendererAllocator : public CBaseAllocator
{
private:
	CMediaSample	*m_pMediaSample;	//!< ���݂̃��f�B�A�T���v���ւ̃|�C���^
	TBufferRenderer	*m_pRenderer;		//!< ���̃A���P�[�^�[���������_�[�ւ̃|�C���^

protected:
	void Free(void);
	HRESULT Alloc(void);

public:
	TBufferRendererAllocator(TBufferRenderer *, TCHAR *, LPUNKNOWN, HRESULT *);
	TBufferRendererAllocator(TCHAR *, LPUNKNOWN, HRESULT *);
	virtual ~TBufferRendererAllocator();

	// �I�[�o�[���C�h
	STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual);

	// �_�u���o�b�t�@�����O�p�|�C���^�����ւ����\�b�h
	void SetPointer(IMediaSample *media, BYTE *ptr);
	void SetPointer(BYTE *ptr) { SetPointer(m_pMediaSample, ptr); };
};

//----------------------------------------------------------------------------
//! @brief Buffer Renderer�̃C���v�b�g�s��
//----------------------------------------------------------------------------
class TBufferRendererInputPin : public CRendererInputPin
{
private:
	TBufferRenderer	*m_pRenderer;		//!< ���̃s�����������_�[�ւ̃|�C���^
	CCritSec		*m_pInterfaceLock;	//!< ���b�N�p���\�[�X
	bool			m_ActiveAllocator;	//!< ���̃s�����������_�[�̃A���P�[�^�[���g���Ă��邩�ǂ���

public:
	TBufferRendererInputPin(TBufferRenderer *pRenderer, CCritSec *pInterfaceLock, HRESULT *phr, LPCWSTR name);
	virtual ~TBufferRendererInputPin();
	bool ActiveAllocator(void) const;

	//�I�[�o�[���C�h
	STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);
	STDMETHODIMP NotifyAllocator(IMemAllocator * pAllocator, BOOL bReadOnly);

	// �_�u���o�b�t�@�����O�p�|�C���^�����ւ����\�b�h
	void SetPointer(IMediaSample *media, BYTE *ptr);
	void SetPointer(BYTE *ptr);
};

//----------------------------------------------------------------------------
//! @brief �o�b�t�@�w�����_�����O����
//----------------------------------------------------------------------------
class TBufferRenderer : public CBaseVideoRenderer, public IRendererBufferAccess, public IRendererBufferVideo
{
	friend class TBufferRendererInputPin;
	friend class TBufferRendererAllocator;

private:
	long	m_VideoWidth;		//!< �r�f�I�̕�
	long	m_VideoHeight;		//!< �r�f�I�̍���
	long	m_VideoPitch;		//!< �r�f�I�̃s�b�`(1�s�̃o�C�g��)
	BYTE	*m_Buffer[2];		//!< �����_�����O����o�b�t�@�ւ̃|�C���^
	bool	m_IsBufferOwner[2];	//!< �o�b�t�@�����̃N���X�Ɋ��蓖�Ă�ꂽ���̂��ǂ�����ێ�
	int		m_FrontBuffer;		//!< ���݂̃t�����g�o�b�t�@���ǂ��炩��ێ�
	LONG	m_StartFrame;
	LONG	m_StopFrame;

	TBufferRendererInputPin		m_InputPin;		//!< ���̓s��
	TBufferRendererAllocator	m_Allocator;	//!< �������̊��蓖��

	CCritSec		m_BufferLock;	//!< �o�b�t�@�փA�N�Z�X���鎞�Ƀ��b�N����
	CMediaType		m_MtIn;			//!< Source connection media type

public:
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN, HRESULT *);
	TBufferRenderer(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr);
	~TBufferRenderer();

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	HRESULT OnStartStreaming(void);
	void OnRenderStart(IMediaSample *pMediaSample);

	// IRendererBufferAccess
	STDMETHOD(SetFrontBuffer)(BYTE *buff, long *size);
	STDMETHOD(SetBackBuffer) (BYTE *buff, long *size);
	STDMETHOD(GetFrontBuffer)(BYTE **buff, long *size);
	STDMETHOD(GetBackBuffer) (BYTE **buff, long *size);

	// IRendererBufferVideo
	virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_AvgTimePerFrame(
		/* [retval][out] */ REFTIME *pAvgTimePerFrame);
	virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_VideoWidth(
		/* [retval][out] */ long *pVideoWidth);
	virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_VideoHeight(
		/* [retval][out] */ long *pVideoHeight);

protected:
	// �I�[�o�[���C�h
	// DShow������ɃR�[������
	HRESULT CheckMediaType(const CMediaType *pmt);		// Format acceptable?
	HRESULT DoRenderSample(IMediaSample *pMediaSample);	// New video sample
	HRESULT SetMediaType(const CMediaType *pmt);			// Video format notification

	void SwapBuffer(IMediaSample *pSample);

	void AllocFrontBuffer(size_t size);
	void AllocBackBuffer(size_t size);

	void FreeFrontBuffer();
	void FreeBackBuffer();

	bool IsAllocatedFrontBuffer() { return ((m_FrontBuffer == 1) ? (m_Buffer[1] != NULL) : (m_Buffer[0] != NULL)); }
	bool IsAllocatedBackBuffer() { return ((m_FrontBuffer == 1) ? (m_Buffer[0] != NULL) : (m_Buffer[1] != NULL)); }

	long GetBufferSize() { return (m_VideoHeight * m_VideoPitch); }

	void SetFrontBuffer(BYTE *buff);
	void SetBackBuffer(BYTE *buff);
	BYTE *GetFrontBuffer();
	BYTE *GetBackBuffer();

	//! �_�u���o�b�t�@�����O�p�|�C���^�����ւ����\�b�h
	void SetPointer(IMediaSample *media, BYTE *ptr)
	{
		m_InputPin.SetPointer(media, ptr);
	}
	void SetPointer(BYTE *ptr)
	{
		m_InputPin.SetPointer(ptr);
	}
};


#endif	// __VIDEO_BUFFER_RENDER_H__
