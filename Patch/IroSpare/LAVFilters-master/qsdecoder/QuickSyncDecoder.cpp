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
#include "QuickSyncDecoder.h"
#include "d3d_device.h"
#include "d3d11_device.h"

CQuickSyncDecoder::CQuickSyncDecoder(const CQsConfig& cfg, mfxStatus& sts) :
    m_mfxVideoSession(NULL),
    m_mfxImpl(MFX_IMPL_UNSUPPORTED),
    m_Config(cfg),
    m_pmfxDEC(0),
    m_pVideoParams(0),
    m_pFrameAllocator(NULL),
    m_pFrameSurfaces(NULL),
    m_nRequiredFramesNum(0),
    m_nAuxFrameCount(0),
    m_pRendererD3dDeviceManager(NULL),
    m_HwDevice(NULL)
{
    MSDK_ZERO_VAR(m_AllocResponse);

    m_ApiVersion.Major = MIN_REQUIRED_API_VER_MAJOR;
    m_ApiVersion.Minor = MIN_REQUIRED_API_VER_MINOR;
    mfxIMPL impl = MFX_IMPL_AUTO_ANY;

    // Uncomment for SW emulation (Media SDK software DLL must be present)
    //impl = MFX_IMPL_SOFTWARE;

    int d3d9IntelAdapter = 0;
    if (impl != MFX_IMPL_SOFTWARE)
    {
        d3d9IntelAdapter = GetIntelAdapterIdD3D9(NULL);

        // select d3d9 or d3d11 HW implementation base on config and GPU/OS capabilities
        if (d3d9IntelAdapter >= 0 && !m_Config.bDefaultToD3D11)
        {
            impl |= MFX_IMPL_VIA_D3D9;
        }
        else if (m_Config.bEnableD3D11)
        {
            impl |= MFX_IMPL_VIA_D3D11;
        }
        else
        {
            MSDK_TRACE("QsDecoder: Can't create HW decoder, the iGPU is not connected to a screen!\n");
            sts = MFX_ERR_UNSUPPORTED;
            return;
        }
    }

    sts = InitSession(impl);
    if (MSDK_SUCCEEDED(sts) && !m_Config.bEnableD3D11 && 0 > d3d9IntelAdapter)
    {
        MSDK_TRACE("QsDecoder: can't create HW decoder, the iGPU is not connected to a screen!\n");
        sts = MFX_ERR_UNSUPPORTED;
    }
}

CQuickSyncDecoder::~CQuickSyncDecoder()
{
    CloseSession();

    FreeFrameAllocator();
    delete m_pFrameAllocator;
    CloseD3D();
}

mfxStatus CQuickSyncDecoder::InitSession(mfxIMPL impl)
{
    if (m_mfxVideoSession != NULL)
        return MFX_ERR_NONE;

    m_mfxVideoSession = new MFXVideoSession;
    mfxStatus sts = m_mfxVideoSession->Init(impl, &m_ApiVersion);
    if (MSDK_FAILED(sts))
    {
        MSDK_TRACE("QsDecoder: failed to initialize MSDK session!\n");
        return sts;
    }

    m_mfxVideoSession->QueryIMPL(&m_mfxImpl);
    m_mfxVideoSession->QueryVersion(&m_ApiVersion);

    m_bHwAcceleration = m_mfxImpl != MFX_IMPL_SOFTWARE;
    m_bUseD3DAlloc = m_bHwAcceleration;
    m_bUseD3D11Alloc = m_bUseD3DAlloc && ((m_mfxImpl & MFX_IMPL_VIA_D3D11) == MFX_IMPL_VIA_D3D11);

    m_pmfxDEC = new MFXVideoDECODE((mfxSession)*m_mfxVideoSession);

#if MFX_D3D11_SUPPORT
    if (m_bUseD3D11Alloc)
    {
        int nAdapterID = GetMSDKAdapterNumber(*m_mfxVideoSession);

        if (NULL == m_HwDevice)
        {
            m_HwDevice = new CD3D11Device();
            if (MSDK_FAILED(sts = m_HwDevice->Init(nAdapterID)))
            {
                MSDK_TRACE("QsDecoder: D3D11 init have failed!\n");
                MSDK_SAFE_DELETE(m_HwDevice);
                return sts;
            }
        }

        mfxHDL h = m_HwDevice->GetHandle(MFX_HANDLE_D3D11_DEVICE);
        sts = m_mfxVideoSession->SetHandle(MFX_HANDLE_D3D11_DEVICE, h);
    }
#endif

    MSDK_ZERO_MEMORY((void*)&m_LockedSurfaces, sizeof(m_LockedSurfaces));
    return MFX_ERR_NONE;
}

void CQuickSyncDecoder::CloseSession()
{
    MSDK_SAFE_DELETE(m_pmfxDEC);
    MSDK_SAFE_DELETE(m_mfxVideoSession);
}

mfxFrameSurface1* CQuickSyncDecoder::FindFreeSurface()
{
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
        MSDK_TRACE("QsDecoder: FindFreeSurface - all surfaces are in use, retrying in 1ms (%d)\n", ++s_SleepCount);
        Sleep(1);
    }

    return NULL;
}

mfxStatus CQuickSyncDecoder::InitFrameAllocator(mfxVideoParam* pVideoParams, mfxU32 nPitch)
{
    MSDK_TRACE("QsDecoder: InitFrameAllocator\n");
    // Already initialized
    if (m_pFrameSurfaces)
    {
        return MFX_ERR_NONE;
    }

    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_NOT_INITIALIZED);
    mfxStatus sts = MFX_ERR_NONE;

    // Initialize frame allocator (if needed)
    sts = CreateAllocator();
    MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, sts);

    // Find how many surfaces are needed
    mfxFrameAllocRequest allocRequest;
    MSDK_ZERO_VAR(allocRequest);
    sts = m_pmfxDEC->QueryIOSurf(pVideoParams, &allocRequest);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);
    MSDK_CHECK_RESULT_P_RET(sts, MFX_ERR_NONE);
    allocRequest.NumFrameSuggested = (mfxU16)m_nAuxFrameCount + allocRequest.NumFrameSuggested;
    allocRequest.NumFrameMin = allocRequest.NumFrameSuggested;

    // Decide memory type
    allocRequest.Type = MFX_MEMTYPE_EXTERNAL_FRAME | MFX_MEMTYPE_FROM_DECODE;
    allocRequest.Type |= (m_bUseD3DAlloc) ?
       MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET : MFX_MEMTYPE_SYSTEM_MEMORY;

    memcpy(&allocRequest.Info, &pVideoParams->mfx.FrameInfo, sizeof(mfxFrameInfo));

    // Allocate frames with H aligned at 32 for both progressive and interlaced content
    allocRequest.Info.Height = MSDK_ALIGN32(allocRequest.Info.Height); 
    allocRequest.Info.Width = (mfxU16)nPitch;

    // Perform allocation call. result is saved in m_AllocResponse
    sts = m_pFrameAllocator->Alloc(m_pFrameAllocator->pthis, &allocRequest, &m_AllocResponse);
    MSDK_CHECK_RESULT_P_RET(sts, MFX_ERR_NONE);

    m_nRequiredFramesNum = m_AllocResponse.NumFrameActual;
    ASSERT(m_nRequiredFramesNum == allocRequest.NumFrameSuggested);
    m_pFrameSurfaces = new mfxFrameSurface1[m_nRequiredFramesNum];
    MSDK_CHECK_POINTER(m_pFrameSurfaces, MFX_ERR_MEMORY_ALLOC);
    MSDK_ZERO_MEMORY(m_pFrameSurfaces, sizeof(mfxFrameSurface1) * m_nRequiredFramesNum);

    // Allocate decoder work & output surfaces
    for (mfxU32 i = 0; i < m_nRequiredFramesNum; ++i)
    {
        // Copy frame info
        memcpy(&(m_pFrameSurfaces[i].Info), &pVideoParams->mfx.FrameInfo, sizeof(mfxFrameInfo));

        // Save pointer to allocator specific surface object (mid)
        m_pFrameSurfaces[i].Data.MemId  = m_AllocResponse.mids[i];
        m_pFrameSurfaces[i].Data.Pitch  = (mfxU16)nPitch;
    }

    return sts;
}

mfxStatus CQuickSyncDecoder::FreeFrameAllocator()
{
    mfxStatus sts = MFX_ERR_NONE;
    if (m_pFrameAllocator && m_AllocResponse.NumFrameActual > 0)
    {
        sts = m_pFrameAllocator->Free(m_pFrameAllocator->pthis, &m_AllocResponse);
        MSDK_ZERO_VAR(m_AllocResponse);
    }

    m_nRequiredFramesNum = 0;
    MSDK_SAFE_DELETE_ARRAY(m_pFrameSurfaces);
    return MFX_ERR_NONE;
}

mfxStatus CQuickSyncDecoder::InternalReset(mfxVideoParam* pVideoParams, mfxU32 nPitch, bool bInited)
{
    MSDK_CHECK_POINTER(pVideoParams, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_NOT_INITIALIZED);
    
    mfxStatus sts = MFX_ERR_NONE;
    m_pVideoParams = pVideoParams;

    if (NULL == m_pFrameAllocator)
    {
        bInited = false;
    }

    // Reset decoder
    if (bInited)
    {
        sts = m_pmfxDEC->Reset(pVideoParams);
        // Need to reset the frame allocator
        if (MSDK_FAILED(sts))
        {
            m_pmfxDEC->Close();
            FreeFrameAllocator();
            bInited = false;
        }
        
        if (m_pFrameSurfaces != NULL)
        {
            // Another VC1 decoder + VPP bug workaround
            for (int i = 0; i < m_nRequiredFramesNum; ++i)
            {
                m_pFrameSurfaces[i].Data.Locked = 0;
                m_LockedSurfaces[i] = 0;
            }
        }
    }

    // Full init
    if (!bInited)
    {
        // Setup allocator - will initialize D3D if needed
        sts = InitFrameAllocator(pVideoParams, nPitch);
        MSDK_CHECK_RESULT_P_RET(sts, MFX_ERR_NONE);

        // Init MSDK decoder
        sts = m_pmfxDEC->Init(pVideoParams);
        switch (sts)
        {
        case MFX_ERR_NONE:
            MSDK_TRACE("QsDecoder: decoder Init is successful\n");
            break;
        case MFX_WRN_PARTIAL_ACCELERATION:
            MSDK_TRACE("QsDecoder: decoder Init is successful w/o HW acceleration\n");
            m_bHwAcceleration = false;
            break;
        case MFX_WRN_INCOMPATIBLE_VIDEO_PARAM:
            MSDK_TRACE("QsDecoder: decoder Init is successful - wrong video parameters\n");
            break;
        default:
            MSDK_TRACE("QsDecoder: decoder Init has failed!\n");
            break;
        }
    }

    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    return sts;
}

mfxStatus CQuickSyncDecoder::CheckHwAcceleration(mfxVideoParam* pVideoParams)
{
    MSDK_CHECK_POINTER(pVideoParams, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_NOT_INITIALIZED);

    // If we are already using SW decoder then no need for further checks...
    mfxIMPL impl = QueryIMPL();
    if (MFX_IMPL_SOFTWARE == impl)
        return MFX_ERR_NONE;

    mfxU16 ioPaternSave = pVideoParams->IOPattern;
    pVideoParams->IOPattern = (mfxU16)MFX_IOPATTERN_OUT_VIDEO_MEMORY;
    mfxVideoParam tmp = *pVideoParams;
    mfxStatus sts = m_pmfxDEC->Query(pVideoParams, &tmp);
    pVideoParams->IOPattern = ioPaternSave;
    return sts;
}

mfxStatus CQuickSyncDecoder::GetVideoParams(mfxVideoParam* pVideoParams)
{
    MSDK_CHECK_POINTER(pVideoParams, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_NOT_INITIALIZED);

    return m_pmfxDEC->GetVideoParam(pVideoParams);
}

mfxStatus CQuickSyncDecoder::Decode(mfxBitstream* pBS, mfxFrameSurface1*& pOutSurface)
{
    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_NOT_INITIALIZED);    

    mfxStatus sts = MFX_ERR_NONE;
    mfxSyncPoint syncp;
    mfxFrameSurface1* pWorkSurface = FindFreeSurface();
    MSDK_CHECK_POINTER(pWorkSurface, MFX_ERR_NOT_ENOUGH_BUFFER);
    do
    {
        sts = m_pmfxDEC->DecodeFrameAsync(pBS, pWorkSurface, &pOutSurface, &syncp);
        // Need 1 more work surface
        if (MFX_ERR_MORE_SURFACE == sts)
        {
            pWorkSurface = FindFreeSurface();
            MSDK_CHECK_POINTER(pWorkSurface, MFX_ERR_NOT_ENOUGH_BUFFER);
        }
        else if (MFX_WRN_DEVICE_BUSY == sts)
        {
            static int s_BusyCount = 0;
            MSDK_TRACE("QsDecoder: MFX_WRN_DEVICE_BUSY (%i)\n", ++s_BusyCount);
            Sleep(1);
        }
    } while (MFX_WRN_DEVICE_BUSY == sts || MFX_ERR_MORE_SURFACE == sts);

    // Output will be shortly available
    if (MSDK_SUCCEEDED(sts)) 
    {
        // Wait for the asynch decoding to finish
        sts = m_mfxVideoSession->SyncOperation(syncp, 0xFFFF);

        if (MSDK_SUCCEEDED(sts))
        {
            // The surface is locked from being reused in another Decode call
            LockSurface(pOutSurface);
        }
    }

    return sts;
}

void CQuickSyncDecoder::CloseD3D()
{
    if (m_bUseD3DAlloc)
    {
        if (m_mfxVideoSession)
        {
            m_mfxVideoSession->SetHandle(MFX_HANDLE_D3D9_DEVICE_MANAGER, NULL);
            m_mfxVideoSession->SetHandle(MFX_HANDLE_D3D11_DEVICE, NULL);
        }

        MSDK_SAFE_DELETE(m_HwDevice);
    }
}

mfxStatus CQuickSyncDecoder::DecodeHeader(mfxBitstream* bs, mfxVideoParam* par)
{
    MSDK_CHECK_POINTER(m_pmfxDEC && bs && par, MFX_ERR_NULL_PTR);
    mfxStatus sts = m_pmfxDEC->DecodeHeader(bs, par);

    // Try again, marking the bitstream as complete
    // This workaround should work on all driver versions. But it doesn't work on 15.28 & 15.31
    if (MFX_ERR_MORE_DATA == sts)
    {
        mfxU16 oldFlag = bs->DataFlag;
        bs->DataFlag = MFX_BITSTREAM_COMPLETE_FRAME;
        sts = m_pmfxDEC->DecodeHeader(bs, par);
        bs->DataFlag = oldFlag;
    }

    // Another workaround for 15.28 and 15.31 drivers
    if (MFX_ERR_MORE_DATA == sts && par->mfx.CodecId == MFX_CODEC_AVC)
    {
        mfxBitstream bs2 = *bs;
        
        bs2.Data = new mfxU8[bs->DataLength + 5];
        memcpy(bs2.Data, bs->Data + bs->DataOffset, bs->DataLength - bs->DataOffset);
        bs2.MaxLength = bs2.DataLength = bs->DataLength + 5 - bs->DataOffset;

        // Write H264 start code + start of splice section
        *((unsigned*)(bs2.Data + bs2.DataLength - 5)) = 0x01000000;
        bs2.Data[bs2.DataLength -1]                = 1; // write SPLICE NALU
        sts = m_pmfxDEC->DecodeHeader(&bs2, par);
        delete[] bs2.Data; // Cleanup
    }

    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    return sts; 
}

mfxStatus CQuickSyncDecoder::CreateAllocator()
{
    if (m_pFrameAllocator != NULL)
        return MFX_ERR_NONE;

    MSDK_TRACE("QsDecoder: CreateAllocator\n");

    ASSERT(m_pVideoParams != NULL);
    if (NULL == m_pVideoParams)
        return MFX_ERR_NOT_INITIALIZED;

    std::auto_ptr<mfxAllocatorParams> pParam(NULL);
    mfxStatus sts = MFX_ERR_NONE;

    // Setup allocator - HW acceleration
    if (m_bUseD3DAlloc)
    {
        m_pVideoParams->IOPattern = MFX_IOPATTERN_OUT_VIDEO_MEMORY | MFX_IOPATTERN_IN_VIDEO_MEMORY;
        int nAdapterID = GetMSDKAdapterNumber(*m_mfxVideoSession);
        // D3D11 HW device
        if (m_bUseD3D11Alloc)
        {
#if MFX_D3D11_SUPPORT
            // HW device must be initialized early - within session init.
            // If a call to DecodeHeader was called before session->SetHandle, SetHandle would fail.
            ASSERT(m_HwDevice);
            MSDK_CHECK_POINTER(m_HwDevice, MFX_ERR_NULL_PTR);

            D3D11AllocatorParams* p = new D3D11AllocatorParams;
            p->pDevice = (ID3D11Device*)m_HwDevice->GetHandle(MFX_HANDLE_D3D11_DEVICE);
            pParam.reset(p);
            m_pFrameAllocator = new D3D11FrameAllocator();
#endif
        }
        // D3D9 HW device
        else
        {
            // Having the D3D9 device manager from the renderer allows working in full screen exclusive mode
            // This parameter can be NULL for other usages
            m_HwDevice = new CD3D9Device(m_pRendererD3dDeviceManager);
            if (MSDK_FAILED(sts = m_HwDevice->Init(nAdapterID)))
            {
                MSDK_TRACE("QsDecoder: D3D9 init have failed!\n");
                MSDK_SAFE_DELETE(m_HwDevice);
                return sts;
            }

            // Set the pointer to the HW device (or device manager) to the session
            mfxHDL h = m_HwDevice->GetHandle(MFX_HANDLE_D3D9_DEVICE_MANAGER);
            sts = m_mfxVideoSession->SetHandle(MFX_HANDLE_D3D9_DEVICE_MANAGER, h);
            MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, sts);

            D3DAllocatorParams* p = new D3DAllocatorParams;
            p->pManager = (IDirect3DDeviceManager9*)h;
            pParam.reset(p);
            m_pFrameAllocator = new D3DFrameAllocator();
        }
    }
    // Setup allocator - No HW acceleration
    else
    {
        m_bUseD3DAlloc = false;
        m_pVideoParams->IOPattern = MFX_IOPATTERN_OUT_SYSTEM_MEMORY | MFX_IOPATTERN_IN_SYSTEM_MEMORY;
        m_pFrameAllocator = new SysMemFrameAllocator();
    }

    sts = m_pFrameAllocator->Init(pParam.get());
    if (MSDK_SUCCEEDED(sts))
    {
        // Note - setting the session allocator can be done only once per session!
        sts = m_mfxVideoSession->SetFrameAllocator(m_pFrameAllocator);
        if (MSDK_FAILED(sts))
        {
            MSDK_TRACE("QsDecoder: Session SetFrameAllocator failed!\n");    
        }
    }
    else
    // Allocator failed to initialize
    {
        MSDK_TRACE("QsDecoder: Allocator Init failed!\n");

        MSDK_SAFE_DELETE(m_pFrameAllocator);
        ASSERT(false);
    }

    return sts;
}

mfxStatus CQuickSyncDecoder::LockFrame(mfxFrameSurface1* pSurface, mfxFrameData* pFrameData)
{
    MSDK_CHECK_POINTER(pSurface, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(pFrameData, MFX_ERR_NULL_PTR);
    return m_pFrameAllocator->Lock(m_pFrameAllocator, pSurface->Data.MemId, pFrameData);
}

mfxStatus CQuickSyncDecoder::UnlockFrame(mfxFrameSurface1* pSurface, mfxFrameData* pFrameData)
{
    MSDK_CHECK_POINTER(pSurface, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(pFrameData, MFX_ERR_NULL_PTR);
    return m_pFrameAllocator->Unlock(m_pFrameAllocator, pSurface->Data.MemId, pFrameData);
}

IDirect3DDeviceManager9* CQuickSyncDecoder::GetD3DDeviceManager()
{
    if (!m_bUseD3D11Alloc && m_HwDevice != NULL)
    {
        return (IDirect3DDeviceManager9*)m_HwDevice->GetHandle(MFX_HANDLE_DIRECT3D_DEVICE_MANAGER9);
    }

    return NULL;
}

bool CQuickSyncDecoder::SetD3DDeviceManager(IDirect3DDeviceManager9* pDeviceManager)
{
    if (m_pRendererD3dDeviceManager == pDeviceManager)
        return false;

    MSDK_TRACE("QsDecoder: SetD3DDeviceManager called\n");
    m_pRendererD3dDeviceManager = pDeviceManager;
    return true;
}

void CQuickSyncDecoder::SetAuxFramesCount(size_t count)
{
    if (m_nAuxFrameCount != count)
    {
        m_nAuxFrameCount = (mfxU16)count;
        FreeFrameAllocator();
    }
}
