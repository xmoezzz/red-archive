/*
 * Copyright (c) 2011, INTEL CORPORATION
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
#include "QuickSync_defs.h"
#include "CodecInfo.h"
#include "QuickSyncUtils.h"

static bool s_SSE4_1_enabled = IsSSE41Enabled();
static bool s_AVX2_enabled = IsAVX2Enabled();

static const
struct
{
    // actual implementation
    mfxIMPL impl;
    // adapter's number
    int adapterID;

} implTypes[] =
{
    {MFX_IMPL_HARDWARE, 0},
    {MFX_IMPL_SOFTWARE, -1},
    {MFX_IMPL_HARDWARE2, 1},
    {MFX_IMPL_HARDWARE3, 2},
    {MFX_IMPL_HARDWARE4, 3}
};

// gpu_memcpy_sse41 is a memcpy style function that copied data very fast from a
// GPU tiled memory (uncached speculative write back memory)
// Performance tip: page offset (12 lsb) of both addresses should be different
//  optimally use a 2K offset between them.
void* gpu_memcpy_sse41(void* d, const void* s, size_t size)
{
    static const size_t regsInLoop = 2;

    if (d == NULL || s == NULL) return NULL;

    // If memory is not aligned, use memcpy
    bool isAligned = (((size_t)(s) | (size_t)(d)) & 0xF) == 0;
    if (!(isAligned && s_SSE4_1_enabled))
    {
        return memcpy(d, s, size);
    }

    size_t reminder = size & (regsInLoop * sizeof(__m128i) - 1); // Copy 32 bytes every loop
    size_t end = 0;

    __m128i xmm0, xmm1; // Will actually use xmm registers
    __m128i* pTrg = (__m128i*)d;
    __m128i* pTrgEnd = pTrg + ((size - reminder) >> 4);
    __m128i* pSrc = (__m128i*)s;
    
    // Make sure source is synced - doesn't hurt if not needed.
    _mm_sfence();

    while (pTrg < pTrgEnd)
    {
        // _mm_stream_load_si128 emits the Streaming SIMD Extensions 4 (SSE4.1) instruction MOVNTDQA
        // Fastest method for copying GPU RAM. Available since Penryn (45nm Core 2 Duo/Quad)
        xmm0  = _mm_stream_load_si128(pSrc);
        xmm1  = _mm_stream_load_si128(pSrc + 1);
        pSrc += regsInLoop;

        // _mm_store_si128 emit the SSE2 intruction MOVDQA (aligned store)
        // Note a streaming store instruction (bypass L3 cache) will probably be faster in synthetic
        // benchmarks but slower in real world usage as the buffer will be used by the application soon
        // and better keep the L3 cache hot.
        _mm_store_si128(pTrg     , xmm0);
        _mm_store_si128(pTrg +  1, xmm1);
        _mm_store_si128(pTrg +  1, xmm1); // Not a bug - works 4.5% faster!
        _mm_store_si128(pTrg +  1, xmm1);
        pTrg += regsInLoop;
    }

    // Copy in 16 byte steps
    if (reminder >= 16)
    {
        size = reminder;
        reminder = size & 15;
        end = size >> 4;
        for (size_t i = 0; i < end; ++i)
        {
            pTrg[i] = _mm_stream_load_si128(pSrc + i);
        }
    }

    // Copy last bytes - shouldn't happen as strides are modulu 16
    if (reminder)
    {
        __m128i temp = _mm_stream_load_si128(pSrc + end);

        char* ps = (char*)(&temp);
        char* pt = (char*)(pTrg + end);

        for (size_t i = 0; i < reminder; ++i)
        {
            pt[i] = ps[i];
        }
    }

    return d;
}

// AVX2 copy function. Available since Haswell (4th Generation Core architecture) on premium models.
#if !defined (AVX2_ENABLED)
void* gpu_memcpy_avx2(void* d, const void* s, size_t size)
{
    return gpu_memcpy_sse41(d, s, size);
}
#else //#if defined (AVX2_ENABLED)
void* gpu_memcpy_avx2(void* d, const void* s, size_t size)
{
    // Must be exp of 2
    static const size_t regsInLoop = 4; //TODO: need to tune this...

    if (d == NULL || s == NULL) return NULL;

    // If memory is not AVX aligned (32B), use gpu_memcpy_sse41 -->TODO: check actual performance hit
    bool isAligned = (((size_t)(s) | (size_t)(d)) & 0x1F) == 0;
    if (!(isAligned && s_SSE4_1_enabled))
    {
        return gpu_memcpy_sse41(d, s, size);
    }

    size_t reminder = size & (regsInLoop * sizeof(__m256i) - 1); // Copy 64 bytes every loop
    size_t end = 0;

    __m256i ymm0, ymm1, ymm2, ymm3; // Will actually use ymm registers
    __m256i* pTrg = (__m256i*)d;
    __m256i* pTrgEnd = pTrg + ((size - reminder) >> 5);
    __m256i* pSrc = (__m256i*)s;
    
    // Make sure source is synced - doesn't hurt if not needed.
    _mm_sfence();

    while (pTrg < pTrgEnd)
    {
        // _mm256_stream_load_si256 emits the AVX2 instruction VMOVNTDQA
        // Available since Haswell (22nm, 4th Generation Core)
        ymm0  = _mm256_stream_load_si256(pSrc);
        ymm1  = _mm256_stream_load_si256(pSrc + 1);
        ymm2  = _mm256_stream_load_si256(pSrc + 2);
        ymm3  = _mm256_stream_load_si256(pSrc + 3);
        pSrc += regsInLoop;

        // _mm256_store_si256 emit the AVX intruction VMOVDQA (aligned store)
        // Note a streaming store instruction (bypass L3 cache) will probably be faster in synthetic
        // benchmarks but slower in real world usage as the buffer will be used by the application soon
        // and better keep the L3 cache hot.
        _mm256_store_si256(pTrg     , ymm0);
        _mm256_store_si256(pTrg +  1, ymm1);
        _mm256_store_si256(pTrg +  2, ymm2);
        _mm256_store_si256(pTrg +  3, ymm3);
        pTrg += regsInLoop;
    }

    // Copy in 32 byte steps
    if (reminder >= 32)
    {
        size = reminder;
        reminder = size & 31;
        end = size >> 5;
        for (size_t i = 0; i < end; ++i)
        {
            pTrg[i] = _mm256_stream_load_si256(pSrc + i);
        }
    }

    // Copy last bytes
    if (reminder)
    {
        __m256i temp = _mm256_stream_load_si256(pSrc + end);

        char* ps = (char*)(&temp);
        char* pt = (char*)(pTrg + end);

        for (size_t i = 0; i < reminder; ++i)
        {
            pt[i] = ps[i];
        }
    }

    return d;
}
#endif // #if defined (AVX2_ENABLED)

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType;     // Must be 0x1000.
   LPCSTR szName;    // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags;    // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

/*
    Renames current thread. Useful for identifying a thread in VS.
*/
void SetThreadName(LPCSTR szThreadName, DWORD dwThreadID)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

    __try
    {
        #define MS_VC_EXCEPTION 0x406D1388
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(DWORD), (ULONG_PTR*)&info);
    }
// Disable static analysis warnings about this code
#pragma warning(push)
#pragma warning(disable: 6312 6322)
    __except(EXCEPTION_CONTINUE_EXECUTION)
    {
    }
#pragma warning(pop)
} 

#ifdef _DEBUG
void DebugAssert(const TCHAR* pCondition,const TCHAR* pFileName, int iLine)
{
    TCHAR szInfo[2048];
    _sntprintf_s(szInfo, sizeof(szInfo), _T("%s \nAt line %d of %s\nContinue? (Cancel to debug)"), pCondition, iLine, pFileName);

    int msgId = MessageBox(NULL, szInfo, _T("ASSERT Failed"),
        MB_SYSTEMMODAL | MB_ICONHAND | MB_YESNOCANCEL | MB_SETFOREGROUND);
    switch (msgId)
    {
    case IDNO:              /* Kill the application */
        FatalAppExit(FALSE, _T("Application terminated"));
        break;

    case IDCANCEL:          /* Break into the debugger */
        DebugBreak();
        break;

    case IDYES:             /* Ignore assertion continue execution */
        break;
    }
}

#endif //_DEBUG

// Finds greatest common denominator
mfxU32 GCD(mfxU32 a, mfxU32 b)
{    
    if (0 == a)
        return b;
    else if (0 == b)
        return a;

    mfxU32 a1, b1;

    if (a >= b)
    {
        a1 = a;
        b1 = b;
    }
    else 
    {
        a1 = b;
        b1 = a;
    }

    // a1 >= b1;
    mfxU32 r = a1 % b1;

    while (0 != r)
    {
        a1 = b1;
        b1 = r;
        r = a1 % b1;
    }

    return b1;
}

mfxStatus PARtoDAR(DWORD parw, DWORD parh, DWORD w, DWORD h, DWORD& darw, DWORD& darh)
{
    MSDK_CHECK_ERROR(parw, 0, MFX_ERR_UNDEFINED_BEHAVIOR);
    MSDK_CHECK_ERROR(parh, 0, MFX_ERR_UNDEFINED_BEHAVIOR);
    MSDK_CHECK_ERROR(w, 0, MFX_ERR_UNDEFINED_BEHAVIOR);
    MSDK_CHECK_ERROR(h, 0, MFX_ERR_UNDEFINED_BEHAVIOR);

    mfxU32 reduced_w = 0, reduced_h = 0;
    mfxU32 gcd = GCD(w, h);

    // Divide by greatest common divisor
    reduced_w =  w / gcd;
    reduced_h =  h / gcd;

    darw = reduced_w * parw;
    darh = reduced_h * parh;
    gcd = GCD(darw, darh);

    darh /= gcd;
    darw /= gcd;

    return MFX_ERR_NONE;
}

mfxStatus DARtoPAR(mfxU32 darw, mfxU32 darh, mfxU32 w, mfxU32 h, mfxU16& parw, mfxU16& parh)
{
    MSDK_CHECK_ERROR(darw, 0, MFX_ERR_UNDEFINED_BEHAVIOR);
    MSDK_CHECK_ERROR(darh, 0, MFX_ERR_UNDEFINED_BEHAVIOR);
    MSDK_CHECK_ERROR(w, 0, MFX_ERR_UNDEFINED_BEHAVIOR);
    MSDK_CHECK_ERROR(h, 0, MFX_ERR_UNDEFINED_BEHAVIOR);

    mfxU16 reduced_w = 0, reduced_h = 0;
    mfxU32 gcd = GCD(w, h);

    // Divide by greatest common divisor to fit into mfxU16
    reduced_w =  (mfxU16) (w / gcd);
    reduced_h =  (mfxU16) (h / gcd);

    // For mpeg2 we need to set exact values for par (standard supports only dar 4:3, 16:9, 221:100, 1:1)
    if (darw * 3 == darh * 4)
    {
        parw = 4 * reduced_h;
        parh = 3 * reduced_w;
    }
    else if (darw * 9 == darh * 16)
    {
        parw = 16 * reduced_h;
        parh = 9  * reduced_w;
    }
    else if (darw * 100 == darh * 221)
    {
        parw = 221 * reduced_h;
        parh = 100 * reduced_w;
    }
    else if (darw == darh)
    {
        parw = reduced_h;
        parh = reduced_w;
    }
    else
    {
        parw = (mfxU16)((DOUBLE)(darw * reduced_h) / (darh * reduced_w) * 1000);
        parh = 1000;
    }

    // Reduce ratio
    gcd = GCD(parw, parh);
    parw /= (mfxU16)gcd;
    parh /= (mfxU16)gcd;

    return MFX_ERR_NONE;
}

const char* GetCodecName(DWORD codec)
{
    switch (codec)
    {
    case MFX_CODEC_AVC:   return "AVC";
    case MFX_CODEC_MPEG2: return "MPEG2";
    case MFX_CODEC_VC1:   return "VC1";
    default:
        return "Unknown";
    }
}

const char* GetProfileName(DWORD codec, DWORD profile)
{
    switch (codec)
    {
    case MFX_CODEC_AVC:
        switch (profile)
        {
        case QS_PROFILE_H264_BASELINE:             return "Baseline";
        case QS_PROFILE_H264_CONSTRAINED_BASELINE: return "Constrained Baseline";
        case QS_PROFILE_H264_MAIN:                 return "Main";
        case QS_PROFILE_H264_EXTENDED:             return "Extended";
        case QS_PROFILE_H264_HIGH:                 return "High";
        case QS_PROFILE_H264_HIGH_10:              return "High 10";
        case QS_PROFILE_H264_HIGH_10_INTRA:        return "High 10 Intra";
        case QS_PROFILE_H264_HIGH_422:             return "High 4:2:2";
        case QS_PROFILE_H264_HIGH_422_INTRA:       return "High 4:2:2 Intra";
        case QS_PROFILE_H264_HIGH_444:             return "High 4:4:4";
        case QS_PROFILE_H264_HIGH_444_PREDICTIVE:  return "High 4:4:4 Preditive";
        case QS_PROFILE_H264_HIGH_444_INTRA:       return "High 4:4:4 Intra";
        case QS_PROFILE_H264_CAVLC_444:            return "CAVLC 4:4:4 Intra";
        case QS_PROFILE_H264_MULTIVIEW_HIGH:       return "Multi View High";
        case QS_PROFILE_H264_STEREO_HIGH:          return "Stereo High"; 
        }
        break;

    case MFX_CODEC_MPEG2:
        switch (profile)
        {
        case QS_PROFILE_MPEG2_422:                return "4:2:2";
        case QS_PROFILE_MPEG2_HIGH:               return "High";
        case QS_PROFILE_MPEG2_SPATIALLY_SCALABLE: return "Spatially Scalable";
        case QS_PROFILE_MPEG2_SNR_SCALABLE:       return "SNR Scalable";
        case QS_PROFILE_MPEG2_MAIN:               return "Main";
        case QS_PROFILE_MPEG2_SIMPLE:             return "Simple";
        }
        break;

    case MFX_CODEC_VC1:
        switch (profile)
        {
        case QS_PROFILE_VC1_SIMPLE:   return "Simple";
        case QS_PROFILE_VC1_MAIN:     return "Main";
        case QS_PROFILE_VC1_COMPLEX:  return "Complex";
        case QS_PROFILE_VC1_ADVANCED: return "Advanced";
        }
        break;

    default: break;
    }

    return "Unknown";
}

bool IsSSE41Enabled() // for MOVNTDQA instruction
{
   int CPUInfo[4];
    __cpuid(CPUInfo, 1);

    return 0 != (CPUInfo[2] & (1<<19)); // 19th bit of 2nd reg means sse4.1 is enabled
}

bool IsAVX2Enabled() // for VMOVNTDQA
{
   int CPUInfo[4];
    __cpuid(CPUInfo, 7);

    return 0 != (CPUInfo[1] & (1<<5)); // 5th bit of 2nd reg means AVX2 is enabled
}

#define MIN_BUFF_SIZE (1 << 18)

static void* mt_copy(void* d, const void* s, size_t size, Tmemcpy memcpyFunc)
{
    MSDK_CHECK_POINTER(d, NULL);
    MSDK_CHECK_POINTER(s, NULL);

    // Buffer is very small and not worth the effort
    if (size < MIN_BUFF_SIZE)
    {
        return memcpyFunc(d, s, size);
    }

    size_t blockSize = (size / 2) & ~31; // Make size a multiple of 32 bytes
    const size_t offsets[] = { 0, blockSize, size };

    Concurrency::parallel_for(0, 2, [d, s, &offsets, memcpyFunc](int i)
    {
        memcpyFunc((char*)d + offsets[i], (const char*)s + offsets[i], offsets[i + 1] - offsets[i]);
    });

    return d;
}

void* mt_memcpy(void* d, const void* s, size_t size)
{
    return mt_copy(d, s, size, memcpy);
}

void* mt_gpu_memcpy(void* d, const void* s, size_t size)
{
    return (s_AVX2_enabled) ?
        mt_copy(d, s, size, gpu_memcpy_avx2) :
        mt_copy(d, s, size, gpu_memcpy_sse41);
}

int GetIntelAdapterIdD3D9(IDirect3D9* _pd3d)
{
    CComPtr<IDirect3D9> pd3d = _pd3d;
    if (!pd3d)
    {
        pd3d = Direct3DCreate9(D3D_SDK_VERSION);
        if (!pd3d) return -1;
    }

    unsigned adapterCount = (int)pd3d->GetAdapterCount();
    D3DADAPTER_IDENTIFIER9 adIdentifier;
    for (int i = adapterCount-1; i >= 0; --i)
    {
        HRESULT hr = pd3d->GetAdapterIdentifier(i, 0, &adIdentifier);
        if (SUCCEEDED(hr))
        {
            // Look for Intel's vendor ID (8086h)
            if (adIdentifier.VendorId == 0x8086)
                return i;
        }
    }

    return -1;
}

// Get the ID of the adapter (GPU) associated with an MSDK session, -1 for SW session
int GetMSDKAdapterNumber(mfxSession session)
{
    int adapterNum = -1; // SW
    mfxIMPL impl = MFX_IMPL_SOFTWARE; // default in case no HW IMPL is found

    // we don't care for error codes in further code; if something goes wrong we fall back to the default adapter
    if (session)
    {
        MFXQueryIMPL(session, &impl);
    }
    else
    {
        // an auxiliary session, internal for this function
        mfxSession auxSession;
        memset(&auxSession, 0, sizeof(auxSession));

        mfxVersion ver = {1, 1}; // minimum API version which supports multiple devices
        MFXInit(MFX_IMPL_HARDWARE_ANY, &ver, &auxSession);
        MFXQueryIMPL(auxSession, &impl);
        MFXClose(auxSession);
    }

    // extract the base implementation type
    mfxIMPL baseImpl = MFX_IMPL_BASETYPE(impl);

    // get corresponding adapter number
    for (mfxU8 i = 0; i < sizeof(implTypes)/sizeof(implTypes[0]); ++i)
    {
        if (implTypes[i].impl == baseImpl)
        {
            adapterNum = implTypes[i].adapterID;
            break;
        }
    }

    return adapterNum;
}
