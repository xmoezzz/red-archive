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

#include <streams.h>
#include <atlbase.h>
#include "BufferRenderer.h"

#ifdef _DEBUG
#include <stdio.h>
#include "DShowException.h"
#endif

//----------------------------------------------------------------------------
//##	TBufferRenderer
//----------------------------------------------------------------------------
//! @brief	  	This goes in the factory template table to create new filter instances
//! @param		pUnk : �W������ IUnknown �C���^�[�t�F�C�X�ւ̃|�C���^�B
//! @param		phr : ���\�b�h�̐����E���s������ HRESULT �l���󂯎��ϐ��ւ̃|�C���^�B
//----------------------------------------------------------------------------
CUnknown * WINAPI TBufferRenderer::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
	CUnknown	*punk = new TBufferRenderer(NAME("Buffer Renderer"), pUnk, phr);
	if (punk == NULL)
		*phr = E_OUTOFMEMORY;
	return punk;
}
#pragma warning(disable: 4355)	// �R���X�g���N�^�̃x�[�X�����o����������this���g���ƃ��[�j���O���o��̂ł����}�~
//----------------------------------------------------------------------------
//! @brief	  	TBufferRenderer constructor
//! @param		pName : �f�o�b�O�̂��߂Ɏg�p�����L�q�ւ̃|�C���^�B
//! @param		pUnk : �W�����ꂽ���L�҃I�u�W�F�N�g�ւ̃|�C���^�B
//! @param		phr : HRESULT �l�ւ̃|�C���^�B
//----------------------------------------------------------------------------
TBufferRenderer::TBufferRenderer(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr)
	: CBaseVideoRenderer(CLSID_BufferRenderer, pName, pUnk, phr)
	, m_InputPin(this, &m_InterfaceLock, phr, L"Input")
	, m_Allocator(this, NAME("Allocator"), GetOwner(), phr)
{
	//CBaseRender::m_pInputPin�Ƀ|�C���^��ݒ肷��B
	m_pInputPin = &m_InputPin;

	// Store and AddRef the texture for our use.
	*phr = S_OK;
	m_Buffer[0] = NULL;
	m_Buffer[1] = NULL;

	m_IsBufferOwner[0] = false;
	m_IsBufferOwner[1] = false;

	m_FrontBuffer = 0;

	m_StartFrame = 0;
}
#pragma warning(default: 4355)
//----------------------------------------------------------------------------
//! @brief	  	TBufferRenderer destructor
//----------------------------------------------------------------------------
TBufferRenderer::~TBufferRenderer()
{
	//CBaseRender::m_pInputPin�Ƀ|�C���^�����Z�b�g����B
	//��������Ȃ���CBaseRender�̃f�X�g���N�^��delete����Ă��܂��̂Œ��ӁI
	m_pInputPin = NULL;

	// �����Ŋm�ۂ��Ă���ꍇ�o�b�t�@�̉��
	FreeFrontBuffer();
	FreeBackBuffer();
}
//----------------------------------------------------------------------------
//! @brief	  	�v�����ꂽ�C���^�[�t�F�C�X��Ԃ�
//! 
//! Overriden to say what interfaces we support and where
//! @param		riid : �C���^�[�t�F�C�X��IID
//! @param		ppv : �C���^�[�t�F�C�X��Ԃ��|�C���^�[�ւ̃|�C���^
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
STDMETHODIMP TBufferRenderer::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	CheckPointer(ppv, E_POINTER);
	if (riid == IID_IRendererBufferAccess) {
		*ppv = static_cast<IRendererBufferAccess*>(this);
		static_cast<IUnknown*>(*ppv)->AddRef();
		return S_OK;
	}
	else if (riid == IID_IRendererBufferVideo) {
		*ppv = static_cast<IRendererBufferVideo*>(this);
		static_cast<IUnknown*>(*ppv)->AddRef();
		return S_OK;
	}
	return CBaseVideoRenderer::NonDelegatingQueryInterface(riid, ppv);
}
//----------------------------------------------------------------------------
//! @brief	  	����̃��f�B�A �^�C�v���t�B���^���󂯓���邩�ǂ������m�F����
//! 
//! This method forces the graph to give us an R8G8B8 video type, making our copy 
//! to texture memory trivial.
//! @param		pmt : ��Ă��ꂽ���f�B�A �^�C�v���܂� CMediaType �I�u�W�F�N�g�ւ̃|�C���^
//! @return		��Ă��ꂽ���f�B�A �^�C�v���󂯓������Ȃ� S_OK ��Ԃ��B
//!				�����łȂ���� S_FALSE ���G���[ �R�[�h��Ԃ��B
//----------------------------------------------------------------------------
HRESULT TBufferRenderer::CheckMediaType(const CMediaType *pmt)
{
	HRESULT		hr = E_FAIL;
	VIDEOINFO	*pvi;

	// Reject the connection if this is not a video type
	if (*pmt->FormatType() != FORMAT_VideoInfo)
		return E_INVALIDARG;

	// Only accept RGB32
	pvi = (VIDEOINFO *)pmt->Format();
	if (IsEqualGUID(*pmt->Type(), MEDIATYPE_Video))
	{
		if (IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_RGB32) ||
			IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_ARGB32))
		{
			hr = S_OK;
			m_MtIn = (*pmt);
		}
	}

	return hr;
}
//----------------------------------------------------------------------------
//! @brief	  	Graph connection has been made. 
//! @param		pmt : ���f�B�A �^�C�v���w�肷�� CMediaType �I�u�W�F�N�g�ւ̃|�C���^
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
HRESULT TBufferRenderer::SetMediaType(const CMediaType *pmt)
{
	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����

	// Retrive the size of this media type
	VIDEOINFO *pviBmp;						// Bitmap info header
	pviBmp = (VIDEOINFO *)pmt->Format();
	m_VideoWidth = pviBmp->bmiHeader.biWidth;
	m_VideoHeight = abs(pviBmp->bmiHeader.biHeight);
	m_VideoPitch = m_VideoWidth * 4;	// RGB32�Ɍ��ߑł�

	if (!IsAllocatedFrontBuffer())
		AllocFrontBuffer(GetBufferSize());

	if (!IsAllocatedBackBuffer())
		AllocBackBuffer(GetBufferSize());

	return S_OK;
}
//----------------------------------------------------------------------------
//! @brief	  	A sample has been delivered. Copy it to the texture.
//! @param		pSample : �T���v���� IMediaSample �C���^�[�t�F�C�X�ւ̃|�C���^
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
HRESULT TBufferRenderer::DoRenderSample(IMediaSample * pSample)
{
	DWORD	*pBmpBuffer, *pTxtBuffer;	// Bitmap buffer, texture buffer
	BYTE	*pTxtOrgPos;

	//	if( m_bEOS ) return S_OK;

	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����

	// Get the video bitmap buffer
	pSample->GetPointer(reinterpret_cast<BYTE**>(&pBmpBuffer));

	// Get the texture buffer & pitch
	pTxtBuffer = reinterpret_cast<DWORD*>(GetBackBuffer());
	pTxtOrgPos = reinterpret_cast<BYTE*>(pTxtBuffer);

	HRESULT		hr;
	LONG		EventParam1 = -1;
	LONGLONG	TimeStart = 0;
	LONGLONG	TimeEnd = 0;

	if (SUCCEEDED(hr = pSample->GetMediaTime(&TimeStart, &TimeEnd)))
	{
		EventParam1 = (LONG)TimeStart;
	}
	if (m_StopFrame && EventParam1 >= m_StopFrame)
		return S_OK;	// �Đ����Ȃ��t���[��

	if (pTxtBuffer == pBmpBuffer)	// ���O�̃A���P�[�^�[���g���Ă���
	{
		SwapBuffer(pSample);	// Front��Back�o�b�t�@�����ւ���
		if (m_pSink)
			m_pSink->Notify(EC_UPDATE, EventParam1, NULL);
		return S_OK;
	}

	// ���O�̃A���P�[�^�[�ł͂Ȃ��̂Ń��������R�s�[����
#if 0
	// �������ɃR�s�[(�㉺���]��)
	{
		int		height = m_VideoHeight;
		int		width = m_VideoWidth;
		pBmpBuffer += width * (height - 1);
		for (int j = 0; j < height; j++)
		{
			for (int i = 0; i < width; i++)
			{
				pTxtBuffer[i] = pBmpBuffer[i];
			}
			pBmpBuffer -= width;
			pTxtBuffer += width;
		}
	}
#else
	// �ォ�牺�ɃR�s�[
	{
		int		height = m_VideoHeight;
		int		width = m_VideoWidth;
		for (int j = 0; j < height; j++)
		{
			for (int i = 0; i < width; i++)
			{
				pTxtBuffer[i] = pBmpBuffer[i];
			}
			pBmpBuffer += width;
			pTxtBuffer += width;
		}
	}
#endif
	if (m_pSink)
		m_pSink->Notify(EC_UPDATE, EventParam1, NULL);
	SwapBuffer(pSample);	// Front��Back�o�b�t�@�����ւ���
	return S_OK;
}
//---------------------------------------------------------------------------
//! @brief	  	�t�����g�o�b�t�@�ƃo�b�N�o�b�t�@�����ւ���
//! @param		pSample : �T���v���B���̒��̃|�C���^��ύX����
//----------------------------------------------------------------------------
void TBufferRenderer::SwapBuffer(IMediaSample *pSample)
{
	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����
	if (m_FrontBuffer == 1)
	{
		SetPointer(pSample, m_Buffer[1]);
		m_FrontBuffer = 0;
	}
	else
	{
		SetPointer(pSample, m_Buffer[0]);
		m_FrontBuffer = 1;
	}
}
//---------------------------------------------------------------------------
//! @brief	  	�t�����g�o�b�t�@�Ƀ����������蓖�Ă�
//! @param		size : ���蓖�Ă�T�C�Y
//----------------------------------------------------------------------------
void TBufferRenderer::AllocFrontBuffer(size_t size)
{
	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����
	BYTE	*buff = NULL;

	FreeFrontBuffer();
	if (m_FrontBuffer == 1)
	{
		buff = m_Buffer[1] = reinterpret_cast<BYTE*>(CoTaskMemAlloc(size));
		m_IsBufferOwner[1] = true;
	}
	else
	{
		buff = m_Buffer[0] = reinterpret_cast<BYTE*>(CoTaskMemAlloc(size));
		m_IsBufferOwner[0] = true;
	}

	if (buff == NULL)
		throw L"Cannot allocate memory in filter.";
}
//---------------------------------------------------------------------------
//! @brief	  	�o�b�N�o�b�t�@�Ƀ����������蓖�Ă�B
//! @param		size : ���蓖�Ă�T�C�Y
//----------------------------------------------------------------------------
void TBufferRenderer::AllocBackBuffer(size_t size)
{
	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����
	BYTE	*buff = NULL;

	FreeBackBuffer();
	if (m_FrontBuffer == 1)
	{
		buff = m_Buffer[0] = reinterpret_cast<BYTE*>(CoTaskMemAlloc(size));
		m_IsBufferOwner[0] = true;
	}
	else
	{
		buff = m_Buffer[1] = reinterpret_cast<BYTE*>(CoTaskMemAlloc(size));
		m_IsBufferOwner[1] = true;
	}

	if (buff == NULL)
		throw L"Cannot allocate memory in filter.";
}
//---------------------------------------------------------------------------
//! @brief	  	�t�����g�o�b�t�@�Ɋ��蓖�Ă��Ă��郁�������J������
//!
//! �����A���蓖�Ă��Ă��郁�������A���̃N���X�ɂ���Ċ��蓖�Ă�ꂽ���̂łȂ��ꍇ�́A
//! ������Ȃ��B
//----------------------------------------------------------------------------
void TBufferRenderer::FreeFrontBuffer()
{
	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����
	if (m_FrontBuffer == 1)
	{
		if (m_Buffer[1] != NULL)
		{
			if (m_IsBufferOwner[1])
				CoTaskMemFree(m_Buffer[1]);
			m_Buffer[1] = NULL;
		}
		m_IsBufferOwner[1] = false;
	}
	else
	{
		if (m_Buffer[0] != NULL)
		{
			if (m_IsBufferOwner[0])
				CoTaskMemFree(m_Buffer[0]);
			m_Buffer[0] = NULL;
		}
		m_IsBufferOwner[0] = false;
	}
}
//---------------------------------------------------------------------------
//! @brief	  	�o�b�N�o�b�t�@�Ɋ��蓖�Ă��Ă��郁�������J������
//!
//! �����A���蓖�Ă��Ă��郁�������A���̃N���X�ɂ���Ċ��蓖�Ă�ꂽ���̂łȂ��ꍇ�́A
//! ������Ȃ��B
//----------------------------------------------------------------------------
void TBufferRenderer::FreeBackBuffer()
{
	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����
	if (m_FrontBuffer == 1)
	{
		if (m_Buffer[0] != NULL)
		{
			if (m_IsBufferOwner[0])
				CoTaskMemFree(m_Buffer[0]);
			m_Buffer[0] = NULL;
		}
		m_IsBufferOwner[0] = false;
	}
	else
	{
		if (m_Buffer[1] != NULL)
		{
			if (m_IsBufferOwner[1])
				CoTaskMemFree(m_Buffer[1]);
			m_Buffer[1] = NULL;
		}
		m_IsBufferOwner[1] = false;
	}
}
//---------------------------------------------------------------------------
//! @brief	  	�t�����g�o�b�t�@�Ƀo�b�t�@�ւ̃|�C���^��ݒ肷��
//! @param		buff : �o�b�t�@�ւ̃|�C���^
//----------------------------------------------------------------------------
void TBufferRenderer::SetFrontBuffer(BYTE *buff)
{
	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����
	FreeFrontBuffer();
	if (m_FrontBuffer == 1)
		m_Buffer[1] = buff;
	else
		m_Buffer[0] = buff;
}
//---------------------------------------------------------------------------
//! @brief	  	�o�b�N�o�b�t�@�Ƀo�b�t�@�ւ̃|�C���^��ݒ肷��
//! @param		buff : �o�b�t�@�ւ̃|�C���^
//----------------------------------------------------------------------------
void TBufferRenderer::SetBackBuffer(BYTE *buff)
{
	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����
	FreeBackBuffer();
	if (m_FrontBuffer == 1)
		m_Buffer[0] = buff;
	else
		m_Buffer[1] = buff;

	SetPointer(buff);
}
//---------------------------------------------------------------------------
//! @brief	  	�t�����g�o�b�t�@�ւ̃|�C���^���擾����
//! @return		�o�b�t�@�ւ̃|�C���^
//----------------------------------------------------------------------------
BYTE *TBufferRenderer::GetFrontBuffer()
{
	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����
	if (m_FrontBuffer == 1)
		return m_Buffer[1];
	else
		return m_Buffer[0];
}
//---------------------------------------------------------------------------
//! @brief	  	�o�b�N�o�b�t�@�ւ̃|�C���^���擾����
//! @return		�o�b�t�@�ւ̃|�C���^
//----------------------------------------------------------------------------
BYTE *TBufferRenderer::GetBackBuffer()
{
	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����
	if (m_FrontBuffer == 1)
		return m_Buffer[0];
	else
		return m_Buffer[1];
}
//----------------------------------------------------------------------------
//! @brief	  	�t�����g�o�b�t�@��ݒ肵�܂��B
//! @param		buff : �t�����g�o�b�t�@�p�o�b�t�@�ւ̃|�C���^
//! @param		size : �o�b�t�@�̃T�C�Y��n���ϐ��ւ̃|�C���^�B@n
//!					buff��NULL�̎��A�����ɗ~�����T�C�Y���Ԃ�
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
HRESULT TBufferRenderer::SetFrontBuffer(BYTE *buff, long *size)
{
	if (m_State == State_Running)
		return S_FALSE;

	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����
	if (buff == NULL && size != NULL)
	{
		*size = GetBufferSize();
		return S_OK;
	}
	if (buff == NULL || size == NULL)
		return E_POINTER;

	if ((*size) != GetBufferSize())
		return E_INVALIDARG;

	FreeFrontBuffer();
	SetFrontBuffer(buff);
	return S_OK;
}
//----------------------------------------------------------------------------
//! @brief	  	�o�b�N�o�b�t�@��ݒ肵�܂��B
//! @param		buff : �o�b�N�o�b�t�@�p�o�b�t�@�ւ̃|�C���^
//! @param		size : �o�b�t�@�̃T�C�Y��n���ϐ��ւ̃|�C���^�B@n
//!					buff��NULL�̎��A�����ɗ~�����T�C�Y���Ԃ�
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
HRESULT TBufferRenderer::SetBackBuffer(BYTE *buff, long *size)
{
	if (m_State == State_Running)
		return S_FALSE;

	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����
	if (buff == NULL && size != NULL)
	{
		*size = GetBufferSize();
		return S_OK;
	}
	if (buff == NULL || size == NULL)
		return E_POINTER;

	if ((*size) != GetBufferSize())
		return E_INVALIDARG;

	FreeBackBuffer();
	SetBackBuffer(buff);
	return S_OK;
}
//----------------------------------------------------------------------------
//! @brief		�t�����g�o�b�t�@�ւ̃|�C���^���擾���܂��B
//! @param		buff : �t�����g�o�b�t�@�ւ̃|�C���^��Ԃ����߂̃o�b�t�@�ւ̃|�C���^
//! @param		size : �o�b�t�@�̃T�C�Y��Ԃ��ϐ��ւ̃|�C���^
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
HRESULT TBufferRenderer::GetFrontBuffer(BYTE **buff, long *size)
{
	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����
	*buff = GetFrontBuffer();
	*size = GetBufferSize();
	return S_OK;
}
//----------------------------------------------------------------------------
//! @brief	  	�o�b�N�o�b�t�@�ւ̃|�C���^���擾���܂��B
//! @param		buff : �o�b�N�o�b�t�@�ւ̃|�C���^��Ԃ����߂̃o�b�t�@�ւ̃|�C���^
//! @param		size : �o�b�t�@�̃T�C�Y��Ԃ��ϐ��ւ̃|�C���^
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
HRESULT TBufferRenderer::GetBackBuffer(BYTE **buff, long *size)
{
	CAutoLock cAutoLock(&m_BufferLock);	// �N���e�B�J���Z�N�V����
	*buff = GetBackBuffer();
	*size = GetBufferSize();
	return S_OK;
}
//----------------------------------------------------------------------------
//! @brief	  	1�t���[���̕��ϕ\�����Ԃ��擾���܂�
//! @param		pAvgTimePerFrame : 1�t���[���̕��ϕ\������
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
HRESULT TBufferRenderer::get_AvgTimePerFrame(REFTIME *pAvgTimePerFrame)
{
	if (pAvgTimePerFrame) {
		*pAvgTimePerFrame = (reinterpret_cast<VIDEOINFOHEADER *>(m_MtIn.Format())->AvgTimePerFrame) / 10000000.0;
		return S_OK;
	}
	else
		return E_POINTER;
}
//----------------------------------------------------------------------------
//! @brief	  	�r�f�I�̕����擾���܂�
//! @param		pVideoWidth : �r�f�I�̕�
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
HRESULT TBufferRenderer::get_VideoWidth(long *pVideoWidth)
{
	if (pVideoWidth) {
		*pVideoWidth = reinterpret_cast<VIDEOINFOHEADER *>(m_MtIn.Format())->bmiHeader.biWidth;
		return S_OK;
	}
	else
		return E_POINTER;
}
//----------------------------------------------------------------------------
//! @brief	  	�r�f�I�̍������擾���܂�
//! @param		pVideoHeight : �r�f�I�̍���
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
HRESULT TBufferRenderer::get_VideoHeight(long *pVideoHeight)
{
	if (pVideoHeight) {
		*pVideoHeight = reinterpret_cast<VIDEOINFOHEADER *>(m_MtIn.Format())->bmiHeader.biHeight;
		return S_OK;
	}
	else
		return E_POINTER;
}
//----------------------------------------------------------------------------
//! @brief	  	�X�g���[�~���O���J�n���ꂽ���ɃR�[�������
//!
//! �J�n�t���[�����L�^����B
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
HRESULT TBufferRenderer::OnStartStreaming(void)
{
	HRESULT		hr;
	CComPtr<IMediaSeeking>	mediaSeeking;

	if (m_pGraph)
	{
		if (m_pGraph->QueryInterface(&mediaSeeking) != S_OK)
			mediaSeeking = NULL;
	}

	bool		bGetTime = false;
	LONGLONG	Current = 0;
	if (mediaSeeking.p != NULL)
	{	// IMediaSeeking���g���Ď��Ԃ̎擾�����݂�
		GUID	Format;
		if (SUCCEEDED(hr = mediaSeeking->GetTimeFormat(&Format)))
		{
			if (SUCCEEDED(hr = mediaSeeking->GetCurrentPosition(&Current)))
			{
				if (IsEqualGUID(TIME_FORMAT_MEDIA_TIME, Format))
				{
					double	renderTime = Current / 10000000.0;
					REFTIME	AvgTimePerFrame;	// REFTIME :  �b��������������\���{���x���������_���B
					if (SUCCEEDED(hr = get_AvgTimePerFrame(&AvgTimePerFrame)))
					{
						m_StartFrame = (LONG)(renderTime / AvgTimePerFrame + 0.5);
						bGetTime = true;
					}
				}
				else if (IsEqualGUID(TIME_FORMAT_FRAME, Format))
				{
					m_StartFrame = (LONG)Current;
					bGetTime = true;
				}
			}
		}
	}
	if (bGetTime == false)
		m_StartFrame = 0;

	mediaSeeking.Release();

	return CBaseVideoRenderer::OnStartStreaming();
}
//----------------------------------------------------------------------------
//! @brief	  	�����_�����O�O�ɃR�[�������
//!
//! ���f�B�A�T���v���Ƀ��f�B�A�^�C�����L�^����B
//! ���f�B�A�^�C���͊J�n�t���[���Ɍ��݂̃X�g���[�����Ԃ����Z�������̂ɂȂ�B
//! �����A�t�B���^��IMediaSeeking�C���^�[�t�F�C�X�����p�ł��Ȃ��ꍇ�́A
//! ���̃����_�[�t�B���^���`�悵���t���[�����ƃh���b�v�����t���[���������Z����B
//! ���̏ꍇ�A����ʂ̃t�B���^�Ńh���b�v�����t���[�����͂킩��Ȃ��̂ŁA
//! �኱���x��������B
//! @param		pMediaSample : ���f�B�A�T���v��
//----------------------------------------------------------------------------
void TBufferRenderer::OnRenderStart(IMediaSample *pMediaSample)
{
	CBaseVideoRenderer::OnRenderStart(pMediaSample);

	HRESULT		hr;
	bool		bGetTime = false;
	LONGLONG	Current = 0, Stop = 0;
	IMediaSeeking	*mediaSeeking = NULL;
	if (GetMediaPositionInterface(IID_IMediaSeeking, (void**)&mediaSeeking) == S_OK)
	{
		GUID	Format;
		if (SUCCEEDED(hr = mediaSeeking->GetTimeFormat(&Format)))
		{
			if (SUCCEEDED(hr = mediaSeeking->GetCurrentPosition(&Current)) &&
				SUCCEEDED(hr = mediaSeeking->GetStopPosition(&Stop)))
			{
				if (IsEqualGUID(TIME_FORMAT_MEDIA_TIME, Format))
				{
					double	renderTime = Current / 10000000.0;
					double	stopTime = Stop / 10000000.0;
					REFTIME	AvgTimePerFrame;	// REFTIME :  �b��������������\���{���x���������_���B
					if (SUCCEEDED(hr = get_AvgTimePerFrame(&AvgTimePerFrame)))
					{
						Current = (LONGLONG)(renderTime / AvgTimePerFrame + 0.5);
						Stop = (LONGLONG)(stopTime / AvgTimePerFrame + 0.5);
						bGetTime = true;
					}
				}
				else if (IsEqualGUID(TIME_FORMAT_FRAME, Format))
				{
					bGetTime = true;
				}
			}
		}
		mediaSeeking->Release();
		mediaSeeking = NULL;
	}
	LONGLONG	TimeStart = m_StartFrame + m_cFramesDrawn + m_cFramesDropped;;
	LONGLONG	TimeEnd = m_StartFrame + m_cFramesDrawn + m_cFramesDropped;;
	if (bGetTime == true)
	{
		TimeStart = m_StartFrame + Current;
		TimeEnd = m_StartFrame + Current;
		m_StopFrame = m_StartFrame + static_cast<LONG>(Stop);
	}
	else
	{
		TimeStart = m_StartFrame + m_cFramesDrawn + m_cFramesDropped;;
		TimeEnd = m_StartFrame + m_cFramesDrawn + m_cFramesDropped;;
		m_StopFrame = 0;
	}
	pMediaSample->SetMediaTime(&TimeStart, &TimeEnd);
}
//----------------------------------------------------------------------------
//##	TBufferRendererInputPin
//----------------------------------------------------------------------------
//! @brief	  	���̓s���I�u�W�F�N�g���\�z���܂��B
//! @param		pRenderer : �����_�[�I�u�W�F�N�g���w�肵�܂��B
//! @param		pInterfaceLock : CCritSec ���b�N�ւ̃|�C���^�ŁA��Ԉڍs���p�����邽�߂Ɏg�p����B@n
//!					����̓t�B���^ ���b�N CBaseFilter.m_pLock �Ɠ��l�̃N���e�B�J�� �Z�N�V�����ƂȂ肤��B 
//! @param		phr : ���\�b�h�̐����E���s������ HRESULT �l���擾����ϐ��̃|�C���^�B
//! @param		name : �I�u�W�F�N�g�̃f�o�b�O�p�̖��O�����镶����B
//----------------------------------------------------------------------------
TBufferRendererInputPin::TBufferRendererInputPin(TBufferRenderer *pRenderer, CCritSec *pInterfaceLock, HRESULT *phr, LPCWSTR name)
	: CRendererInputPin(pRenderer, phr, name), m_pRenderer(pRenderer), m_pInterfaceLock(pInterfaceLock),
	m_ActiveAllocator(false)
{
}
//----------------------------------------------------------------------------
//! @brief	  	�f�X�g���N�^�B���݂͉������Ȃ��B
//----------------------------------------------------------------------------
TBufferRendererInputPin::~TBufferRendererInputPin()
{
}
//----------------------------------------------------------------------------
//! @brief	  	���O�̃A���P�[�^���L�����ǂ������ׂ܂��B
//! @return		�L���Ȃ�TRUE��Ԃ��܂��B
//----------------------------------------------------------------------------
bool TBufferRendererInputPin::ActiveAllocator(void) const
{
	return m_ActiveAllocator;
}
//----------------------------------------------------------------------------
//! @brief	  	���O�̃A���P�[�^�I�u�W�F�N�g�����蓖�Ă܂��B
//! @param		ppAllocator : �Ԃ��A���P�[�^�[
//----------------------------------------------------------------------------
STDMETHODIMP TBufferRendererInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
	CAutoLock cInterfaceLock(m_pInterfaceLock);
	CheckPointer(ppAllocator, E_POINTER);

	// �A���P�[�^���܂��ݒ肳��Ă��Ȃ��Ƃ�
	if (m_pAllocator == NULL) {
		m_pAllocator = &(m_pRenderer->m_Allocator);
		m_pAllocator->AddRef();
	}
	// �Q�ƃJ�E���g���c���̂̓C���^�t�F�[�X�̎d�l�ł��B
	m_pAllocator->AddRef();
	*ppAllocator = m_pAllocator;

	return S_OK;
}
//----------------------------------------------------------------------------
//! @brief	  	�A���P�[�^�����܂����Ƃ��ɌĂяo����܂��B
//! @param		pAllocator ����̐ڑ��Ŏg�p����A���P�[�^���w�肵�܂��B
//! @param		bReadOnly ���̃A���P�[�^����̃T���v�����ǂ݂Ƃ��p�Ȃ�TRUE���w�肵�܂��B
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
STDMETHODIMP TBufferRendererInputPin::NotifyAllocator(IMemAllocator * pAllocator, BOOL bReadOnly)
{
	CAutoLock cInterfaceLock(m_pInterfaceLock);

	// ���N���X�Ăяo��
	HRESULT hr = CBaseInputPin::NotifyAllocator(pAllocator, bReadOnly);
	if (FAILED(hr))
		return hr;

	//���O�̃A���P�[�^���L�����ǂ������L�^���܂�
	m_ActiveAllocator = (pAllocator == (&(m_pRenderer->m_Allocator)));

	return S_OK;
}
//----------------------------------------------------------------------------
//! @brief	  	�w�肵�����f�B�A�T���v���Ƀ|�C���^��ݒ肵�܂�
//! @param		media : ���f�B�A�T���v��
//! @param		ptr : �ݒ肷��|�C���^
//----------------------------------------------------------------------------
void TBufferRendererInputPin::SetPointer(IMediaSample *media, BYTE *ptr)
{
	m_pRenderer->m_Allocator.SetPointer(media, ptr);
}
//----------------------------------------------------------------------------
//! @brief	  	�A���P�[�^�[�������f�B�A�T���v���Ƀ|�C���^��ݒ肵�܂�
//! @param		ptr : �ݒ肷��|�C���^
//----------------------------------------------------------------------------
void TBufferRendererInputPin::SetPointer(BYTE *ptr)
{
	m_pRenderer->m_Allocator.SetPointer(ptr);
}
//----------------------------------------------------------------------------
//## TBufferRendererAllocator
//----------------------------------------------------------------------------
//! @brief	  	�R���X�g���N�^
//! @param		pRenderer : �����_�[�I�u�W�F�N�g���w�肵�܂��B
//! @param		pName : �I�u�W�F�N�g�̃f�o�b�O�p�̖��O�����镶����B
//! @param		pUnk : �W�����ꂽ���L�҃I�u�W�F�N�g�ւ̃|�C���^�B
//! @param		phr : ���\�b�h�̐����E���s������ HRESULT �l���擾����ϐ��̃|�C���^�B
//----------------------------------------------------------------------------
TBufferRendererAllocator::TBufferRendererAllocator(TBufferRenderer *pRenderer, TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr)
	: CBaseAllocator(pName, pUnk, phr), m_pMediaSample(NULL), m_pRenderer(pRenderer)
{}
//----------------------------------------------------------------------------
//! @brief	  	CBaseAllocator::Decommit���R�[�����Ȃ���΂Ȃ�Ȃ��̂ŁA�R�[������B
//----------------------------------------------------------------------------
TBufferRendererAllocator::~TBufferRendererAllocator()
{
	Decommit();
	ASSERT(m_lAllocated == m_lFree.GetCount());
	// Free up all the CMediaSamples
	CMediaSample *pSample;
	for (;;)
	{
		pSample = m_lFree.RemoveHead();
		if (pSample != NULL)
			delete pSample;
		else
			break;
	}
	m_lAllocated = 0;
}
//----------------------------------------------------------------------------
//! @brief	  	�������Ȃ��B�A���P�[�^�[����������������邱�Ƃ͂Ȃ��B
//----------------------------------------------------------------------------
void TBufferRendererAllocator::Free(void)
{
}
//----------------------------------------------------------------------------
//! @brief	  	�����������蓖�ĂāA��������X�g�ɒǉ�����
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
HRESULT TBufferRendererAllocator::Alloc(void)
{
	CAutoLock	lck(this);

	/* Check he has called SetProperties */
	HRESULT hr = CBaseAllocator::Alloc();
	if (FAILED(hr))
		return hr;

	/* If the requirements haven't changed then don't reallocate */
	if (hr == S_FALSE)
		return NOERROR;

	ASSERT(hr == S_OK); // we use this fact in the loop below
	LPBYTE	pBuffer = static_cast<LPBYTE>(m_pRenderer->GetBackBuffer());

	CMediaSample *pSample;
	pSample = new CMediaSample(NAME("buffer media sample"), this, &hr, pBuffer, m_lSize);
	ASSERT(SUCCEEDED(hr));
	if (pSample == NULL) {
		return E_OUTOFMEMORY;
	}
	m_lFree.Add(pSample);
	m_lAllocated++;
	m_pMediaSample = pSample;
	return NOERROR;
}

//----------------------------------------------------------------------------
//! @brief	  	�v�����郁�����̏ڍׂ�ݒ肷��B
//! @param		pRequest : �o�b�t�@�v�����܂� ALLOCATOR_PROPERTIES �\���̂̃|�C���^
//! @param		pActual : ���ۂ̃o�b�t�@ �v���p�e�B���󂯎�� ALLOCATOR_PROPERTIES �\���̂̃|�C���^
//! @return		�G���[�R�[�h
//----------------------------------------------------------------------------
STDMETHODIMP TBufferRendererAllocator::SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual)
{
	CheckPointer(pActual, E_POINTER);
	ValidateReadWritePtr(pActual, sizeof(ALLOCATOR_PROPERTIES));
	CAutoLock	cObjectLock(this);

	ZeroMemory(pActual, sizeof(ALLOCATOR_PROPERTIES));

	if (m_bCommitted == TRUE)
		return VFW_E_ALREADY_COMMITTED;

	if (m_lFree.GetCount() < m_lAllocated)	// m_lAllocated��1�ȏ�̎��̓G���[�ɂ��������悢�H
		return VFW_E_BUFFERS_OUTSTANDING;

	if (pRequest->cBuffers == 1 && pRequest->cbBuffer == m_pRenderer->GetBufferSize() &&
		pRequest->cbAlign == 1 && pRequest->cbPrefix == 0)
	{
		*pActual = *pRequest;
		m_lSize = pRequest->cbBuffer;
		m_lCount = pRequest->cBuffers;
		m_lAlignment = pRequest->cbAlign;
		m_lPrefix = pRequest->cbPrefix;
		m_bChanged = TRUE;
		return S_OK;
	}

	return VFW_E_BADALIGN;
}
//----------------------------------------------------------------------------
//! @brief	  	�w�肵�����f�B�A�T���v���Ƀ|�C���^��ݒ肵�܂�
//! @param		media : ���f�B�A�T���v��
//! @param		ptr : �ݒ肷��|�C���^
//----------------------------------------------------------------------------
void TBufferRendererAllocator::SetPointer(IMediaSample *media, BYTE *ptr)
{
	BYTE	*pBufferParam, *pBufferOwn;
	if (media)
	{
		media->GetPointer(&pBufferParam);
		if (m_pMediaSample != NULL)
		{
			m_pMediaSample->GetPointer(&pBufferOwn);
			if (pBufferOwn == pBufferParam)	// �����o�b�t�@���w���Ă���̂ŁA�ێ����Ă���T���v���Ɠ����ƌ��Ȃ�
			{
				LONG	cBytes = m_pMediaSample->GetSize();	// �T�C�Y�͕ς���Ă��Ȃ��ƌ��Ȃ��A���O�Ƀ`�F�b�N���Ă�������
				m_pMediaSample->SetPointer(ptr, cBytes);
			}
		}
	}
}
