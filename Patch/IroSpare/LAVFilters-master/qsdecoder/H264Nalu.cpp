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
#include "H264Nalu.h"

H264_NaluIterator::H264_NaluIterator(const uint8_t* pBuffer, size_t bufSize, int nalSize) :
    m_NalSize(nalSize),
    m_pBuffer(pBuffer),
    m_BufSize(bufSize),
    m_StreamType((nalSize == 0) ? AnnexB : RTP),
    m_CurPos(0),     
    m_NextRTP(0),   
    m_NalPos(0),
    m_NalDataPos(0)
{
    // Stream type is constant throughout the stream.
    // RTP style streams are accompanied by extra data passed to the decoder
    //  at initialization time.
        
    // Find 1st AnnexB NALU
    if (m_StreamType == AnnexB && bufSize > 3)
        FindNextStartCode();
}

bool H264_NaluIterator::FindNextStartCode()
{
    //ASSERT(m_StreamType == AnnexB); // Sanity check

    const uint8_t* p = m_pBuffer + m_CurPos;
    const uint8_t* pEnd = m_pBuffer + m_BufSize - 4;

    while (p < pEnd)
    {
        if ((*((uint32_t*)(p)) & 0x00FFFFFF) == 0x00010000) // == 00 00 01
        {
            // Find next AnnexB NAL
            m_CurPos = p - m_pBuffer;
            return true;
        }

        ++p;
    }

    m_CurPos = m_BufSize;
    return false;
}

H264_NAL_RC H264_NaluIterator::Next()
{
    // End of stream
    if (m_CurPos + m_NalSize >= m_BufSize) return NALU_EOS;
    bool isPartial = false;

    // RTP style NALU (AVC1):
    // (XX XX) XX XX NAL..., with XX XX XX XX or XX XX equal to NAL size (big endian)
    if (m_StreamType == RTP)
    {
        // Position of next NALU
        m_NalPos = m_CurPos;
        // Position of next NALU data section
        m_NalDataPos  = m_CurPos + m_NalSize;
        
        // Get NALU size in bytes from stream (big endian)
        size_t nSize = 0;
        for (int i = 0;  i <m_NalSize; ++i)
        {
            nSize = (nSize << 8) + m_pBuffer[m_CurPos++];
        }

        // Find next NALU
        m_NextRTP += (nSize + m_NalSize);

        // Partial NALU
        if (m_NextRTP > m_BufSize)
        {
            m_CurPos = m_BufSize;
            isPartial = true;
        }

        m_CurPos = m_NextRTP;
    }
    // AnnexB style NALU:
    // (00) 00 00 01 NAL...
    else
    {
        // Remove trailing zeroes and the optional (00) BYTE:
        const size_t bufEnd =  m_BufSize - 3;
        while (m_pBuffer[m_CurPos] == 0 && ((*((uint32_t*)(m_pBuffer+m_CurPos)) & 0x00FFFFFF) != 0x00010000) && m_CurPos < bufEnd)
            ++m_CurPos;

        m_NalPos = m_CurPos;
        m_CurPos = m_NalDataPos = m_CurPos + 3;
        
        // Note: AnnexB streams usually do not end with a start code so
        //       a fragmented stream can't be identified.
        if (!FindNextStartCode())
        {
            isPartial = true;
        }
    }

    // Get NALU metadata
    m_Nal.data = m_pBuffer[m_NalDataPos];
    bool isValid = (m_Nal.forbidden_bit == 0) && IS_VALID_NALU(m_Nal.nal_unit_type);

    if (isPartial && isValid) return NALU_PARTIAL;
    return (isValid) ? NALU_OK : NALU_INVALID;
}
