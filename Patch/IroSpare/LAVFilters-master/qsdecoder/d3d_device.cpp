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
#include "QuickSync_defs.h"
#include "QuickSyncUtils.h"
#include "d3d_device.h"

CD3D9Device::CD3D9Device(IDirect3DDeviceManager9* pDeviceManager9) :
    m_pDeviceManager9(pDeviceManager9)
{
    m_pD3D9 = NULL;
    m_pD3DD9 = NULL;
    MSDK_ZERO_VAR(m_D3DPP);
    m_ResetToken = 0;
}

mfxStatus CD3D9Device::FillD3DPP(mfxHDL hWindow, D3DPRESENT_PARAMETERS &D3DPP)
{
    mfxStatus sts = MFX_ERR_NONE;

    D3DPP.Windowed = true;
    D3DPP.hDeviceWindow = (HWND)hWindow;

    D3DPP.Flags                      = D3DPRESENTFLAG_VIDEO;
    D3DPP.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    D3DPP.PresentationInterval       = D3DPRESENT_INTERVAL_ONE; // note that this setting leads to an implicit timeBeginPeriod call
    D3DPP.BackBufferCount            = 1;
    D3DPP.BackBufferFormat           = D3DFMT_X8R8G8B8;

    D3DPP.BackBufferWidth  = 1;
    D3DPP.BackBufferHeight = 1;

    //
    // Mark the back buffer lockable if software DXVA2 could be used.
    // This is because software DXVA2 device requires a lockable render target
    // for the optimal performance.
    //
    D3DPP.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    D3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;

    return sts;
}

mfxStatus CD3D9Device::Init(int nAdapterNum)
{
    mfxStatus sts = InitFromScratch(nAdapterNum);
    if (sts != MFX_ERR_NONE && m_pDeviceManager9)
    {
        Close();
        sts = InitFromRenderer(nAdapterNum);
    }
    return sts;
}

mfxStatus CD3D9Device::Reset()
{
    HRESULT hr = NO_ERROR;

    D3DPRESENT_PARAMETERS d3dpp=m_D3DPP;
    ZeroMemory(&d3dpp, sizeof(d3dpp));

    D3DDISPLAYMODE d3dmodeTemp;
    m_pD3DD9->GetDisplayMode(0, &d3dmodeTemp);

    d3dpp.BackBufferWidth  = d3dmodeTemp.Width;
    d3dpp.BackBufferHeight = d3dmodeTemp.Height;

    // Reset will change the parameters, so use a copy instead.
    D3DPRESENT_PARAMETERS d3dppTmp=d3dpp;
    hr = m_pD3DD9->Reset(&d3dppTmp);
    if (FAILED(hr))
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    m_D3DPP = d3dpp;

    hr = m_pDeviceManager9->ResetDevice(m_pD3DD9, m_ResetToken);
    if (FAILED(hr))
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    return MFX_ERR_NONE;
}

void CD3D9Device::Close()
{
    MSDK_SAFE_RELEASE(m_pDeviceManager9);
    MSDK_SAFE_RELEASE(m_pD3DD9);
    MSDK_SAFE_RELEASE(m_pD3D9);
}

CD3D9Device::~CD3D9Device()
{
    Close();
}

mfxHDL CD3D9Device::GetHandle(mfxHandleType type)
{
    if (MFX_HANDLE_DIRECT3D_DEVICE_MANAGER9 == type)
    {
        return (mfxHDL)m_pDeviceManager9;
    }

    return NULL;
}

mfxStatus CD3D9Device::InitFromScratch(int nAdapterNum)
{
    mfxStatus sts = MFX_ERR_NONE;

    HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D9);
    if (!m_pD3D9 || FAILED(hr))
        return MFX_ERR_DEVICE_FAILED;

    ZeroMemory(&m_D3DPP, sizeof(m_D3DPP));
    sts = FillD3DPP(NULL, m_D3DPP);
    MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, sts);     

    hr = m_pD3D9->CreateDevice(
        nAdapterNum,
        D3DDEVTYPE_HAL,
        NULL,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
        &m_D3DPP,
        &m_pD3DD9);

    if (FAILED(hr))
        return MFX_ERR_NULL_PTR;

    UINT resetToken = 0;

    hr = DXVA2CreateDirect3DDeviceManager9(&resetToken, &m_pDeviceManager9);
    if (FAILED(hr)) 
        return MFX_ERR_NULL_PTR;

    hr = m_pDeviceManager9->ResetDevice(m_pD3DD9, resetToken);
    if (FAILED(hr))
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    m_ResetToken = resetToken;
    return sts;
}

mfxStatus CD3D9Device::InitFromRenderer(int nAdapterNum)
{
    mfxStatus sts = MFX_ERR_NONE;
    IDirect3DDeviceManager9* extD3DManager = m_pDeviceManager9;
    m_pDeviceManager9 = NULL;

    // Get DirectX Object
    HANDLE hDevice;
    IDirect3DDevice9* pDevice = NULL;
    CComPtr<IDirect3D9> pD3D;
    D3DDEVICE_CREATION_PARAMETERS devParames;
    ZeroMemory(&m_D3DPP, sizeof(m_D3DPP));
    sts = FillD3DPP(NULL, m_D3DPP);
    MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, sts);     

    HRESULT hr = extD3DManager->OpenDeviceHandle(&hDevice);
    if (FAILED(hr))
    {
        MSDK_TRACE("QsDecoder: failed to open device handle!\n");
        goto done;
    }

    hr = extD3DManager->LockDevice(hDevice, &pDevice, TRUE);
    if (FAILED(hr) || NULL == pDevice)
    {
        MSDK_TRACE("QsDecoder: failed to lock device!\n");
        switch (hr)
        {
        case DXVA2_E_NEW_VIDEO_DEVICE:
            MSDK_TRACE("QsDecoder: The device handle is invalid.!\n");
            break;
        case DXVA2_E_NOT_INITIALIZED:
            MSDK_TRACE("QsDecoder: The Direct3D device manager was not initialized.!\n");
            break;
        case E_HANDLE:
            MSDK_TRACE("QsDecoder: The specified handle is not a Direct3D device handle.!\n");
            break;
        default:
            MSDK_TRACE("QsDecoder: Unknown error while locking D3D device %x\n", hr);
        }
        goto done;
    }

    hr = pDevice->GetDirect3D(&pD3D);
    if (FAILED(hr))
    {
        MSDK_TRACE("QsDecoder: failed to get D3D9 object!\n");
        goto done;
    }

    hr = pDevice->GetCreationParameters(&devParames);
    if (FAILED(hr))
    {
        MSDK_TRACE("QsDecoder: failed to get device creation params!\n");
        goto done;
    }

    // Create d3d device
    m_pD3DD9 = 0;
    m_D3DPP.hDeviceWindow = devParames.hFocusWindow;
    hr = pD3D->CreateDevice(
        nAdapterNum,
        D3DDEVTYPE_HAL,
        devParames.hFocusWindow,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
        &m_D3DPP,
        &m_pD3DD9);

    if (FAILED(hr) || !m_pD3DD9)
    {
        hr = E_FAIL;
        MSDK_TRACE("QsDecoder: InitD3d CreateDevice failed!\n");
        goto done;
    }

    // Create device manager            
    hr = DXVA2CreateDirect3DDeviceManager9(&m_ResetToken, &m_pDeviceManager9);
    if (FAILED(hr) || !m_pDeviceManager9)
    {
        hr = E_FAIL;
        MSDK_TRACE("QsDecoder: InitD3d DXVA2CreateDirect3DDeviceManager9 failed!\n");
        goto done;
    }

    // Reset the d3d device
    hr = m_pDeviceManager9->ResetDevice(m_pD3DD9, m_ResetToken);
    if (FAILED(hr))
    {
        hr = E_FAIL;
        MSDK_TRACE("QsDecoder: InitD3d ResetDevice failed!\n");
        goto done;
    }

    // Cleanup
done:
    if (FAILED(hr))
    {
        sts = MFX_ERR_DEVICE_FAILED;
    }

    MSDK_SAFE_RELEASE(pDevice);
    if (hDevice != NULL)
    {
        extD3DManager->UnlockDevice(hDevice, FALSE);
        extD3DManager->CloseDeviceHandle(hDevice);
    }

    return sts;
}

