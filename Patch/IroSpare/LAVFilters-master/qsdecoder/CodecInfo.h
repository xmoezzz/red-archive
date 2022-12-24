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

//
// FourCC
//

// VC1
#define FOURCC_VC1  mmioFOURCC('W','V','C','1')
#define FOURCC_WMV3 mmioFOURCC('W','M','V','3')

//MPEG2
#define FOURCC_mpg2 mmioFOURCC('m','p','g','2')
#define FOURCC_MPG2 mmioFOURCC('M','P','G','2')

//H264
#define FOURCC_H264 mmioFOURCC('H','2','6','4')
#define FOURCC_X264 mmioFOURCC('X','2','6','4')
#define FOURCC_h264 mmioFOURCC('h','2','6','4')
#define FOURCC_avc1 mmioFOURCC('a','v','c','1')
#define FOURCC_VSSH mmioFOURCC('V','S','S','H')
#define FOURCC_DAVC mmioFOURCC('D','A','V','C')
#define FOURCC_PAVC mmioFOURCC('P','A','V','C')
#define FOURCC_AVC1 mmioFOURCC('A','V','C','1')
#define FOURCC_CCV1 mmioFOURCC('C','C','V','1')

// Output formats
#define FOURCC_NV12 mmioFOURCC('N','V','1','2')

enum
{
    QS_PROFILE_MPEG2_422                = 0, /* 4:2:2 or 4:2:0 */
    QS_PROFILE_MPEG2_HIGH               = 1, /* 4:2:2 or 4:2:0 */
    QS_PROFILE_MPEG2_SPATIALLY_SCALABLE = 2, /* 4:2:0 */
    QS_PROFILE_MPEG2_SNR_SCALABLE       = 3, /* 4:2:0 */
    QS_PROFILE_MPEG2_MAIN               = 4, /* 4:2:0 */
    QS_PROFILE_MPEG2_SIMPLE             = 5  /* 4:2:0 */
};

enum
{
    // profile modifiers
    QS_PROFILE_H264_CONSTRAINED = (1<<9),  // 8+1; constraint_set1_flag
    QS_PROFILE_H264_INTRA       = (1<<11), // 8+3; constraint_set3_flag

    QS_PROFILE_H264_BASELINE             = 66,
    QS_PROFILE_H264_CONSTRAINED_BASELINE = (66|QS_PROFILE_H264_CONSTRAINED),
    QS_PROFILE_H264_MAIN                 = 77,
    QS_PROFILE_H264_EXTENDED             = 88,
    QS_PROFILE_H264_HIGH                 = 100,
    QS_PROFILE_H264_HIGH_10              = 110,
    QS_PROFILE_H264_HIGH_10_INTRA        = (110|QS_PROFILE_H264_INTRA),
    QS_PROFILE_H264_HIGH_422             = 122,
    QS_PROFILE_H264_HIGH_422_INTRA       = (122|QS_PROFILE_H264_INTRA),
    QS_PROFILE_H264_HIGH_444             = 144,
    QS_PROFILE_H264_HIGH_444_PREDICTIVE  = 244,
    QS_PROFILE_H264_HIGH_444_INTRA       = (244|QS_PROFILE_H264_INTRA),
    QS_PROFILE_H264_CAVLC_444            = 44,
    QS_PROFILE_H264_MULTIVIEW_HIGH       = 118,
    QS_PROFILE_H264_STEREO_HIGH          = 128
};

enum
{
    QS_PROFILE_VC1_SIMPLE   = 0,
    QS_PROFILE_VC1_MAIN     = 1,
    QS_PROFILE_VC1_COMPLEX  = 2,
    QS_PROFILE_VC1_ADVANCED = 3
};