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

#include "stdafx.h"
#include "IQuickSyncDecoder.h"
#include "QuickSync_defs.h"
#include "QuickSyncUtils.h"
#include "QuickSyncVPP.h"

CQuickSyncVPP::CQuickSyncVPP(bool bUseD3dAlloc, MFXFrameAllocator* pFrameAllocator) :
    m_pVPP(NULL),
    m_pVideoSession(NULL),
    m_nPitch(0),
    m_bNeedReset(true),
    m_bEnableDI(true),
    m_pFrameAllocator(pFrameAllocator),
    m_pFrameSurfaces(NULL),
    m_nRequiredFramesNum(0),
    m_bUseD3DAlloc(bUseD3dAlloc)
{
    MSDK_TRACE("QsVPP: VPP created\n");

    MSDK_ZERO_VAR(m_ApiVersion);
    MSDK_ZERO_VAR(m_Config);
    MSDK_ZERO_VAR(m_VppVideoParams);
    MSDK_ZERO_MEMORY((void*)&m_LockedSurfaces, sizeof(m_LockedSurfaces));
    MSDK_ZERO_VAR(m_AllocResponse);
}

CQuickSyncVPP::~CQuickSyncVPP()
{
    ASSERT(this != NULL);
    CQsAutoLock lock(&m_csLock);
    Close();
    MSDK_TRACE("QsVPP: VPP destroyed\n");
}

void CQuickSyncVPP::Close()
{
    if (!m_pVPP)
        return;

    MSDK_TRACE("QsVPP: VPP closed\n");

    MSDK_SAFE_DELETE(m_pVPP);

    FreeFrameAllocator();
    m_pVideoSession = NULL;
}

mfxStatus CQuickSyncVPP::Reset(const CQsConfig& config, MFXVideoSession* pVideoSession, mfxFrameSurface1* pSurface)
{
    MSDK_TRACE("QsVPP: VPP reset\n");

    ASSERT(this != NULL);
    CQsAutoLock lock(&m_csLock);

    if (!config.bEnableVideoProcessing)
    {
        Close();
        return MFX_ERR_NOT_INITIALIZED;
    }

    mfxStatus sts;
    MSDK_CHECK_POINTER(pVideoSession, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(m_pFrameAllocator, MFX_ERR_NULL_PTR);

    // Forced deinterlacing:
    // Interlaced frames keep their flags, progressive frames are modified
    if (config.bVppEnableForcedDeinterlacing)
    {
        m_bEnableDI = true;
        m_DefaultPicStruct = 0;

        switch (config.eFieldOrder)
        {
        case QS_FIELD_AUTO:
            {
                if (pSurface->Info.PicStruct & MFX_PICSTRUCT_FIELD_TFF)
                {
                    m_DefaultPicStruct = MFX_PICSTRUCT_FIELD_TFF;
                }
                else if (pSurface->Info.PicStruct & MFX_PICSTRUCT_FIELD_BFF)
                {
                    m_DefaultPicStruct = MFX_PICSTRUCT_FIELD_BFF;
                }
            }
            break;
        case QS_FIELD_TFF:
            m_DefaultPicStruct = MFX_PICSTRUCT_FIELD_TFF;
            break;
        case QS_FIELD_BFF:
            m_DefaultPicStruct = MFX_PICSTRUCT_FIELD_BFF;
            break;
        default:
            break;
        }

        pSurface->Info.PicStruct = m_DefaultPicStruct;
    }

    // Check if VPP is enabled
    if ((!config.bVppEnableDeinterlacing || !m_bEnableDI) && config.nVppDenoiseStrength == 0 && config.nVppDetailStrength == 0)
    {
        Close();
        return MFX_ERR_NOT_INITIALIZED;
    }

    m_nPitch = pSurface->Data.Pitch;
    m_Config = config;

    m_pVideoSession = pVideoSession;

    // Create VPP video params
    m_VppVideoParams.vpp.In  = pSurface->Info;
    m_VppVideoParams.vpp.Out = pSurface->Info;
    m_VppVideoParams.vpp.In.AspectRatioH = m_VppVideoParams.vpp.In.AspectRatioW = 0;
    m_VppVideoParams.vpp.Out.AspectRatioH = m_VppVideoParams.vpp.Out.AspectRatioW = 0;
    m_VppVideoParams.IOPattern = (m_bUseD3DAlloc) ? 
        MFX_IOPATTERN_OUT_VIDEO_MEMORY | MFX_IOPATTERN_IN_VIDEO_MEMORY : 
        MFX_IOPATTERN_OUT_SYSTEM_MEMORY | MFX_IOPATTERN_IN_SYSTEM_MEMORY;

//
// Setup output fields
//
    // Enable auto-deinterlacing by forcing output frames to be progressive
    if (m_Config.bVppEnableDeinterlacing && m_bEnableDI)
    {
        if ((pSurface->Info.PicStruct & 0x1F) != MFX_PICSTRUCT_PROGRESSIVE)
        {
            m_VppVideoParams.vpp.Out.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
            double inputFrameRate = (m_VppVideoParams.vpp.Out.FrameRateExtD > 0) ? (double)m_VppVideoParams.vpp.Out.FrameRateExtN / (double)m_VppVideoParams.vpp.Out.FrameRateExtD : 0;
            if (m_Config.bVppEnableFullRateDI &&  inputFrameRate < 30.001)
            {
                m_VppVideoParams.vpp.Out.FrameRateExtN *= 2;
            }
        }
    }
    else
    {
        // Disable DI
        m_VppVideoParams.vpp.In.PicStruct  = MFX_PICSTRUCT_PROGRESSIVE;
        m_VppVideoParams.vpp.Out.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    }

    std::vector<mfxExtBuffer*> extBuffers;

    // Do not use these algorithms
    mfxExtVPPDoNotUse dnu;
    mfxU16 dnuCount = 0;
    mfxU32 doNotUseAlgs[8]; // { MFX_EXTBUFF_VPP_DENOISE, MFX_EXTBUFF_VPP_DETAIL, MFX_EXTBUFF_VPP_PROCAMP };
    doNotUseAlgs[dnuCount++] = MFX_EXTBUFF_VPP_PROCAMP;
    dnu.Header.BufferId = MFX_EXTBUFF_VPP_DONOTUSE;
    dnu.Header.BufferSz = sizeof(mfxExtVPPDoNotUse);
    dnu.AlgList = doNotUseAlgs;

    // Used algorithms
    mfxExtVPPDoUse du;
    mfxU16 duCount = 0;
    mfxU32 doUseAlgs[8];
    du.Header.BufferId = MFX_EXTBUFF_VPP_DOUSE;
    du.Header.BufferSz = sizeof(mfxExtVPPDoUse);
    du.AlgList = doUseAlgs;

    // Detail
    mfxExtVPPDetail bufDetail;
    if (m_Config.nVppDetailStrength > 0)
    {
        bufDetail.Header.BufferId = MFX_EXTBUFF_VPP_DETAIL;
        bufDetail.Header.BufferSz = sizeof(mfxExtVPPDetail);
        bufDetail.DetailFactor    = (mfxU16)min(100, m_Config.nVppDetailStrength);
        doUseAlgs[duCount++]      = MFX_EXTBUFF_VPP_DETAIL;
        extBuffers.push_back((mfxExtBuffer*)&bufDetail);
    }
    else
    {
        doNotUseAlgs[dnuCount++] = MFX_EXTBUFF_VPP_DETAIL;
    }

    // Denoise
    mfxExtVPPDenoise bufDenoise;
    if (m_Config.nVppDenoiseStrength > 0)
    {
        bufDenoise.Header.BufferId = MFX_EXTBUFF_VPP_DENOISE;
        bufDenoise.Header.BufferSz = sizeof(mfxExtVPPDenoise);
        bufDenoise.DenoiseFactor   = (mfxU16)min(100, m_Config.nVppDenoiseStrength);
        doUseAlgs[duCount++]       = MFX_EXTBUFF_VPP_DENOISE;
        extBuffers.push_back((mfxExtBuffer*)&bufDenoise);
    }
    else
    {
        doNotUseAlgs[dnuCount++] = MFX_EXTBUFF_VPP_DENOISE;
    }

    dnu.NumAlg = dnuCount;
    du.NumAlg  = duCount;

    if (dnuCount > 0) extBuffers.push_back((mfxExtBuffer*)&dnu);
    if (duCount > 0)  extBuffers.push_back((mfxExtBuffer*)&du);

    m_VppVideoParams.NumExtParam = (mfxU16)extBuffers.size();
    m_VppVideoParams.ExtParam = &extBuffers.front();

    m_bNeedReset = false;
    
    // Flush old frames
    while (NULL != FlushFrame())
    {
    }

    // Soft reset
    if (m_pVPP)
    {
        // Setup allocator
        InitFrameAllocator();

        // Init
        sts = m_pVPP->Reset(&m_VppVideoParams);
        MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
        MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);
        if (MSDK_SUCCEEDED(sts))
        {
            return MFX_ERR_NONE;
        }
        else
        {
            m_pVPP->Close();
            sts = m_pVPP->Init(&m_VppVideoParams);
        }
        
        return sts;
    }
    
    // Hard reset
    m_pVPP = new MFXVideoVPP(*pVideoSession);

    // Setup allocator
    InitFrameAllocator();

    // Init
    sts = m_pVPP->Init(&m_VppVideoParams);
    return sts;
}

mfxStatus CQuickSyncVPP::Process(mfxFrameSurface1* pInSurface, mfxFrameSurface1*& pOutSurface)
{
    ASSERT(this != NULL);
    CQsAutoLock lock(&m_csLock);
    mfxStatus sts, rc;
    mfxSyncPoint syncp;

    // Find available output surface
    pOutSurface = FindFreeSurface();
    MSDK_CHECK_POINTER(pOutSurface, MFX_ERR_NOT_ENOUGH_BUFFER);

    mfxU16 picStructSave = (pInSurface != NULL) ? pInSurface->Info.PicStruct : 0;
    mfxU16& inPicStruct  = (pInSurface != NULL) ? pInSurface->Info.PicStruct : picStructSave;

    // Workaround: remove frame duplication flags, causes VPP to fail
    if (inPicStruct & MFX_PICSTRUCT_FRAME_DOUBLING)
        inPicStruct ^= MFX_PICSTRUCT_FRAME_DOUBLING;
    else if (inPicStruct & MFX_PICSTRUCT_FRAME_TRIPLING)
        inPicStruct ^= MFX_PICSTRUCT_FRAME_TRIPLING;

    // Force interlaced flags
    if (m_Config.bVppEnableForcedDeinterlacing && pInSurface != NULL)
    {
        // Frame is TFF
        if (inPicStruct & MFX_PICSTRUCT_FIELD_TFF)
            inPicStruct = MFX_PICSTRUCT_FIELD_TFF;
        // Frame is BFF
        else if (inPicStruct & MFX_PICSTRUCT_FIELD_BFF)
            inPicStruct = MFX_PICSTRUCT_FIELD_BFF;
        // Frame is progressive - override with default flag
        else if (m_DefaultPicStruct != 0)
            inPicStruct = m_DefaultPicStruct;
        else
        {
            // Arbitrary choose TFF
            inPicStruct = MFX_PICSTRUCT_FIELD_TFF;
        }
    }

    // Workaround for DI bug - doesn't accept frame with MFX_PICSTRUCT_PROGRESSIVE flag
    if (m_Config.bVppEnableDeinterlacing)
    {        
        if (0 != (inPicStruct & MFX_PICSTRUCT_PROGRESSIVE) && inPicStruct != MFX_PICSTRUCT_PROGRESSIVE)
        {
            inPicStruct ^= MFX_PICSTRUCT_PROGRESSIVE;
        }
    }

    // Call VPP
    do
    {
        sts = m_pVPP->RunFrameVPPAsync(pInSurface, pOutSurface, NULL, &syncp);

        if (MFX_WRN_DEVICE_BUSY == sts)
        {
            MSDK_TRACE("QsVPP: MFX_WRN_DEVICE_BUSY\n");
            Sleep(1);
        }
    } while (MFX_WRN_DEVICE_BUSY == sts);

    rc = sts;

    if (sts >= 0 || MFX_ERR_MORE_SURFACE == sts)
    {
        // Wait for the asynch decoding to finish
        while (MFX_WRN_IN_EXECUTION == (sts = m_pVideoSession->SyncOperation(syncp, 0xFFFF)))
        {
            MSDK_TRACE("QsVPP: MFX_WRN_IN_EXECUTION\n");
        }

        // Some error has occurred
        if (sts < 0)
        {
            pOutSurface = NULL;
            rc = sts;
        }
        else
        {
            LockSurface(pOutSurface);

            // Restore frame doubling/trippling flags to output frame
            if (picStructSave & MFX_PICSTRUCT_FRAME_DOUBLING)
                pOutSurface->Info.PicStruct ^= MFX_PICSTRUCT_FRAME_DOUBLING;
            else if (picStructSave & MFX_PICSTRUCT_FRAME_TRIPLING)
                pOutSurface->Info.PicStruct ^= MFX_PICSTRUCT_FRAME_TRIPLING;
        }
    }

    inPicStruct = picStructSave;
    return rc;
}

mfxFrameSurface1* CQuickSyncVPP::FindFreeSurface()
{
    ASSERT(this != NULL);
    CQsAutoLock lock(&m_csLock);

    MSDK_CHECK_POINTER(m_pFrameSurfaces, NULL);
#ifdef _DEBUG
    static int s_SleepCount = 0;
#endif

    // 1 second cycle
    for (int tries = 0; tries < 1000; ++tries)
    {
        for (int i = 0; i < m_nRequiredFramesNum; ++i)
        {
            if (!IsSurfaceLocked(m_pFrameSurfaces + i))
            {
                // Found free surface
                return m_pFrameSurfaces + i;
            }
        }

        // Note: code should not reach here!
        MSDK_TRACE("QsVPP: FindFreeSurface - all surfaces are in use, retrying in 1ms (%d)\n", ++s_SleepCount);
        Sleep(1);
    }

    return NULL;
}

mfxStatus CQuickSyncVPP::InitFrameAllocator()
{
    mfxStatus sts = MFX_ERR_NONE;
    MSDK_CHECK_POINTER(m_pFrameAllocator, MFX_ERR_NULL_PTR);

    //Check if existing allocation is OK
    if (m_pFrameSurfaces != NULL)
    {
        const mfxFrameInfo& oldInfo = m_pFrameSurfaces[0].Info;
        const mfxFrameInfo& newInfo = m_VppVideoParams.vpp.In;
        if (newInfo.Width == oldInfo.Width &&
            newInfo.Height == oldInfo.Height && 
            m_pFrameSurfaces[0].Data.Pitch == m_nPitch)
        {
            goto done;
        }
    }
    
    mfxFrameAllocRequest  allocRequest[2];
    MSDK_ZERO_VAR(allocRequest);

    sts = m_pVPP->QueryIOSurf(&m_VppVideoParams, allocRequest);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);
    MSDK_CHECK_RESULT_P_RET(sts, MFX_ERR_NONE);

    FreeFrameAllocator();

    // Decide memory type
    allocRequest[1].Type = MFX_MEMTYPE_EXTERNAL_FRAME | MFX_MEMTYPE_FROM_VPPOUT;
    allocRequest[1].Type |= (m_bUseD3DAlloc) ?
       MFX_MEMTYPE_VIDEO_MEMORY_PROCESSOR_TARGET : MFX_MEMTYPE_SYSTEM_MEMORY;

    // Allocate frames with H aligned at 32 for both progressive and interlaced content
    allocRequest[1].Info.Height = MSDK_ALIGN32(allocRequest[1].Info.Height); 
    allocRequest[1].Info.Width = (mfxU16)m_nPitch;
    allocRequest[1].NumFrameMin = allocRequest[1].NumFrameSuggested;
    
    // Perform allocation call. result is saved in m_AllocResponse
    sts = m_pFrameAllocator->Alloc(m_pFrameAllocator->pthis, &allocRequest[1], &m_AllocResponse);
    MSDK_CHECK_RESULT_P_RET(sts, MFX_ERR_NONE);

    m_nRequiredFramesNum = m_AllocResponse.NumFrameActual;
    ASSERT(m_nRequiredFramesNum == allocRequest[1].NumFrameSuggested);
    m_pFrameSurfaces = new mfxFrameSurface1[m_nRequiredFramesNum];
    MSDK_CHECK_POINTER(m_pFrameSurfaces, MFX_ERR_MEMORY_ALLOC);

done:
    MSDK_ZERO_MEMORY((void*)&m_LockedSurfaces, sizeof(m_LockedSurfaces));
    MSDK_ZERO_MEMORY(m_pFrameSurfaces, sizeof(mfxFrameSurface1) * m_nRequiredFramesNum);

    // Allocate decoder work & output surfaces
    for (mfxU32 i = 0; i < m_nRequiredFramesNum; ++i)
    {
        // Copy frame info
        memcpy(&(m_pFrameSurfaces[i].Info), &m_VppVideoParams.vpp.Out, sizeof(mfxFrameInfo));

        // Save pointer to allocator specific surface object (mid)
        m_pFrameSurfaces[i].Data.MemId  = m_AllocResponse.mids[i];
        m_pFrameSurfaces[i].Data.Pitch  = (mfxU16)m_nPitch;
    }

    return sts;
}

mfxStatus CQuickSyncVPP::FreeFrameAllocator()
{
    mfxStatus sts = MFX_ERR_NONE;
    if (m_pFrameAllocator && m_nRequiredFramesNum)
    {
        sts = m_pFrameAllocator->Free(m_pFrameAllocator->pthis, &m_AllocResponse);
        MSDK_ZERO_VAR(m_AllocResponse);
    }

    m_nRequiredFramesNum = 0;
    MSDK_SAFE_DELETE_ARRAY(m_pFrameSurfaces);
    return MFX_ERR_NONE;
}

void CQuickSyncVPP::EnableDI(bool bEnable)
{
    bEnable  = bEnable || m_Config.bVppEnableForcedDeinterlacing;
    if (m_bEnableDI == bEnable)
        return;

    m_bEnableDI = bEnable;
    m_bNeedReset = true;
}

mfxFrameSurface1* CQuickSyncVPP::FlushFrame()
{
    mfxFrameSurface1* pOutSurface = NULL;

    // Run VPP
    mfxStatus sts = Process(NULL, pOutSurface);
    return (MSDK_SUCCEEDED(sts) || MFX_ERR_MORE_SURFACE == sts) ? pOutSurface : NULL;
}
