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

#include "base_allocator.h"

enum eTypeHandle
{
    DXVA2_PROCESSOR     = 0x00,
    DXVA2_DECODER       = 0x01
};

struct D3DAllocatorParams : mfxAllocatorParams
{
    IDirect3DDeviceManager9 *pManager;
    DWORD surfaceUsage;

    D3DAllocatorParams()
        : pManager()
        , surfaceUsage()
    {
    }

};

class D3DFrameAllocator : public BaseFrameAllocator
{
public:
    D3DFrameAllocator();
    virtual ~D3DFrameAllocator();    

    virtual mfxStatus Init(mfxAllocatorParams *pParams);
    virtual mfxStatus Close();

    virtual IDirect3DDeviceManager9* GetDeviceManager()
    {
        return m_manager;
    }

    virtual mfxStatus LockFrame(mfxMemId mid, mfxFrameData *ptr);
    virtual mfxStatus UnlockFrame(mfxMemId mid, mfxFrameData *ptr);
    virtual mfxStatus GetFrameHDL(mfxMemId mid, mfxHDL *handle);

protected:
    virtual mfxStatus CheckRequestType(mfxFrameAllocRequest *request);
    virtual mfxStatus ReleaseResponse(mfxFrameAllocResponse *response);
    virtual mfxStatus AllocImpl(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);
   
    CComPtr<IDirect3DDeviceManager9> m_manager;
    CComPtr<IDirectXVideoDecoderService> m_decoderService;
    CComPtr<IDirectXVideoProcessorService> m_processorService;
    HANDLE m_hDecoder;
    HANDLE m_hProcessor;
    DWORD m_surfaceUsage;
};
