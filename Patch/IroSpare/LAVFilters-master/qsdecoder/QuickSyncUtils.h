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

#pragma once

extern "C" 
{
    // SSE4.1 based memcpy that copies from video memory to system memory
    void* gpu_memcpy_sse41(void* d, const void* s, size_t _size);
    // AVX2 based memcpy that copies from video memory to system memory
    void* gpu_memcpy_avx2(void* d, const void* s, size_t size);
    void* mt_memcpy(void* d, const void* s, size_t size);
    void* mt_gpu_memcpy(void* d, const void* s, size_t size);
}

// Finds greatest common divider
mfxU32 GCD(mfxU32 a, mfxU32 b);

// Pixel Aspect Ratio to Display Aspect Ratio conversion
mfxStatus PARtoDAR(DWORD parw, DWORD parh, DWORD w, DWORD h, DWORD& darw, DWORD& darh);

// Display Aspect Ratio to Pixel Aspect Ratio conversion
mfxStatus DARtoPAR(mfxU32 darw, mfxU32 darh, mfxU32 w, mfxU32 h, mfxU16& parw, mfxU16& parh);

// Name from codec identifier (MSDK enum)
const char* GetCodecName(DWORD codec);

// Name of codec's profile - profile identifier is accoding to DirectShow
const char* GetProfileName(DWORD codec, DWORD profile);

// Returns true when running on SSE4.1 HW - Intel Penryn or newer.
bool IsSSE41Enabled();

// Returns true when running on AVX2 HW - Intel 4th generation Core (Haswell) or newer
bool IsAVX2Enabled();

// Set current thread name in VS debugger
void SetThreadName(LPCSTR szThreadName, DWORD dwThreadID = -1 /* current thread */);

// Get an Intel GPU ID using Direct3D9 (screen must be connected to Intel GPU)
int GetIntelAdapterIdD3D9(IDirect3D9* _pd3d);

// Get the ID of the adapter (GPU) associated with an MSDK session, -1 for SW session
int GetMSDKAdapterNumber(mfxSession session);

#ifdef _DEBUG
// Print assert to debugger output
void DebugAssert(const TCHAR *pCondition,const TCHAR *pFileName, int iLine);
#endif

typedef void* (*Tmemcpy)(void*, const void*, size_t);

// Wrapper for low level critical section
class CQsLock
{
    friend class CQsAutoLock;
    friend class CQsAutoUnlock;
public:
    CQsLock()  { InitializeCriticalSection(&m_CritSec); }
    ~CQsLock() { DeleteCriticalSection(&m_CritSec); }
    __forceinline bool IsLocked()
    {
        if (TryEnterCriticalSection(&m_CritSec))
        {
            // Not locked
            LeaveCriticalSection(&m_CritSec);
            return false;
        }

        return true;
    }

private:
    // Lock and Unlock are private and can only be called from
    // the CQsAutoLock/CQsAutoUnlock classes
    __forceinline void Lock()   { ASSERT(this != NULL); EnterCriticalSection(&m_CritSec); }
    __forceinline void Unlock() { ASSERT(this != NULL); LeaveCriticalSection(&m_CritSec); }

    // Make copy constructor and assignment operator inaccessible
    DISALLOW_COPY_AND_ASSIGN(CQsLock)
    CRITICAL_SECTION m_CritSec;
};

// Locks a critical section, and unlocks it automatically
// when the auto-lock object goes out of scope
class CQsAutoLock
{
public:
    __forceinline CQsAutoLock(CQsLock* plock)
    {
        m_pLock = plock;
        if (m_pLock != NULL)
            m_pLock->Lock();
    }

    __forceinline ~CQsAutoLock()
    {
        if (m_pLock != NULL)
            m_pLock->Unlock();
    }

private:
    DISALLOW_COPY_AND_ASSIGN(CQsAutoLock);
    CQsLock* m_pLock;
};

// Unlocks a critical section, and locks it automatically
// when the auto-unlock object goes out of scope
class CQsAutoUnlock
{
public:
    __forceinline CQsAutoUnlock(CQsLock* plock)
    {
        m_pLock = plock;
        if (m_pLock != NULL)
            m_pLock->Unlock();
    }

    __forceinline ~CQsAutoUnlock()
    {
        if (m_pLock != NULL)
            m_pLock->Lock();
    }

private:
    DISALLOW_COPY_AND_ASSIGN(CQsAutoUnlock);
    CQsLock* m_pLock;
};

// Simple SSE friendly buffer class
class CQsAlignedBuffer
{
public:
    CQsAlignedBuffer(size_t bufferSize) : m_BufferSize(bufferSize)
    {
        m_Buffer = (BYTE*)_aligned_malloc(m_BufferSize, 64);
    }

    ~CQsAlignedBuffer()
    {
        _aligned_free(m_Buffer);
    }

    __forceinline size_t GetBufferSize() { return m_BufferSize; }
    __forceinline BYTE* GetBuffer() { return m_Buffer; }

private:
    size_t m_BufferSize;
    BYTE* m_Buffer;
};

class CQsTimer
{
public:
    CQsTimer()
    {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        m_Frequency = freq.QuadPart;

        // Calibration
        Start();
        Stop();

        m_Correction = m_Stop.QuadPart - m_Start.QuadPart;
    }

    __forceinline void Start()
    {
        // Ensure we will not be interrupted by any other thread for a while
        Sleep(0);
        QueryPerformanceCounter(&m_Start);
    }

    __forceinline void Stop()
    {
        QueryPerformanceCounter(&m_Stop);
    }

    __forceinline double GetDuration() const
    {
        return (double)(m_Stop.QuadPart - m_Start.QuadPart - m_Correction) * 1000000.0 / m_Frequency;
    }


protected:
    LARGE_INTEGER m_Start;
    LARGE_INTEGER m_Stop;

    LONGLONG m_Frequency;
    LONGLONG m_Correction;
};
