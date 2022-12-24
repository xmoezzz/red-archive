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

#include "d3d_allocator.h"
#include "sysmem_allocator.h"

class CQuickSyncVPP
{
public:
    CQuickSyncVPP(bool bUseD3dAlloc, MFXFrameAllocator* pFrameAllocator);
    virtual ~CQuickSyncVPP();
    mfxStatus Reset(const CQsConfig& config, MFXVideoSession* pVideoSession, mfxFrameSurface1* pSurface);
    void Reset() { ASSERT(this != NULL); m_bNeedReset = true; }
    bool NeedReset() { return m_bNeedReset; }
    mfxStatus Process(mfxFrameSurface1* pInSurface, mfxFrameSurface1*& pOutSurface);
    mfxFrameSurface1* FlushFrame();
    mfxFrameSurface1* FindFreeSurface();
    void EnableDI(bool bEnable);
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
        ASSERT(i < m_nRequiredFramesNum);

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

protected:
    void Close();
    mfxStatus InitFrameAllocator();
    mfxStatus FreeFrameAllocator();

    MFXVideoVPP*     m_pVPP;
    mfxVideoParam    m_VppVideoParams;

    MFXVideoSession* m_pVideoSession;
    mfxVersion       m_ApiVersion;
    CQsConfig        m_Config;
    mfxU32           m_nPitch;
    bool             m_bNeedReset;
    bool             m_bEnableDI;
    mfxU16           m_DefaultPicStruct;

    // Allocator
    MFXFrameAllocator*    m_pFrameAllocator;
    mfxFrameSurface1*     m_pFrameSurfaces;
    mfxFrameAllocResponse m_AllocResponse;
    mfxU16                m_nRequiredFramesNum;
    bool                  m_bUseD3DAlloc;
    volatile LONG         m_LockedSurfaces[MSDK_MAX_SURFACES];

    CQsLock  m_csLock;

private:
   DISALLOW_COPY_AND_ASSIGN(CQuickSyncVPP);
};
