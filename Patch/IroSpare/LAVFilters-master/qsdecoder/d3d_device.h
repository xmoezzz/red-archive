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

/** Direct3D 9 device implementation.
@note Can be initilized for only 1 or two 2 views. Handle to 
MFX_HANDLE_GFXS3DCONTROL must be set prior if initializing for 2 views.

@note Device always set D3DPRESENT_PARAMETERS::Windowed to TRUE.
*/
class CD3D9Device : public CHWDevice
{
public:
    CD3D9Device(IDirect3DDeviceManager9* pDeviceManager9);
    virtual ~CD3D9Device();

    virtual mfxStatus Init(int nAdapterNum);
    virtual mfxStatus Reset();
    virtual mfxHDL    GetHandle(mfxHandleType type);
    virtual void      Close();
   
protected:
    virtual mfxStatus FillD3DPP(mfxHDL hWindow, D3DPRESENT_PARAMETERS &D3DPP);
    mfxStatus InitFromScratch(int nAdapterNum);
    mfxStatus InitFromRenderer(int nAdapterNum);

private:
    IDirect3D9Ex*               m_pD3D9;
    IDirect3DDevice9*           m_pD3DD9;
    IDirect3DDeviceManager9*    m_pDeviceManager9;
    D3DPRESENT_PARAMETERS       m_D3DPP;
    UINT                        m_ResetToken;
};
