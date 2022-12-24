/*
 * Copyright (c) 2013, INTEL CORPORATION
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 * Neither the name of INTEL CORPORATION nor the names of its contributors may
 * be used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "hw_device.h"
#include "d3d_allocator.h"
#include "d3d11_allocator.h"
#include "sysmem_allocator.h"

typedef std::deque<mfxFrameSurface1*> TSurfaceQueue;

class CQuickSyncDecoder 
{
public:
    CQuickSyncDecoder(const CQsConfig& cfg, mfxStatus& sts);
    ~CQuickSyncDecoder();

    mfxStatus Init(mfxVideoParam* pVideoParams, mfxU32 nPitch)
    {  
        return InternalReset(pVideoParams, nPitch, false);     
    }

    mfxStatus Reset(mfxVideoParam* pVideoParams, mfxU32 nPitch)
    {
        return InternalReset(pVideoParams, nPitch, true);     
    }

    mfxStatus Decode(mfxBitstream* pBS, mfxFrameSurface1*& pOutSurface);
    mfxStatus GetVideoParams(mfxVideoParam* pVideoParams);
    IDirect3DDeviceManager9* GetD3DDeviceManager();
    bool SetD3DDeviceManager(IDirect3DDeviceManager9* pDeviceManager);

    mfxStatus CreateAllocator();
    mfxStatus InitFrameAllocator(mfxVideoParam* pVideoParams, mfxU32 nPitch);
    mfxStatus FreeFrameAllocator();
    MFXFrameAllocator* GetFrameAllocator() { return m_pFrameAllocator; }

    inline mfxIMPL QueryIMPL()
    {
        return m_mfxImpl;
    }

    mfxStatus DecodeHeader(mfxBitstream* bs, mfxVideoParam* par);
    mfxStatus CheckHwAcceleration(mfxVideoParam* pVideoParams);
    void SetConfig(const CQsConfig& cfg) { m_Config = cfg; }
    __forceinline TSurfaceQueue& GetOutputQueue()
    {
        CQsAutoLock lock(&m_csOutputQueueLock);
        return m_OutputSurfaceQueue;
    }
    
    __forceinline bool OutputQueueEmpty()
    {
        CQsAutoLock lock(&m_csOutputQueueLock);
        return m_OutputSurfaceQueue.empty();
    }

    __forceinline size_t OutputQueueSize()
    {
        CQsAutoLock lock(&m_csOutputQueueLock);
        return m_OutputSurfaceQueue.size();
    }

    __forceinline void PushSurface(mfxFrameSurface1* pSurface)
    {
        CQsAutoLock lock(&m_csOutputQueueLock);
        m_OutputSurfaceQueue.push_back(pSurface);
    }

    __forceinline mfxFrameSurface1* PopSurface()
    {
        CQsAutoLock lock(&m_csOutputQueueLock);
        if (m_OutputSurfaceQueue.empty())
            return NULL;

        mfxFrameSurface1* pSurface = m_OutputSurfaceQueue.front();
        m_OutputSurfaceQueue.pop_front();
        return pSurface;
    }

    __forceinline void LockSurface(mfxFrameSurface1* pSurface)
    {
        ASSERT(pSurface != NULL);
        if (NULL == pSurface) return;

        size_t i = pSurface - m_pFrameSurfaces;
        ASSERT(i < m_nRequiredFramesNum);

        if (i < m_nRequiredFramesNum)
        {
            InterlockedIncrement(&m_LockedSurfaces[i]);
        }
    }

    __forceinline void UnlockSurface(mfxFrameSurface1* pSurface)
    {
        ASSERT(pSurface != NULL);
        if (NULL == pSurface) return;

        size_t i = pSurface - m_pFrameSurfaces;

        if (i < m_nRequiredFramesNum)
        {
            ASSERT(m_LockedSurfaces[i] > 0);
            InterlockedDecrement(&m_LockedSurfaces[i]);
        }
    }

    __forceinline bool IsSurfaceLocked(mfxFrameSurface1* pSurface)
    {
        ASSERT(pSurface != NULL);
        if (NULL == pSurface) return true;

        size_t i = pSurface - m_pFrameSurfaces;
        ASSERT(i < m_nRequiredFramesNum);
        return (i < m_nRequiredFramesNum) ? (m_LockedSurfaces[i] > 0 || pSurface->Data.Locked > 0) : true;
    }

    __forceinline bool IsD3DAlloc() const { return m_bUseD3DAlloc; }
    __forceinline bool IsD3D11Alloc() const { return m_bUseD3D11Alloc; }
    __forceinline bool IsHwAccelerated() const { return m_bHwAcceleration; }

    mfxStatus LockFrame(mfxFrameSurface1* pSurface, mfxFrameData* pFrameData);
    mfxStatus UnlockFrame(mfxFrameSurface1* pSurface, mfxFrameData* pFrameData);

    void SetAuxFramesCount(size_t count);
    mfxFrameSurface1* FindFreeSurface();
    inline MFXVideoSession* GetSession()
    {
        return m_mfxVideoSession;
    }

protected:
    mfxStatus         InternalReset(mfxVideoParam* pVideoParams, mfxU32 nPitch, bool bInited);
    mfxStatus         InitSession(mfxIMPL impl);
    void              CloseSession();
    void              CloseD3D();

// data members
    // session
    MFXVideoSession* m_mfxVideoSession;
    mfxVersion       m_ApiVersion;
    mfxIMPL          m_mfxImpl;
    CQsConfig        m_Config;
    bool             m_bHwAcceleration;

    // Decoder
    MFXVideoDECODE* m_pmfxDEC;
    mfxVideoParam*  m_pVideoParams;

    // Allocator
    MFXFrameAllocator*    m_pFrameAllocator;
    mfxFrameSurface1*     m_pFrameSurfaces;
    mfxFrameAllocResponse m_AllocResponse;
    mfxU16                m_nRequiredFramesNum;
    bool                  m_bUseD3DAlloc;
    bool                  m_bUseD3D11Alloc;
    mfxU16                m_nAuxFrameCount;

    // D3D/DXVA interfaces
    IDirect3DDeviceManager9* m_pRendererD3dDeviceManager;
    CHWDevice*               m_HwDevice;

    TSurfaceQueue m_OutputSurfaceQueue;
    volatile LONG m_LockedSurfaces[MSDK_MAX_SURFACES];

    // Various locks
    CQsLock m_csOutputQueueLock;

private:
   DISALLOW_COPY_AND_ASSIGN(CQuickSyncDecoder);
};
