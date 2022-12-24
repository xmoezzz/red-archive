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

class CFrameConstructor
{
public:
    CFrameConstructor(CDecTimeManager* tsManager);
    virtual ~CFrameConstructor();
    virtual mfxStatus ConstructHeaders(VIDEOINFOHEADER2* vih, const GUID& guidFormat, size_t nMtSize, size_t nVideoInfoSize);
    virtual mfxStatus ConstructFrame(IMediaSample* pSample, mfxBitstream* pBS);
    virtual void Reset();
    void SaveResidualData(mfxBitstream* pBS);
    inline mfxBitstream& GetHeaders() { return m_Headers; }
    void SetDvdPacketStripping(bool stripPackets) { m_bDvdStripPackets = stripPackets; }

protected:
    inline void UpdateTimeStamp(IMediaSample* pSample, mfxBitstream* pBS);
    inline void WriteResidualData(mfxU8*& pData);
    inline void WriteHeaders(mfxU8*& pData);
    inline void WriteSampleData(mfxU8*& pData, const mfxU8* pSrc, size_t nDataSize);
    static void StripDvdPacket(BYTE*& p, int& len);

    CDecTimeManager* m_TimeManager;
    bool m_bSeqHeaderInserted;
    bool m_bDvdStripPackets;
    mfxBitstream m_Headers; 
    mfxBitstream m_ResidialBS;
};

////////////////////////////////////////////////////////////////////////////////////////////

class CVC1FrameConstructor : public CFrameConstructor
{
public:    
    CVC1FrameConstructor(CDecTimeManager* tsManager);
    mfxStatus ConstructFrame(IMediaSample* pSample, mfxBitstream* pBS);
    mfxStatus ConstructHeaders(VIDEOINFOHEADER2* vih, const GUID& guidFormat, size_t nMtSize, size_t nVideoInfoSize);

protected:
    mfxStatus ConstructHeaderSM(mfxU8* pHeaderSM, mfxU32 nHeaderSize, mfxU8* pDataBuffer, mfxU32 nDataSize);
    bool StartCodeExist(mfxU8* pStart);

    FOURCC m_FourCC;
    DWORD m_Width, m_Height;
};

////////////////////////////////////////////////////////////////////////////////////////////

class CAVCFrameConstructor : public CFrameConstructor
{
public:
    CAVCFrameConstructor(CDecTimeManager* tsManager);
    ~CAVCFrameConstructor();
    mfxStatus ConstructFrame(IMediaSample* pSample, mfxBitstream* pBS);
    mfxStatus ConstructHeaders(VIDEOINFOHEADER2* vih,
        const GUID& guidFormat,
        size_t nMtSize,
        size_t nVideoInfoSize);
    void Reset();

private:
    mfxU32             m_NalSize; 
    mfxU32             m_HeaderNalSize; 
    mfxU8              m_H264StartCode[4];
    std::vector<mfxU8> m_InputBuffer;
    std::vector<mfxU8> m_OutputBuffer;       // Used for building the output stream
};
