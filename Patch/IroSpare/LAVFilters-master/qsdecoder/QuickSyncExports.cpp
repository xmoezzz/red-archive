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
#include "TimeManager.h"
#include "QuickSyncDecoder.h"
#include "QuickSyncVPP.h"
#include "QuickSync.h"

#ifdef __INTEL_COMPILER
#   if __INTEL_COMPILER >= 1300
#       define COMPILER "ICL 13"
#   elif __INTEL_COMPILER >= 1200
#       define COMPILER "ICL 12"
#   elif __INTEL_COMPILER >= 1100
#       define COMPILER "ICL 11"
#   elif __INTEL_COMPILER >= 1000
#       define COMPILER "ICL 10"
#   else
#       define COMPILER "ICL"
#   endif
#elif defined(_MSC_VER)
#   if _MSC_VER==1700
#       define COMPILER "MSVC 2012"
#   elif _MSC_VER==1600
#       define COMPILER "MSVC 2010"
#   elif _MSC_VER==1500
#       define COMPILER "MSVC 2008"
#   else    
#       define COMPILER "MSVC"
#   endif
#else
#   define COMPILER "Unknown and not supported compiler"
#endif

#ifndef _M_X64
#   define ARCH "x86"
#else
#   define ARCH "x64"
#endif

IQuickSyncDecoder* __stdcall createQuickSync()
{
    return new CQuickSync();
}

void __stdcall destroyQuickSync(IQuickSyncDecoder* p)
{
    delete (CQuickSync*)(p);
}

void __stdcall getVersion(char* ver, const char** license)
{
    static const char s_Version[] = QS_DEC_VERSION " by Eric Gur. " COMPILER ", " ARCH " (" __DATE__ " " __TIME__ ")";
    strcpy_s(ver, 100, s_Version);
    *license = "(C) 2013 Intel\xae Corp.";
}

DWORD __stdcall check()
{
    mfxVersion apiVersion = {1, 1};
    MFXVideoSession* pSession = new MFXVideoSession;
    mfxIMPL impl = MFX_IMPL_AUTO_ANY | MFX_IMPL_VIA_ANY;
    mfxStatus sts = pSession->Init(impl, &apiVersion);
    MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, QS_CAP_UNSUPPORTED);
    pSession->QueryIMPL(&impl);

    DWORD caps;
    if (impl == MFX_IMPL_SOFTWARE)
    {
        caps = QS_CAP_SW_EMULATION;
    }
    else
    {
        caps = QS_CAP_HW_ACCELERATION;

        // Test SW emulation
        pSession->Close();
        sts = pSession->Init(MFX_IMPL_SOFTWARE, &apiVersion);
        if (MSDK_SUCCEEDED(sts))
        {
            caps |= QS_CAP_SW_EMULATION;
        }
    }

    caps |= QS_CAP_DEINTERLACING;
    caps |= QS_CAP_DETAIL;
    caps |= QS_CAP_DENOISE;

    delete pSession;
    return caps;
}
