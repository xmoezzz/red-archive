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
#include "CodecInfo.h"
#include "TimeManager.h"
#include "QuickSyncUtils.h"
#include "frame_constructors.h"
#include "QuickSyncDecoder.h"
#include "QuickSyncVPP.h"
#include "QuickSync.h"

#define PAGE_MASK 4095

EXTERN_GUID(WMMEDIASUBTYPE_WVC1, 
0x31435657, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71); 
EXTERN_GUID(WMMEDIASUBTYPE_WMV3, 
0x33564D57, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71); 

DEFINE_GUID(WMMEDIASUBTYPE_WVC1_PDVD,
0xD979F77B, 0xDBEA, 0x4BF6, 0x9E, 0x6D, 0x1D, 0x7E, 0x57, 0xFB, 0xAD, 0x53);

////////////////////////////////////////////////////////////////////
//                      CQuickSync
////////////////////////////////////////////////////////////////////
CQuickSync::CQuickSync() :
    m_bInitialized(false),
    m_pVPP(NULL),
    m_nPitch(0),
    m_pFrameConstructor(NULL),
    m_nSegmentFrameCount(0),
    m_bFlushing(false),
    m_bNeedToFlush(false),
    m_bDvdDecoding(false),
    m_PicStruct(0),
    m_SurfaceType(QS_SURFACE_SYSTEM),
    m_ProcessedFrame(new QsFrameData, new CQsAlignedBuffer(0)
    )
{
    MSDK_TRACE("QsDecoder: Constructor\n");
    strcpy_s(m_CodecName, "Intel\xae QuickSync Decoder");

    mfxStatus sts = MFX_ERR_NONE;

    MSDK_ZERO_VAR(m_DecVideoParams);
    //m_DecVideoParams.AsyncDepth = 0; // Causes issues when 1 - in old drivers
    m_DecVideoParams.mfx.ExtendedPicStruct = 1;
//
// Set default configuration - override what's not zero/false
//
    m_Config.bTimeStampCorrection = true;

    m_Config.nOutputQueueLength = 8;
    m_Config.bEnableH264  = true;
    m_Config.bEnableMPEG2 = true;
    m_Config.bEnableVC1   = true;
    m_Config.bEnableWMV9  = true;

    m_Config.bEnableMultithreading = true;
    m_Config.bEnableMtCopy         = m_Config.bEnableMultithreading;

    // For testing purposes only - menus are not handled well
//    m_Config.bEnableSwEmulation  = true;

    // Currently not working well - menu decoding :(
//    m_Config.bEnableDvdDecoding = true;

    // Force field order
//    m_Config.bForceFieldOrder = true;
//    m_Config.eFieldOrder = QS_FIELD_TFF;

    // VPP - Video Post Processing
    m_Config.bEnableVideoProcessing  = true;
    m_Config.bVppEnableDeinterlacing = true;
    m_Config.bVppEnableFullRateDI    = true;
    m_Config.bVppEnableDITimeStampsInterpolation = true;
//    m_Config.bVppEnableForcedDeinterlacing = true;
//    m_Config.nVppDetailStrength      = 16; // 0-64
//    m_Config.nVppDenoiseStrength     = 16; // 0-64

//    m_Config.bDropDuplicateFrames = true;

    DWORD dwVersion = GetVersion();
    DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    DWORD dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
    // No QS support for Windows version older than Vista
    if (dwMajorVersion < 6)
    {
        sts = MFX_ERR_UNSUPPORTED;
        return;
    }
    // D3D11 decode was introduced with Win8 (Windows 6.2)
    // Window Vista and 7 can use D3D9
    else if ((dwMajorVersion == 6 && dwMinorVersion >= 2) || (dwMajorVersion > 6))
    {
#if MFX_D3D11_SUPPORT
        m_Config.bEnableD3D11 = true;
//        m_Config.bDefaultToD3D11 = true;
#endif
    }
    m_pDecoder = new CQuickSyncDecoder(m_Config, sts);

    m_OK = MSDK_SUCCEEDED(sts);
}

CQuickSync::~CQuickSync()
{
    MSDK_TRACE("QsDecoder: Destructor\n");
    
    // This will quicken the exit 
    BeginFlush();

    CQsAutoLock cObjectLock(&m_csLock);

    delete m_ProcessedFrame.first;
    delete m_ProcessedFrame.second;

    MSDK_SAFE_DELETE(m_pFrameConstructor);
    MSDK_SAFE_DELETE(m_pDecoder);
}

HRESULT CQuickSync::HandleSubType(const AM_MEDIA_TYPE* mtIn, FOURCC fourCC, mfxVideoParam& videoParams, CFrameConstructor*& pFrameConstructor)
{
    const GUID& subType = mtIn->subtype;
    const GUID& guidFormat = mtIn->formattype;

    MSDK_SAFE_DELETE(pFrameConstructor);

    // MPEG2
    if (subType == MEDIASUBTYPE_MPEG2_VIDEO)
    {
        if (!m_Config.bEnableMPEG2)
            return VFW_E_INVALIDMEDIATYPE;

        videoParams.mfx.CodecId = MFX_CODEC_MPEG2;
        pFrameConstructor = new CFrameConstructor(&m_TimeManager);
    }    
    // VC1 or WMV3
    else if ((fourCC == FOURCC_VC1) || (fourCC == FOURCC_WMV3))
    {
        videoParams.mfx.CodecId = MFX_CODEC_VC1;

        if (subType == WMMEDIASUBTYPE_WMV3)
        {
            videoParams.mfx.CodecProfile = MFX_PROFILE_VC1_SIMPLE;        
            if (!m_Config.bEnableWMV9)
                return VFW_E_INVALIDMEDIATYPE;
        }   
        else if (fourCC == FOURCC_VC1)
        {
            videoParams.mfx.CodecProfile = MFX_PROFILE_VC1_ADVANCED;
            if (!m_Config.bEnableVC1)
                return VFW_E_INVALIDMEDIATYPE;
        }
        else
        {
            return VFW_E_INVALIDMEDIATYPE;
        }

        pFrameConstructor = new CVC1FrameConstructor(&m_TimeManager);
    }
    // H264
    else if ((fourCC == FOURCC_H264) || (fourCC == FOURCC_X264) || (fourCC == FOURCC_h264) ||
        (fourCC == FOURCC_avc1) || (fourCC == FOURCC_VSSH) || (fourCC == FOURCC_DAVC) ||
        (fourCC == FOURCC_PAVC) || (fourCC == FOURCC_AVC1) || (fourCC == FOURCC_CCV1))
    {
        if (!m_Config.bEnableH264)
            return VFW_E_INVALIDMEDIATYPE;

        videoParams.mfx.CodecId = MFX_CODEC_AVC;
        // Note: CAVCFrameConstructor can handle both H264 stream types but it can't handle fragment streams (e.g. live TV)
        pFrameConstructor = ((fourCC == FOURCC_avc1) || (fourCC == FOURCC_AVC1) || (fourCC == FOURCC_CCV1)) ?
            new CAVCFrameConstructor(&m_TimeManager) :
            new CFrameConstructor(&m_TimeManager);
//        pFrameConstructor = new CAVCFrameConstructor(&m_TimeManager);
    }
    else
    {
        return E_NOTIMPL;
    }

    // Check if codec profile and level are supported before calling DecodeHeader
    mfxInfoMFX& mfx = videoParams.mfx;
    if (FORMAT_MPEG2_VIDEO == guidFormat)
    {
        MPEG2VIDEOINFO* mp2 = (MPEG2VIDEOINFO*)(mtIn->pbFormat);
        HRESULT hr = CheckCodecProfileSupport(mfx.CodecId, mp2->dwProfile);
        if (hr != S_OK)
        {
            MSDK_TRACE("QsDecoder::InitDecoder - failed due to unsupported codec (%s), profile (%s), level (%i) combination\n",
                ::GetCodecName(mfx.CodecId), GetProfileName(mfx.CodecId, mp2->dwProfile), mp2->dwLevel);
            return E_NOTIMPL;
        }
        else
        {
            MSDK_TRACE("QsDecoder::InitDecoder - codec (%s), profile (%s), level (%i)\n",
                ::GetCodecName(mfx.CodecId), GetProfileName(mfx.CodecId, mp2->dwProfile), mp2->dwLevel);
        }
    }
    else
    {
        MSDK_TRACE("QsDecoder::InitDecoder - codec (%s)\n", ::GetCodecName(mfx.CodecId));
    }

    return S_OK;
}

HRESULT CQuickSync::TestMediaType(const AM_MEDIA_TYPE* mtIn, FOURCC fourCC)
{
    MSDK_TRACE("QsDecoder: TestMediaType\n");
    if (!m_OK)
    {
        MSDK_TRACE("QsDecoder: TestMediaType was called on an invalid object!\n");
        return E_FAIL;
    }
    
    // Parameter check
    MSDK_CHECK_POINTER(mtIn, E_POINTER);
    MSDK_CHECK_POINTER(mtIn->pbFormat, E_UNEXPECTED);

    if (mtIn->majortype != MEDIATYPE_Video && mtIn->majortype != MEDIATYPE_DVD_ENCRYPTED_PACK)
    {
        MSDK_TRACE("QsDecoder: Invalid majortype GUID!\n");
        return E_FAIL;
    }
    
    VIDEOINFOHEADER2* vih2 = NULL;
    size_t nSampleSize = 0, nVideoInfoSize = 0;
    mfxVideoParam videoParams;
    MSDK_ZERO_VAR(videoParams);
    CFrameConstructor* pFrameConstructor = NULL;
    HRESULT hr = DecodeHeader(mtIn, fourCC, pFrameConstructor, vih2, nSampleSize, nVideoInfoSize, videoParams);
    MSDK_SAFE_DELETE(pFrameConstructor);
    MSDK_CHECK_RESULT_P_RET(hr, S_OK);

    // If SW emulation is disabled - check if HW acceleration is supported
    if (!m_Config.bEnableSwEmulation)
    {
        mfxStatus sts = m_pDecoder->CheckHwAcceleration(&videoParams);
        if (MSDK_FAILED(sts))
        {
            MSDK_TRACE("HW accelration is not supported. Aborting!\n");
            hr = E_FAIL;
        }
    }

    delete[] (mfxU8*)vih2;
    MSDK_TRACE("QsDecoder: TestMediaType finished: %s\n", (SUCCEEDED(hr)) ? "success" : "failure");
    return hr;
}

HRESULT CQuickSync::CopyMediaTypeToVIDEOINFOHEADER2(const AM_MEDIA_TYPE* mtIn, VIDEOINFOHEADER2*& vih2, size_t& nVideoInfoSize, size_t& nSampleSize)
{
    vih2 = NULL;
    const GUID& guidFormat = mtIn->formattype;
    nVideoInfoSize = sizeof(VIDEOINFOHEADER2);

    if (FORMAT_VideoInfo2 == guidFormat || FORMAT_MPEG2_VIDEO == guidFormat)
    {
        if (FORMAT_MPEG2_VIDEO == guidFormat)
        {
            nVideoInfoSize = sizeof(MPEG2VIDEOINFO);
        }

        nSampleSize = mtIn->cbFormat;
        vih2 = (VIDEOINFOHEADER2*) new mfxU8[nSampleSize];

        // Copy the sample as is
        memcpy(vih2, mtIn->pbFormat, nSampleSize);
    }
    // Translate VIDEOINFOHEADER to VIDEOINFOHEADER2 (easier to work with down the road)
    else if (FORMAT_VideoInfo == guidFormat)
    {
        size_t extraDataLen = mtIn->cbFormat - sizeof(VIDEOINFOHEADER);
        nSampleSize = sizeof(VIDEOINFOHEADER2) + extraDataLen;
        vih2 = (VIDEOINFOHEADER2*) new mfxU8[nSampleSize];
        MSDK_ZERO_MEMORY(vih2, sizeof(VIDEOINFOHEADER2));

        // Initialize the VIDEOINFOHEADER2 structure
        VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*)(mtIn->pbFormat);
        vih2->rcSource           = pvi->rcSource;
        vih2->rcTarget           = pvi->rcTarget;
        vih2->dwBitRate          = pvi->dwBitRate;
        vih2->dwBitErrorRate     = pvi->dwBitErrorRate;
        vih2->bmiHeader          = pvi->bmiHeader;
        vih2->AvgTimePerFrame    = pvi->AvgTimePerFrame;
        vih2->dwPictAspectRatioX = pvi->bmiHeader.biWidth;
        vih2->dwPictAspectRatioY = pvi->bmiHeader.biHeight;

        // Copy the out of band data (data past the VIDEOINFOHEADER structure.
        // Note: copy past the size of vih2
        if (extraDataLen)
        {
            memcpy(((mfxU8*)vih2) + nVideoInfoSize, mtIn->pbFormat + sizeof(VIDEOINFOHEADER), extraDataLen);
        }
    }
    else
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

    return S_OK;
}

HRESULT CQuickSync::DecodeHeader(
    const AM_MEDIA_TYPE* mtIn,
    FOURCC fourCC,
    CFrameConstructor*& pFrameConstructor,
    VIDEOINFOHEADER2*& vih2,
    size_t nSampleSize,
    size_t& nVideoInfoSize,
    mfxVideoParam& videoParams)
{
    mfxInfoMFX& mfx = videoParams.mfx;
    const GUID& guidFormat = mtIn->formattype;
    HRESULT hr = S_OK;
    mfxStatus sts = MFX_ERR_NONE;
     
    hr = HandleSubType(mtIn, fourCC, videoParams, pFrameConstructor);
    MSDK_CHECK_RESULT_P_RET(hr, S_OK);

    hr = CopyMediaTypeToVIDEOINFOHEADER2(mtIn, vih2, nVideoInfoSize, nSampleSize);
    MSDK_CHECK_RESULT_P_RET(hr, S_OK);

    if (MEDIATYPE_DVD_ENCRYPTED_PACK == mtIn->majortype)
    {
        if (!m_Config.bEnableDvdDecoding)
            return VFW_E_INVALIDMEDIATYPE;

        pFrameConstructor->SetDvdPacketStripping(true);
        m_bDvdDecoding = true;
    }

    if (!vih2->bmiHeader.biWidth || !vih2->bmiHeader.biHeight)
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

    // Construct sequence header and decode the sequence header.
    if (nVideoInfoSize < nSampleSize)
    {
        sts = pFrameConstructor->ConstructHeaders(vih2, guidFormat, nSampleSize, nVideoInfoSize);
        if (MSDK_SUCCEEDED(sts))
        {
            sts = m_pDecoder->DecodeHeader(&pFrameConstructor->GetHeaders(), &videoParams);
            if (MSDK_FAILED(sts))
            {
                MSDK_TRACE("QsDecoder: Warning DecodeHeader failed!\n");
//            ASSERT(MSDK_SUCCEEDED(sts));
            }
        }
    }
    
    // Simple info header (without extra data) or DecodeHeader failed (usually not critical, in many cases header will appear later in the stream)
    if (nVideoInfoSize == nSampleSize || MSDK_FAILED(sts))
    {
        mfx.FrameInfo.CropW        = (mfxU16)vih2->bmiHeader.biWidth;
        mfx.FrameInfo.CropH        = (mfxU16)vih2->bmiHeader.biHeight;
        mfx.FrameInfo.Width        = (mfxU16)MSDK_ALIGN16(mfx.FrameInfo.CropW);
        mfx.FrameInfo.Height       = (mfxU16)MSDK_ALIGN16(mfx.FrameInfo.CropH);
        mfx.FrameInfo.FourCC       = MFX_FOURCC_NV12;
        mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
        ConvertFrameRate((vih2->AvgTimePerFrame) ? 1e7 / vih2->AvgTimePerFrame : 0, 
            mfx.FrameInfo.FrameRateExtN, 
            mfx.FrameInfo.FrameRateExtD);
        
        // Get codec/profile from MPEG2VIDEOINFO header
        if (nVideoInfoSize == sizeof(MPEG2VIDEOINFO))
        {
            if (mfx.CodecId == MFX_CODEC_AVC)
            {
                MPEG2VIDEOINFO* mp2 = (MPEG2VIDEOINFO*)(vih2);
                mfx.CodecProfile = (mfxU16)mp2->dwProfile;
                mfx.CodecLevel   = (mfxU16)mp2->dwLevel;
            }
        }

        sts = MFX_ERR_NONE;
    }

    hr = (MSDK_SUCCEEDED(sts)) ? S_OK : E_FAIL;

    if (FAILED(hr))
    {
        delete[] (mfxU8*)vih2;
        vih2 = NULL;
    }
    return hr;
}

HRESULT CQuickSync::InitDecoder(const AM_MEDIA_TYPE* mtIn, FOURCC fourCC)
{
    MSDK_TRACE("QsDecoder: InitDecoder\n");
    CQsAutoLock cObjectLock(&m_csLock);

    // 2nd initialization - empty queues and kill VPP
    if (m_bInitialized)
    {
        OnSeek(0);
        MSDK_ZERO_VAR(m_DecVideoParams.mfx.FrameInfo);
    }

    m_bNeedToFlush = true;
    m_bInitialized = true;

    VIDEOINFOHEADER2* vih2 = NULL;
    mfxStatus sts = MFX_ERR_NONE;
    mfxInfoMFX& mfx = m_DecVideoParams.mfx;
    HRESULT hr;
    MSDK_CHECK_POINTER(mtIn, E_POINTER);
    MSDK_CHECK_POINTER(mtIn->pbFormat, E_UNEXPECTED);
    bool bIsFields = false;
    size_t nSampleSize = 0, nVideoInfoSize = 0;

    if (!(mtIn->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK || mtIn->majortype == MEDIATYPE_Video))
        return VFW_E_INVALIDMEDIATYPE;

    // Delete frame constructor from previous run
    MSDK_SAFE_DELETE(m_pFrameConstructor);
    hr = DecodeHeader(mtIn, fourCC, m_pFrameConstructor, vih2, nSampleSize, nVideoInfoSize, m_DecVideoParams);

    // Setup frame rate from either the media type or the decoded header
    bIsFields = (vih2->dwInterlaceFlags & AMINTERLACE_IsInterlaced) &&
        (vih2->dwInterlaceFlags & AMINTERLACE_1FieldPerSample);

    // Try getting the frame rate from the decoded headers
    if (0 < vih2->AvgTimePerFrame)
    {
        m_TimeManager.SetFrameRate(1e7 / vih2->AvgTimePerFrame, bIsFields);

        // Workaround for buggy SDK frame manipulation when H264 SPS header contains a zero frame rate
        if (0 == mfx.FrameInfo.FrameRateExtN * mfx.FrameInfo.FrameRateExtD)
        {
            ConvertFrameRate((vih2->AvgTimePerFrame) ? 1e7 / vih2->AvgTimePerFrame : 0, 
                mfx.FrameInfo.FrameRateExtN, 
                mfx.FrameInfo.FrameRateExtD);
        }
    }
    // In case we don't have a frame rate
    else if (mfx.FrameInfo.FrameRateExtN * mfx.FrameInfo.FrameRateExtD != 0)
    {
        double frameRate = (double)mfx.FrameInfo.FrameRateExtN / (double)mfx.FrameInfo.FrameRateExtD;
        m_TimeManager.SetFrameRate(frameRate, bIsFields);
    }

    // We might decode well even if DecodeHeader failed with MFX_ERR_MORE_DATA
    MSDK_IGNORE_MFX_STS(sts, MFX_ERR_MORE_DATA);

    m_nPitch = (mfxU16)MSDK_ALIGN32(mfx.FrameInfo.Width);
    mfx.FrameInfo.Height = (mfxU16)MSDK_ALIGN32(mfx.FrameInfo.Height);

    SetAspectRatio(*vih2, mfx.FrameInfo);

    // Disable MT features if main flag is off
    m_Config.bEnableMtCopy = m_Config.bEnableMtCopy && m_Config.bEnableMultithreading;

    // Video processing
    if (m_Config.bEnableVideoProcessing)
    {
        // If forced DI is enabled, DI is enabled.
        // DI is auto enabled when detail/denoise are on as these work on progressive frames.
        if (m_Config.bVppEnableForcedDeinterlacing || m_Config.nVppDetailStrength > 0 || m_Config.nVppDenoiseStrength> 0)
        {
            m_Config.bVppEnableDeinterlacing = true;
        }

        if (m_Config.bVppEnableForcedDeinterlacing)
        {
            MSDK_TRACE("QsDecoder: forced deinterlacing is active\n");
        }
        else if (m_Config.bVppEnableDeinterlacing)
        {
            MSDK_TRACE("QsDecoder: auto deinterlacing is active\n");
        }

        if (m_Config.nVppDetailStrength > 0)
        {
            MSDK_TRACE("QsDecoder: detail filter is at %i%%\n", (32 + 100 * m_Config.nVppDetailStrength) / 64);
        }

        if (m_Config.nVppDenoiseStrength > 0)
        {
            MSDK_TRACE("QsDecoder: denoise filter is at %i%%\n", (32 + 100 * m_Config.nVppDenoiseStrength) / 64);
        }
    }
    // Make sure all VPP features are disabled
    else
    {
        m_Config.vpp = 0;
    }

    if (m_Config.bForceFieldOrder)
    {
        switch (m_Config.eFieldOrder)
        {
        case QS_FIELD_AUTO:
            MSDK_TRACE("QsDecoder: field order is set to AUTO\n");
            break;
        case QS_FIELD_TFF:
            MSDK_TRACE("QsDecoder: field order is set to TFF\n");
            break;
        case QS_FIELD_BFF:
            MSDK_TRACE("QsDecoder: field order is set to BFF\n");
            break;
        default:
            MSDK_TRACE("QsDecoder: invalid field order parameter reverting to AUTO!\n");
            m_Config.eFieldOrder = QS_FIELD_AUTO;
        }
    }

    // Initialization of Media SDK decoder is done in OnSeek to allow late initialization needed
    // by full screen exclusive (FSE) mode since D3D devices can't be created. The DS filter must send
    // the D3D device manager to this decoder for surface allocation.
    if (MSDK_SUCCEEDED(sts))
    {
        m_pDecoder->SetConfig(m_Config);
        size_t surfaceCount = max(8, m_Config.nOutputQueueLength);

        if (m_Config.bVppEnableDeinterlacing)
        {
            surfaceCount += 5;
        }

        m_pDecoder->SetAuxFramesCount(surfaceCount);
    }

    m_TimeManager.Enabled() = m_Config.bTimeStampCorrection;

    _snprintf_s(m_CodecName, MSDK_ARRAY_LEN(m_CodecName), MSDK_ARRAY_LEN(m_CodecName)-1,
        "Intel\xae QuickSync Decoder (%s) - %s",
        ::GetCodecName(m_DecVideoParams.mfx.CodecId),
        (m_pDecoder->IsHwAccelerated()) ? (m_pDecoder->IsD3D11Alloc() ? "HW D3D11" : "HW D3D9" ) : "SW"
        );

    delete[] (mfxU8*)vih2;
    m_OK = MSDK_SUCCEEDED(sts);
    return (m_OK) ? S_OK : E_FAIL;
}

void CQuickSync::SetAspectRatio(VIDEOINFOHEADER2& vih2, mfxFrameInfo& frameInfo)
{
    // Fix small aspect ratio errors
    if (MSDK_ALIGN16(vih2.dwPictAspectRatioX) == frameInfo.CropW)
    {
        vih2.dwPictAspectRatioX = frameInfo.CropW;
    }

    // AR is not always contained in the header (it's optional for MFX decoder initialization though)
    if (frameInfo.AspectRatioW * frameInfo.AspectRatioH == 0)
    {
        // Convert display aspect ratio (is in VIDEOINFOHEADER2) to pixel aspect ratio (is accepted by MediaSDK components)
        mfxStatus sts = DARtoPAR(vih2.dwPictAspectRatioX, vih2.dwPictAspectRatioY, 
            frameInfo.CropW, frameInfo.CropH,
            frameInfo.AspectRatioW, frameInfo.AspectRatioH);

        if (MSDK_FAILED(sts))
        {
            frameInfo.AspectRatioW = frameInfo.AspectRatioH = 1;
        }
    }
}

HRESULT CQuickSync::Decode(IMediaSample* pSample)
{
    // DEBUG
#ifdef _DEBUG
    static bool isSetThreadName = false;
    if (!isSetThreadName) 
    {
        isSetThreadName = true;
        SetThreadName("*** Decoder ***");
    }
#endif
    
    // Input samples should be discarded - we are in the middle of a flush:
    // between BeginFlush and EndFlush events.
    if (m_bFlushing)
        return S_FALSE;
    
    CQsAutoLock cObjectLock(&m_csLock);

    MSDK_VTRACE("QsDecoder: Decode\n");

    // We haven't flushed since the last BeginFlush call - probably a DVD playback scenario
    // where NewSegment/OnSeek isn't issued.
    if (m_bNeedToFlush)
    {
        if (FAILED(OnSeek(0)))
            return E_FAIL;
    }

    MSDK_CHECK_NOT_EQUAL(m_OK, true, E_UNEXPECTED);
    HRESULT hr = S_OK;
    mfxStatus sts = MFX_ERR_NONE;
    mfxFrameSurface1* pSurfaceOut = NULL;
    mfxBitstream mfxBS; 
    MSDK_ZERO_VAR(mfxBS);

    // Check error(s)
    if (NULL == pSample)
    {
        return S_OK;
    }

    if (0 == pSample->GetActualDataLength())
    {
        return S_FALSE;
    }

    // Manipulate bitstream for decoder
    sts = m_pFrameConstructor->ConstructFrame(pSample, &mfxBS);
    ASSERT(mfxBS.DataLength <= mfxBS.MaxLength);
    MSDK_CHECK_ERROR(sts, MFX_ERR_MORE_DATA, S_OK); // not an error
    MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, E_FAIL);
    bool flushed = false;

    // Decode mfxBitstream until all data is taken by decoder
    while (mfxBS.DataLength > 0 && !m_bNeedToFlush)
    {
        // Decode the bitstream
        sts = m_pDecoder->Decode(&mfxBS, pSurfaceOut);                

        if (MSDK_SUCCEEDED(sts))
        {
            // Sync decode - pSurfaceOut holds decoded surface
            if (NULL != pSurfaceOut)
            {
                // Queue the frame for processing
                ProcessDecodedFrame(pSurfaceOut);
            }

            continue;
        }
        else if (MFX_ERR_MORE_DATA == sts)
        {
            break; // Need to receive more data from upstream filter
        }
        else if (MFX_WRN_VIDEO_PARAM_CHANGED == sts)
        {
            MSDK_TRACE("QsDecoder: Decode MFX_WRN_VIDEO_PARAM_CHANGED\n");

            // Need to handle a possible change in the stream parameters
            if (!flushed)
            {
                sts = OnVideoParamsChanged();
                flushed = true;
            }

            continue; // Just continue processing
        }
        // Need another work surface
        else if (MFX_ERR_MORE_SURFACE == sts)
        {
            // Just continue, new work surface will be found in m_pDecoder->Decode
            continue;
        }
        else if (MFX_ERR_NOT_ENOUGH_BUFFER == sts)
        {
            MSDK_TRACE("QsDecoder: Error - ran out of work buffers!\n");
            // This is a system malfunction!
            break;
        }
        else if (MFX_ERR_INCOMPATIBLE_VIDEO_PARAM == sts)
        {
            MSDK_TRACE("QsDecoder: Decode MFX_ERR_INCOMPATIBLE_VIDEO_PARAM\n");

            // Flush existing frames
            Flush(true);

            // Destroy VPP object
            MSDK_SAFE_DELETE(m_pVPP);

            // Retrieve new parameters
            mfxVideoParam VideoParams;
            MSDK_ZERO_VAR(VideoParams);
            VideoParams.mfx.CodecId = m_DecVideoParams.mfx.CodecId;
            sts = m_pDecoder->DecodeHeader(&mfxBS, &VideoParams); 
            if (MFX_ERR_MORE_DATA == sts)
            {
                break;
            }

            // Save IOPattern and update parameters
            VideoParams.IOPattern = m_DecVideoParams.IOPattern; 
            memcpy(&m_DecVideoParams, &VideoParams, sizeof(mfxVideoParam));
            m_nPitch = MSDK_ALIGN32(m_DecVideoParams.mfx.FrameInfo.Width);
            sts = m_pDecoder->Reset(&m_DecVideoParams, m_nPitch);
            if (MSDK_SUCCEEDED(sts))
            {
                continue;
            }

            MSDK_TRACE("QsDecoder: Decode didn't recover from MFX_ERR_INCOMPATIBLE_VIDEO_PARAM\n");
        }
        else
        {
            MSDK_TRACE("QsDecoder: Device failed!\n");
        }

        // The decoder has returned with an error
        hr = E_FAIL;
        break;
    }

    if (SUCCEEDED(hr) && !m_bNeedToFlush)
        m_pFrameConstructor->SaveResidualData(&mfxBS);
    
    MSDK_SAFE_DELETE_ARRAY(mfxBS.Data);

    return hr;
}

void CQuickSync::DeliverSurface(mfxFrameSurface1* pSurface, int duplicates)
{
    MSDK_VTRACE("QsDecoder: DeliverSurface\n");
    MSDK_CHECK_POINTER_NO_RET(pSurface);

    duplicates = min(1, duplicates);

    QsFrameData& outFrameData = *m_ProcessedFrame.first;
    CQsAlignedBuffer*& pOutBuffer = m_ProcessedFrame.second;

    // Clear the outFrameData
    MSDK_ZERO_VAR(outFrameData);

    outFrameData.bCorrupted = pSurface->Data.Corrupted != 0;
    UpdateAspectRatio(pSurface, outFrameData);
    outFrameData.fourCC = pSurface->Info.FourCC;

    // Setup interlacing info
    PicStructToDsFlags(pSurface->Info.PicStruct, outFrameData.dwInterlaceFlags, outFrameData.frameStructure);
    outFrameData.bFilm = 0 != (outFrameData.dwInterlaceFlags & AM_VIDEO_FLAG_REPEAT_FIELD);

    // Time stamp
    outFrameData.rtStart = m_TimeManager.ConvertMFXTime2ReferenceTime(pSurface->Data.TimeStamp);
    outFrameData.rtStop = (outFrameData.rtStart == INVALID_REFTIME) ? INVALID_REFTIME : (outFrameData.rtStart + 1);
    
    // TODO: find actual frame type I/P/B
    // Media sample isn't reliable as it referes to an older frame!
    outFrameData.frameType = QsFrameData::I;

    // Obtain surface data and copy it to temp buffer.
    mfxFrameData frameData;
    m_pDecoder->LockFrame(pSurface, &frameData);

    size_t height = pSurface->Info.CropH; // Cropped image height

    // Fill image size
    outFrameData.rcFull.top    = outFrameData.rcFull.left = 0;
    outFrameData.rcFull.bottom = (LONG)height - 1;
    outFrameData.rcFull.right  = MSDK_ALIGN16(pSurface->Info.CropW + pSurface->Info.CropX) - 1;
    outFrameData.rcClip.top    = 0;
    outFrameData.rcClip.bottom = (LONG)height - 1; // Height is not padded in output buffer

    // Note that we always crop the height
    outFrameData.rcClip.left  = pSurface->Info.CropX;
    outFrameData.rcClip.right = pSurface->Info.CropW + pSurface->Info.CropX - 1;

    outFrameData.dwStride = frameData.Pitch;

    if (m_bNeedToFlush) return;

    if (m_Config.bDropDuplicateFrames) duplicates = 1;


    for (int i = 0; i < duplicates && !m_bNeedToFlush; ++i) 
    {
        // Fix time stamps for duplicated frames
        if (outFrameData.rtStart != INVALID_REFTIME && pSurface->Info.FrameRateExtN > 0)
        {
            outFrameData.rtStart += (REFERENCE_TIME)(0.5 + (1e7 * i) * (double)pSurface->Info.FrameRateExtD / (double)pSurface->Info.FrameRateExtN);
            outFrameData.rtStop = outFrameData.rtStart + 1;
        }

        switch (m_SurfaceType)
        {
        case QS_SURFACE_GPU:
            CopyFramePointers(pSurface, outFrameData, frameData);
            break;

        case QS_SURFACE_SYSTEM:
        default:
            // Copy to output surface and write metadata
            CopyFrame(pSurface, outFrameData, pOutBuffer, frameData);
        }

        if (!m_bNeedToFlush)
        {
            // Send the surface out - return code from dshow filter is ignored.
            MSDK_VTRACE("QsDecoder: DeliverSurfaceCallback (%I64d)\n", outFrameData.rtStart);
            m_DeliverSurfaceCallback(m_ObjParent, &outFrameData);
        }
    }
    
    // Unlock the frame
    m_pDecoder->UnlockFrame(pSurface, &frameData);
}

void CQuickSync::PicStructToDsFlags(mfxU32 picStruct, DWORD& flags, QsFrameData::QsFrameStructure& frameStructure)
{
    // Note: MSDK will never output fields
    frameStructure = (picStruct & MFX_PICSTRUCT_PROGRESSIVE) ?
        QsFrameData::fsProgressiveFrame :
        QsFrameData::fsInterlacedFrame;

    if (m_TimeManager.GetInverseTelecine())
    {
        flags = AM_VIDEO_FLAG_WEAVE;
        return;
    }

    // Progressive frame - note that sometimes interlaced content has the MFX_PICSTRUCT_PROGRESSIVE in combination with other flags
    // Frame doubling/tripling implies progressive source
    if (picStruct == MFX_PICSTRUCT_PROGRESSIVE || picStruct & MFX_PICSTRUCT_FRAME_DOUBLING || picStruct & MFX_PICSTRUCT_FRAME_TRIPLING)
    {
        flags = AM_VIDEO_FLAG_WEAVE;
        return;
    }

    // Interlaced
    flags = 0;

    // Top field first
    if (picStruct & MFX_PICSTRUCT_FIELD_TFF)
    {
        flags |= AM_VIDEO_FLAG_FIELD1FIRST;
    }

    // frame is progressive.
    if (picStruct & MFX_PICSTRUCT_PROGRESSIVE)
    {
        flags |= AM_VIDEO_FLAG_WEAVE;
    }

    // Telecine flag
    if (picStruct & MFX_PICSTRUCT_FIELD_REPEATED)
    {
        flags |= AM_VIDEO_FLAG_REPEAT_FIELD;
    }
}

mfxStatus CQuickSync::ConvertFrameRate(mfxF64 dFrameRate, mfxU32& nFrameRateExtN, mfxU32& nFrameRateExtD)
{
    mfxU32 fr = (mfxU32)(dFrameRate + .5);
    if (fabs(fr - dFrameRate) < 0.0001) 
    {
        nFrameRateExtN = fr;
        nFrameRateExtD = (nFrameRateExtN) ? 1 : 0;
        return MFX_ERR_NONE;
    }

    fr = (mfxU32)(dFrameRate * 1.001 + .5);

    if (fabs(fr * 1000 - dFrameRate * 1001) < 10) 
    {
        nFrameRateExtN = fr * 1000;
        nFrameRateExtD = 1001;
        return MFX_ERR_NONE;
    }

    nFrameRateExtN = (mfxU32)(dFrameRate * 10000 + .5);
    nFrameRateExtD = 10000;
    return MFX_ERR_NONE;    
}

HRESULT CQuickSync::Flush(bool deliverFrames)
{
    MSDK_TRACE("QsDecoder: Flush\n");

    CQsAutoLock cObjectLock(&m_csLock);

    HRESULT hr = S_OK;
    mfxStatus sts = MFX_ERR_NONE;

    // Recieved a BeginFlush that wasn't handled for some reason.
    // This overrides the request to output the remaining frames
    m_bNeedToFlush = m_bNeedToFlush || !deliverFrames;

    // Flush internal queue
    FlushOutputQueue();

    // Flush HW decoder by sending NULL bitstreams.
    while (MSDK_SUCCEEDED(sts) || MFX_ERR_MORE_SURFACE == sts)
    {
        mfxFrameSurface1* pSurf = NULL;
        sts = m_pDecoder->Decode(NULL, pSurf);

        if (MSDK_SUCCEEDED(sts) && !m_bNeedToFlush)
        {
            ProcessDecodedFrame(pSurf);
        }
    }

    // Flush internal queue again
    FlushOutputQueue();
    
    // If VPP is active, we may need to flush it
    FlushVPP();

    m_TimeManager.Reset();

    // All data has been flushed
    m_bNeedToFlush = false;

    MSDK_TRACE("QsDecoder: Flush ended\n");
    return hr;
}

void CQuickSync::FlushOutputQueue()
{
    MSDK_TRACE("QsDecoder: FlushOutputQueue (deliverFrames=%s)\n", (!m_bNeedToFlush) ? "TRUE" : "FALSE");

    // Note that m_bNeedToFlush can be changed by another thread at any time
    // Make sure worker thread has completed all it's tasks

    // Empty the decoded output queue
    for (size_t i = m_pDecoder->OutputQueueSize(); i > 0; --i)
    {
        ProcessDecodedFrame(NULL);
    }

    // Clear the decoded output queue - either failure in flushing or no need to deliver the frames.
    for (size_t i = m_pDecoder->OutputQueueSize(); i > 0; --i)
    {
        // Surface is not needed anymore
        m_pDecoder->UnlockSurface(PopSurface());
    }

    ASSERT(m_pDecoder->OutputQueueEmpty());
}

HRESULT CQuickSync::OnSeek(REFERENCE_TIME /* segmentStart */)
{
    MSDK_TRACE("QsDecoder: OnSeek\n");
    MSDK_CHECK_POINTER(m_pDecoder, E_UNEXPECTED);

    m_bNeedToFlush = true;
    CQsAutoLock cObjectLock(&m_csLock);

    mfxStatus sts = MFX_ERR_NONE;
    m_nSegmentFrameCount = 0;
    m_PicStruct = MFX_PICSTRUCT_PROGRESSIVE;

    // Make sure the worker thread is idle and released all resources
    FlushOutputQueue();

    m_TimeManager.Reset();
    if (m_pVPP)
    {
        // If VPP is active, we may need to flush it
        FlushVPP();

        MSDK_SAFE_DELETE(m_pVPP);
    }

    sts = m_pDecoder->Reset(&m_DecVideoParams, m_nPitch);
    if (MSDK_FAILED(sts))
    {
        MSDK_TRACE("QsDecoder: reset failed!\n");
        return E_FAIL;
    }

    m_pFrameConstructor->Reset();
    FlushOutputQueue();

    m_bNeedToFlush = false;
    MSDK_TRACE("QsDecoder: OnSeek complete\n");
    return (MSDK_SUCCEEDED(sts)) ? S_OK : E_FAIL;
}

bool CQuickSync::SetTimeStamp(mfxFrameSurface1* pSurface, REFERENCE_TIME& rtStart)
{
    if (!m_TimeManager.Enabled()) {
        rtStart = pSurface->Data.TimeStamp; 
        return true; // Return all frames DS filter will handle this
    }

    TFrameVector frames;
    frames.push_back(pSurface);
    auto queue = m_pDecoder->GetOutputQueue();
    frames.insert(frames.end(), queue.begin(), queue.end());

    // Always send frame to time manager so it can track inverse telecine
    bool rc = m_TimeManager.GetSampleTimeStamp(frames, rtStart);

    // Return corrected time stamp
        return rc && rtStart >= 0;
}

mfxStatus CQuickSync::OnVideoParamsChanged()
{    
    mfxVideoParam params;
    MSDK_ZERO_VAR(params);
    mfxStatus sts = m_pDecoder->GetVideoParams(&params);
    MSDK_CHECK_RESULT_P_RET(sts, MFX_ERR_NONE);

    mfxFrameInfo& curInfo = m_DecVideoParams.mfx.FrameInfo;
    mfxFrameInfo& newInfo = params.mfx.FrameInfo;

    bool bFrameRateChange = (curInfo.FrameRateExtN != newInfo.FrameRateExtN) || (curInfo.FrameRateExtD != newInfo.FrameRateExtD);

    if (!bFrameRateChange)
        return MFX_ERR_NONE;

    // Copy video params
    params.AsyncDepth = m_DecVideoParams.AsyncDepth;
    params.IOPattern = m_DecVideoParams.IOPattern;
    memcpy(&m_DecVideoParams, &params, sizeof(params));

    // Flush images with old parameters
    FlushOutputQueue();

    double frameRate = (double)curInfo.FrameRateExtN / (double)curInfo.FrameRateExtD;
    m_TimeManager.OnVideoParamsChanged(frameRate);
    return MFX_ERR_NONE;
}

void CQuickSync::PushSurface(mfxFrameSurface1* pSurface)
{
    m_pDecoder->PushSurface(pSurface);

    // Note - no need to lock as all API functions are already exclusive.
    m_TimeManager.AddOutputTimeStamp(pSurface);
}

mfxFrameSurface1* CQuickSync::PopSurface()
{
    return m_pDecoder->PopSurface();
}

HRESULT CQuickSync::CheckCodecProfileSupport(DWORD codec, DWORD profile)
{
    // H264
    if (codec == MFX_CODEC_AVC)
    {
        switch (profile)
        {
        case QS_PROFILE_H264_BASELINE:
        case QS_PROFILE_H264_CONSTRAINED_BASELINE:
        case QS_PROFILE_H264_MAIN:
        case QS_PROFILE_H264_HIGH:
            return S_OK;
        default:
            return E_NOTIMPL;
        }
    }

    // MPEG2
    if (codec == MFX_CODEC_MPEG2)
    {
        switch (profile)
        {
        case QS_PROFILE_MPEG2_SIMPLE:
        case QS_PROFILE_MPEG2_MAIN:
        case QS_PROFILE_MPEG2_HIGH:
        case QS_PROFILE_MPEG2_SPATIALLY_SCALABLE:
            return S_OK;
        default:
            return E_NOTIMPL;
        }
    }

    // VC1/WMV
    if (codec == MFX_CODEC_VC1)
    {
        switch (profile)
        {
        case QS_PROFILE_VC1_SIMPLE:
        case QS_PROFILE_VC1_MAIN:
        case QS_PROFILE_VC1_ADVANCED:
            return S_OK;
        default:
            return E_NOTIMPL;
        }
    }

    return E_NOTIMPL;
}

void CQuickSync::SetD3DDeviceManager(IDirect3DDeviceManager9* pDeviceManager)
{
    m_pDecoder->SetD3DDeviceManager(pDeviceManager);
}

void CQuickSync::GetConfig(CQsConfig* pConfig)
{
    if (NULL == pConfig)
        return;

    *pConfig = m_Config;
}

void CQuickSync::SetConfig(CQsConfig* pConfig)
{
    if (NULL == pConfig)
    {
        MSDK_TRACE("QsDecoder: SetConfig was called with a NULL parameter\n");
        return;
    }

    CQsAutoLock cObjectLock(&m_csLock);

    // 2nd initialization - empty queues and kill VPP
    if (m_bInitialized)
    {
        OnSeek(0);
        m_bInitialized = false;
        MSDK_ZERO_VAR(m_DecVideoParams.mfx.FrameInfo);
    }

#if (MFX_D3D11_SUPPORT == 0)
    m_Config.bEnableD3D11 = false;
#endif

    m_Config = *pConfig;
}

// This function works on a worker thread
HRESULT CQuickSync::ProcessDecodedFrame(mfxFrameSurface1* pOutSurface)
{
    MSDK_VTRACE("QsDecoder: ProcessDecodedFrame\n");

    // Got a new surface. A NULL surface means to get a surface from the output queue
    if (pOutSurface != NULL)
    {
        // Initial frames with invalid times are discarded
        REFERENCE_TIME rtStart = m_TimeManager.ConvertMFXTime2ReferenceTime(pOutSurface->Data.TimeStamp);
        if (m_TimeManager.Enabled() && m_pDecoder->OutputQueueEmpty() && !m_TimeManager.IsValidTimeStamp(rtStart))
        {
            m_pDecoder->UnlockSurface(pOutSurface);
            return S_OK;
        }

        PushSurface(pOutSurface);

        // Not enough surfaces for proper time stamp correction
        size_t queueSize = (m_bDvdDecoding) ? 0 : m_Config.nOutputQueueLength;
        if (m_pDecoder->OutputQueueSize() <= queueSize)
        {
            return S_OK;
        }
    }

    // Get oldest surface from queue
    pOutSurface = PopSurface();

    // Decoder queue is empty - return without error
    MSDK_CHECK_POINTER(pOutSurface, S_OK);

    // Forced field order
    if (m_Config.bForceFieldOrder && m_Config.eFieldOrder != QS_FIELD_AUTO)
    {
        if (m_Config.eFieldOrder == QS_FIELD_TFF)
            pOutSurface->Info.PicStruct = MFX_PICSTRUCT_FIELD_TFF;
        else if (m_Config.eFieldOrder == QS_FIELD_BFF)
            pOutSurface->Info.PicStruct = MFX_PICSTRUCT_FIELD_BFF;
    }

    // Result is in outFrameData
    // False return value means that the frame has a negative time stamp and should not be displayed.
    REFERENCE_TIME rtStart, rtPrevStart = m_TimeManager.GetLastTimeStamp();

    bool bDiscardFrame = !SetTimeStamp(pOutSurface, rtStart);
    bool bInIVTC = m_TimeManager.GetInverseTelecine(); // When true, current sequence has 3:2 flags
    bool bNeedToResetVpp = false;
    bool bVppNeeded = false;

    // Init, reset or destroy VPP
    if (m_Config.bEnableVideoProcessing)
    {
        bVppNeeded = IsVppNeeded(pOutSurface->Info.PicStruct);
        if (m_pVPP)
        {
            // May cause implicit reset if bInIVTC changes
            // Does nothing when forced DI is on.
            m_pVPP->EnableDI(!bInIVTC);
        }

        bNeedToResetVpp = (m_pVPP && m_pVPP->NeedReset());

        // Create VPP
        if (!m_pVPP && !m_bNeedToFlush && bVppNeeded)
        {
            if (!m_pVPP)
            {
                m_pVPP = new CQuickSyncVPP(m_pDecoder->IsD3DAlloc(), m_pDecoder->GetFrameAllocator());
                m_pVPP->EnableDI(!bInIVTC);
            }
    
            bNeedToResetVpp = true;
        }
    }
    
    // Store original surface
    mfxFrameSurface1* pInSurface = pOutSurface;
    mfxStatus sts = MFX_ERR_NONE;

    // Set corrected time stamp
    pInSurface->Data.TimeStamp = m_TimeManager.ConvertReferenceTime2MFXTime(rtStart);

    int frameDuplicates = 1;
    if (pInSurface->Info.PicStruct & MFX_PICSTRUCT_FRAME_DOUBLING)
        ++frameDuplicates;
    else if (pInSurface->Info.PicStruct & MFX_PICSTRUCT_FRAME_TRIPLING)
        frameDuplicates += 2;

    while (!m_bNeedToFlush)
    {
        // Apply VPP on 
        if (m_pVPP)
        {
            if (bNeedToResetVpp)
            {
                // Flush old frame
                pOutSurface = m_pVPP->FlushFrame();
                
                // Flush returned a frame
                if (pOutSurface != NULL)
                {
                    // More surfaces will be ready during this iteration
                    sts = MFX_ERR_MORE_SURFACE;
                }
                // No more internal frames - ready to reset
                else
                {
                    bNeedToResetVpp = false;
                    sts = m_pVPP->Reset(m_Config, m_pDecoder->GetSession(), pInSurface);

                    // Reset failed...
                    if (sts < 0)
                    {
                        MSDK_TRACE("QsDecoder: Error VPP reset failed!\n")
                        // Change config to disable VPP from being created for this video 
                        if (sts != MFX_ERR_NOT_INITIALIZED)
                        {
                            m_Config.bEnableVideoProcessing = false;
                        }

                        MSDK_SAFE_DELETE(m_pVPP);
                        sts = MFX_ERR_NONE;
                        pOutSurface = pInSurface;
                    }

                    // Do the loop again
                    continue;
                }
            }
            // Apply VPP on decoded image
            else
            {
                // Perform soft inverse telecine
                if (m_TimeManager.GetInverseTelecine())
                {
                    pInSurface->Info.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
                }

                // Run VPP
                sts = m_pVPP->Process(pInSurface, pOutSurface);

                // VPP failed
                if (MSDK_FAILED(sts) && sts != MFX_ERR_MORE_SURFACE)
                {
                    MSDK_TRACE("QsDecoder: VPP->Process failed with error %i\n", (int)sts);
                    pOutSurface = pInSurface;
                    sts = MFX_ERR_NONE;
                }
            }
        }

        // Check time stamp, interpolate newly created frames
        if (MFX_ERR_MORE_SURFACE == sts && m_Config.bVppEnableDITimeStampsInterpolation)
        {
            if (m_TimeManager.IsValidTimeStamp(pOutSurface->Data.TimeStamp) && m_Config.bVppEnableFullRateDI && pOutSurface->Info.FrameRateExtN > 0)
            {
                REFERENCE_TIME rtNewStart = (MFX_ERR_MORE_SURFACE == sts) ? rtPrevStart : m_TimeManager.GetLastTimeStamp();                    
                rtNewStart += (REFERENCE_TIME)(0.5 + 1e7 * (double)pOutSurface->Info.FrameRateExtD / (double)pOutSurface->Info.FrameRateExtN);
                pOutSurface->Data.TimeStamp = m_TimeManager.ConvertReferenceTime2MFXTime(rtNewStart);
            }
        }

        // Note: this flag is not very reliable - don't take any action because of it
        if (pOutSurface->Data.Corrupted)
        {
            MSDK_VTRACE("QsDecoder: warning received a corrupted frame\n");
        }

        // Deliver the frame!
        if (!bDiscardFrame)
        {
            DeliverSurface(pOutSurface, frameDuplicates);
        }

        // Unlock VPP surface
        if (m_pVPP && pOutSurface != pInSurface /* They are equal in case of failure */)
        {
            m_pVPP->UnlockSurface(pOutSurface);
        }

        // VPP might have another surface ready (DI active at double/full rate)
        if (MFX_ERR_MORE_SURFACE != sts)
            break;
    }

    ++m_nSegmentFrameCount; // Count only input frames

    // Release decoder surface
    m_pDecoder->UnlockSurface(pInSurface);

    MSDK_VTRACE("QsDecoder: ProcessDecodedFrame completed\n");
    return S_OK;
}

void CQuickSync::CopyFramePointers(mfxFrameSurface1* pSurface, QsFrameData& outFrameData, mfxFrameData& frameData)
{
    size_t pitch  = frameData.Pitch;      // Image line + padding in bytes --> set by the driver
   
    // Mark Y, U & V pointers on D3D buffer
    outFrameData.y = frameData.Y + (pSurface->Info.CropY * pitch);
    outFrameData.u = frameData.CbCr + (pSurface->Info.CropY * pitch);
    outFrameData.v = 0;
    outFrameData.a = 0;

    // App can't modify this buffer!
    outFrameData.bReadOnly = true;
}

void CQuickSync::CopyFrame(mfxFrameSurface1* pSurface, QsFrameData& outFrameData, CQsAlignedBuffer*& pOutBuffer, mfxFrameData& frameData)
{
    // Setup output buffer
    size_t outSize = 4096 + // Adding 4K for page alignment optimizations
        outFrameData.dwStride * pSurface->Info.CropH * 3 / 2;

    // Make sure we have a buffer with the right size
    if (pOutBuffer->GetBufferSize() < outSize)
    {
        delete pOutBuffer;
        pOutBuffer = new CQsAlignedBuffer(outSize);
    }

    size_t height = pSurface->Info.CropH; // Cropped image height
    size_t pitch  = frameData.Pitch;      // Image line + padding in bytes --> set by the driver

    if (m_pDecoder->IsD3D11Alloc() || !m_pDecoder->IsD3DAlloc())
    {
        outFrameData.y = frameData.Y + (pSurface->Info.CropY * pitch), height * pitch;
        outFrameData.u = frameData.CbCr + (pSurface->Info.CropY * pitch), pitch * height / 2;
        outFrameData.v = 0;
        outFrameData.a = 0;

        // App can modify this buffer
        outFrameData.bReadOnly = false;
    }
    else
    {
        // Offset output buffer's address for fastest SSE4.1 copy.
        // Page offset (12 lsb of addresses) sould be 2K apart from source buffer
        size_t offset = ((size_t)frameData.Y & PAGE_MASK) ^ (1 << 11);

        // Mark Y, U & V pointers on output buffer
        outFrameData.y = pOutBuffer->GetBuffer() + offset;
        outFrameData.u = outFrameData.y + (pitch * height);
        outFrameData.v = 0;
        outFrameData.a = 0;

        // App can modify this buffer
        outFrameData.bReadOnly = false;
#if 1 // Use this to disable actual copying for benchmarking
        Tmemcpy memcpyFunc = (m_pDecoder->IsD3DAlloc()) ?
            ( (m_Config.bEnableMtCopy) ? mt_gpu_memcpy : gpu_memcpy_sse41 ) :
            ( (m_Config.bEnableMtCopy) ? mt_memcpy     : memcpy );

        // Copy Y
        !m_bNeedToFlush && memcpyFunc(outFrameData.y, frameData.Y + (pSurface->Info.CropY * pitch), height * pitch);

        // Copy UV
        !m_bNeedToFlush && memcpyFunc(outFrameData.u, frameData.CbCr + (pSurface->Info.CropY * pitch), pitch * height / 2);
#endif
    }

#ifdef _DEBUG
    // Debug only - mark top left corner: when working with D3D
    ULONGLONG markY = 0;
    ULONGLONG markUV = (m_pDecoder->IsHwAccelerated()) ? ((m_pDecoder->IsD3D11Alloc()) ? 0xFFFFFFFFFFFFFFFF : 0x80FF80FF80FF80FF) : 0xFF80FF80FF80FF80;
    *((ULONGLONG*)outFrameData.y) = markY;
    *((ULONGLONG*)(outFrameData.y + outFrameData.dwStride)) = markY;
    *((ULONGLONG*)(outFrameData.y + 2 * outFrameData.dwStride)) = markY;
    *((ULONGLONG*)(outFrameData.y + 3 * outFrameData.dwStride)) = markY;
    *((ULONGLONG*)outFrameData.u) = markUV; // 4 blue (hw - D3D9) or red (sw) or pink (D3D11)
    *((ULONGLONG*)(outFrameData.u + outFrameData.dwStride)) = markUV;
#endif
}

void CQuickSync::UpdateAspectRatio(mfxFrameSurface1* pSurface, QsFrameData& frameData)
{
    mfxFrameInfo& info = pSurface->Info;
    PARtoDAR(info.AspectRatioW, info.AspectRatioH, info.CropW, info.CropH,
        frameData.dwPictAspectRatioX, frameData.dwPictAspectRatioY);
}

void CQuickSync::FlushVPP()
{
    MSDK_CHECK_POINTER_NO_RET(m_pVPP);
    REFERENCE_TIME rtPrevStart = m_TimeManager.GetLastTimeStamp();
    int count = 0;
    while (!m_bNeedToFlush)
    {
        mfxFrameSurface1* pOutSurface = m_pVPP->FlushFrame();

        // VPP failed or ran out of frames
        if (NULL == pOutSurface)
        {
            break;
        }

        // Check time stamp, interpolate newly created frames
        if (m_Config.bVppEnableDITimeStampsInterpolation)
        {
            if (m_TimeManager.IsValidTimeStamp(pOutSurface->Data.TimeStamp) && m_Config.bVppEnableFullRateDI && pOutSurface->Info.FrameRateExtN > 0)
            {
                REFERENCE_TIME rtNewStart = rtPrevStart;
                rtNewStart += (REFERENCE_TIME)(0.5 + (double)(++count) * 1e7 * (double)pOutSurface->Info.FrameRateExtD / (double)pOutSurface->Info.FrameRateExtN);
                pOutSurface->Data.TimeStamp = m_TimeManager.ConvertReferenceTime2MFXTime(rtNewStart);
            }
        }

        // If input frame was meant to be discarded then we still want to VPP to work -
        // minimize VPP initialization artifacts
        DeliverSurface(pOutSurface);
        m_pVPP->UnlockSurface(pOutSurface);
    }
}

bool CQuickSync::IsVppNeeded(mfxU32 picStruct)
{
    if (!m_Config.bEnableVideoProcessing)
        return false;

    // Detail and Denoise are on
    if (m_Config.nVppDetailStrength || m_Config.nVppDenoiseStrength)
        return true;

    // DI is off
    if (!m_Config.bVppEnableDeinterlacing)
        return false;

    // DI is forced to work
    if (m_Config.bVppEnableForcedDeinterlacing)
        return true;

    // DI is disabled during soft inverse telecine
    if (m_TimeManager.GetInverseTelecine())
        return false;

    // Frame doubling / tripling of progressive frames
    if ((picStruct == MFX_PICSTRUCT_FRAME_DOUBLING) ||
        (picStruct == MFX_PICSTRUCT_FRAME_TRIPLING) ||
        (picStruct == (MFX_PICSTRUCT_FRAME_DOUBLING | MFX_PICSTRUCT_PROGRESSIVE)) ||
        (picStruct == (MFX_PICSTRUCT_FRAME_TRIPLING | MFX_PICSTRUCT_PROGRESSIVE)))
        return false;

    // Note - m_PicStruct is set to progressive in OnSeek()

    // DI might be needed as frame type changed - m_PicStruct remembers the last non progressive type
    if (m_PicStruct != picStruct && picStruct != MFX_PICSTRUCT_PROGRESSIVE)
    {
        m_PicStruct = picStruct;
    }

    return m_PicStruct != MFX_PICSTRUCT_PROGRESSIVE;
}
