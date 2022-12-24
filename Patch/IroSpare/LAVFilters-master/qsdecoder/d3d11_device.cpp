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
#if MFX_D3D11_SUPPORT
#include "QuickSync_defs.h"
#include "d3d11_device.h"

CD3D11Device::CD3D11Device()
{
}

CD3D11Device::~CD3D11Device()
{
    Close();
}

mfxStatus CD3D11Device::Init(int nAdapterNum)
{
    static const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1, 
        D3D_FEATURE_LEVEL_11_0, 
        D3D_FEATURE_LEVEL_10_1, 
        D3D_FEATURE_LEVEL_10_0 
    };
    D3D_FEATURE_LEVEL pFeatureLevelsOut;

    mfxStatus sts = MFX_ERR_NONE;
    HRESULT hres = S_OK;

    hres = CreateDXGIFactory(__uuidof(IDXGIFactory2), (void**)(&m_pDXGIFactory) );
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    hres = m_pDXGIFactory->EnumAdapters(nAdapterNum,&m_pAdapter);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    hres =  D3D11CreateDevice(m_pAdapter,
        D3D_DRIVER_TYPE_UNKNOWN,
        NULL,
        0,
        featureLevels,
        MSDK_ARRAY_LEN(featureLevels),
        D3D11_SDK_VERSION,
        &m_pD3D11Device,
        &pFeatureLevelsOut,
        &m_pD3D11Ctx);

    if (FAILED(hres))    
        return MFX_ERR_DEVICE_FAILED;

    m_pDX11VideoDevice = m_pD3D11Device;
    m_pVideoContext = m_pD3D11Ctx;
    
    MSDK_CHECK_POINTER(m_pDX11VideoDevice.p, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(m_pVideoContext.p, MFX_ERR_NULL_PTR);

    // turn on multithreading for the Context 
    CComQIPtr<ID3D10Multithread> p_mt(m_pVideoContext);

    if (p_mt)
        p_mt->SetMultithreadProtected(true);
    else
        return MFX_ERR_DEVICE_FAILED; 

    return sts;
}

mfxStatus CD3D11Device::Reset()
{
    return MFX_ERR_NONE;
}

mfxHDL CD3D11Device::GetHandle(mfxHandleType type)
{
    if (MFX_HANDLE_D3D11_DEVICE == type)
    {
        return (mfxHDL)m_pD3D11Device.p;
    }

    return NULL;
}

void CD3D11Device::Close()
{
    Reset();
}

#endif // #if MFX_D3D11_SUPPORT
