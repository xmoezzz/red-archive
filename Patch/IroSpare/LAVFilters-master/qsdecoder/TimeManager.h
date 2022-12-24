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

#define MFX_TIME_STAMP_FREQUENCY 90000L
#define MFX_TIME_STAMP_INVALID   (mfxU64)(-1)
#define MFX_TIME_STAMP_MAX       ((mfxI64)100000 * (mfxI64)MFX_TIME_STAMP_FREQUENCY)
#define INVALID_REFTIME          _I64_MIN
#define MAX_FRAME_RATE           125

struct TTimeStampInfo
{
    TTimeStampInfo(int _id = 0, REFERENCE_TIME _rtStart = 0) :
        id(_id), rtStart(_rtStart)
    {
    }

    int id;
    REFERENCE_TIME rtStart;
};

typedef std::deque<TTimeStampInfo> TTimeStampQueue;
typedef std::multiset<REFERENCE_TIME> TSortedTimeStamps;
typedef std::vector<mfxFrameSurface1*> TFrameVector;

class CDecTimeManager
{
public:
    CDecTimeManager(bool bEnableIvtc = true) : m_dOrigFrameRate(0), m_dFrameRate(0), m_bIvtc(false), m_bEnableIvtc(bEnableIvtc), m_bEnabled(true)
    {
        Reset();
    }

    double  GetFrameRate() { return m_dFrameRate; }
    void    SetFrameRate(double frameRate, bool bIsFields)
    {
        m_bIsSampleInFields = bIsFields;
        // Check for invalid values
        if (frameRate < 1 || frameRate > MAX_FRAME_RATE)
        {
            m_dFrameRate = 0;
        }
        else
        {
            m_dFrameRate = (bIsFields && frameRate < 30.0) ? (frameRate / 2) : frameRate;
        }
        m_dOrigFrameRate = m_dFrameRate;
        MSDK_TRACE("QsDecoder: frame rate is %0.2f\n", (float)(m_dFrameRate));
    }

    bool& Enabled() { return m_bEnabled; }
    void Reset();
    void SetInverseTelecine(bool bIvtc);
    inline bool GetInverseTelecine() { return Enabled() && m_bIvtc; }
    inline bool HasValidFrameRate() { return m_bValidFrameRate || m_dFrameRate == 0; }

    bool IsValidTimeStamp(REFERENCE_TIME rtTime)
    {
        return (Enabled()) ? rtTime != MFX_TIME_STAMP_INVALID : rtTime != INVALID_REFTIME;
    }

    mfxU64 ConvertReferenceTime2MFXTime(REFERENCE_TIME rtTime)
    {
        if (!Enabled()) return (mfxU64)rtTime;

        if (-1e7 == rtTime || INVALID_REFTIME == rtTime)
            return MFX_TIME_STAMP_INVALID;

        return (mfxU64)(((double)rtTime / 1e7) * (double)MFX_TIME_STAMP_FREQUENCY);
    }

    REFERENCE_TIME ConvertMFXTime2ReferenceTime(mfxU64 nTime)
    {
        if (!Enabled()) return (REFERENCE_TIME)nTime;

        if (MFX_TIME_STAMP_INVALID == nTime)
            return (REFERENCE_TIME)INVALID_REFTIME;

        mfxI64 t = (mfxI64)nTime; // Don't lose the sign

        if (t < -MFX_TIME_STAMP_MAX || t > MFX_TIME_STAMP_MAX)
            return (REFERENCE_TIME)INVALID_REFTIME;

        return (REFERENCE_TIME)(((double)t / (double)MFX_TIME_STAMP_FREQUENCY) * 1e7);
    }

    void AddOutputTimeStamp(mfxFrameSurface1* pSurface);
    bool CalcPtsOrder(const TFrameVector& frames);
    bool GetSampleTimeStamp(const TFrameVector& frames,
                            REFERENCE_TIME& rtStart);
    bool IsSampleInFields() { return m_bIsSampleInFields; }
    void OnVideoParamsChanged(double frameRate);
    REFERENCE_TIME GetLastTimeStamp() { return m_rtPrevStart; }

    static const double fps2997;
    static const double fps23976;

protected:
    void FixFrameRate(double frameRate);
    bool CalcCurrentFrameRate(double& frameRate, size_t nQueuedFrames);

    bool   m_bEnabled;
    double m_dOrigFrameRate;
    double m_dFrameRate;
    bool   m_bValidFrameRate;
    bool   m_bIvtc;
    int    m_nLastSeenFieldDoubling;
    bool   m_bIsPTS; // True for Presentation Time Stamps (input time stamps). Output is always PTS.
    bool   m_bCalculatedPts;
    int    m_nSegmentSampleCount;
    int    m_nOutputFrames;
    bool   m_bIsSampleInFields;
    bool   m_bEnableIvtc;
    REFERENCE_TIME m_rtPrevStart;
    TSortedTimeStamps m_OutputTimeStamps;
};
