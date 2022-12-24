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

enum H264_STREAM_TYPE
{
    RTP     = 1, // No start codes - prefixed with length field
    AnnexB  = 2  // With start codes (00 00 00 01) - first 00 is optional
};

enum NALU_TYPE
{
    NALU_TYPE_SLICE     = 1,
    NALU_TYPE_DPA       = 2,
    NALU_TYPE_DPB       = 3,
    NALU_TYPE_DPC       = 4,
    NALU_TYPE_IDR       = 5,
    NALU_TYPE_SEI       = 6,
    NALU_TYPE_SPS       = 7,
    NALU_TYPE_PPS       = 8,
    NALU_TYPE_AUD       = 9,
    NALU_TYPE_EOSEQ     = 10,
    NALU_TYPE_EOSTREAM  = 11,
    NALU_TYPE_FILL      = 12,
    NALU_TYPE_MAX_VALID = 12
};

 enum NALU_REF_IDC
 {
    NALU_PRIORITY_DISPOSABLE  = 0,
    NALU_PRIORITY_LOW         = 1,
    NALU_PRIORITY_HIGH        = 2,
    NALU_PRIORITY_HIGHEST     = 3
 };

union H264_NAL
{
    uint8_t data;
    struct
    {
        NALU_TYPE nal_unit_type     : 5;
        unsigned  nal_reference_idc : 2; /* VS2010 treat  NALU_REF_IDC as signed - should be unsigned*/
        unsigned  forbidden_bit     : 1;
    };
};

enum H264_NAL_RC
{
    NALU_OK      = 0,  // Full NAL
    NALU_PARTIAL = 1,  // partial - need to resend the partial packet concatinated to the next packet
    NALU_INVALID = 2,  // Invalid attributes - decoder should usually discard this one
    NALU_EOS     = 3   // End of stream - nothing to output
};

#define IS_VALID_NALU(n) (n > 0 && n <= NALU_TYPE_MAX_VALID)

class H264_NaluIterator
{
public:
    H264_NaluIterator(const uint8_t *pBuffer, size_t bufSize, int nalSize);
    H264_STREAM_TYPE GetStreamType() const { return m_StreamType; }
    NALU_TYPE      GetNaluType() const { return m_Nal.nal_unit_type; }
    NALU_REF_IDC   GetNaluRefIdc() const { return (NALU_REF_IDC)m_Nal.nal_reference_idc; }
    bool           IsRefFrame() const { return (m_Nal.nal_reference_idc != NALU_PRIORITY_DISPOSABLE); }
    size_t         GetDataLength() const { return m_CurPos - m_NalDataPos; }
    const uint8_t* GetDataBuffer() const { return m_pBuffer + m_NalDataPos; }
    size_t         GetNalLength() const { return m_CurPos - m_NalPos; }
    const uint8_t* GetNALBuffer() const { return m_pBuffer + m_NalPos; }
    bool IsEOF() const { return m_CurPos >= m_BufSize; }
    H264_NAL_RC Next();

private:
    // No copying supported!
    H264_NaluIterator(const H264_NaluIterator&);
    H264_NaluIterator& operator=(const H264_NaluIterator&);

    bool FindNextStartCode();

    // data members
    const int        m_NalSize;
    const uint8_t*   m_pBuffer;
    const size_t     m_BufSize;
    H264_STREAM_TYPE m_StreamType;  // Either AVC or AnnexB
    H264_NAL         m_Nal;
    size_t           m_CurPos;
    size_t           m_NextRTP;
    size_t           m_NalPos;      // NALU start (including startcode / size)
    size_t           m_NalDataPos;  // Data part of NALU
};
