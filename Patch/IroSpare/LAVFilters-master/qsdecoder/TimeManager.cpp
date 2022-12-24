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
#include "TimeManager.h"

using namespace std;
// returns true if value is between upper and lower bounds (inclusive)
#define InRange(val, lb, up) ( (val) <= (up) && (val) >= (lb))
#define GetSampleRefTime(s) CDecTimeManager::ConvertMFXTime2ReferenceTime((s)->Data.TimeStamp)

const double CDecTimeManager::fps2997 = 30.0 * 1000.0 / 1001.0;
const double CDecTimeManager::fps23976 = 24.0 * 1000.0 / 1001.0;

void CDecTimeManager::Reset()
{
    m_nOutputFrames = -1;
    m_nSegmentSampleCount = 0;
    m_OutputTimeStamps.clear();
    m_bValidFrameRate = false;
    m_bCalculatedPts = false;
    m_nLastSeenFieldDoubling = 0;
    m_rtPrevStart = INVALID_REFTIME;
    m_bIsSampleInFields = false;
    SetInverseTelecine(false);
}

bool CDecTimeManager::CalcPtsOrder(const TFrameVector& frames)
{
    if (m_bCalculatedPts)
        return true;

    // Find if the time stamps are PTS (presentation) or DTS (decoding).
    // PTS time stamps are monotonic.
    // this is important for deriving correct time stamp in GetSampleTimeStamp
    m_bIsPTS = true;
    REFERENCE_TIME prevStart = GetSampleRefTime(frames[0]);
    for (size_t i = 1; i < frames.size(); ++i)
    {
        const REFERENCE_TIME& rtStart = GetSampleRefTime(frames[i]);
        if (INVALID_REFTIME != prevStart)
        {
            // not monotonic:
            if (rtStart < prevStart)
            {
                m_bIsPTS = false;
                break;
            }
        }

        prevStart = rtStart;
    }

    m_bCalculatedPts = true;
    return true;
}

void CDecTimeManager::SetInverseTelecine(bool bIvtc)
{
    // Nothing changed
    bIvtc = bIvtc && m_bEnableIvtc;

    if (bIvtc == m_bIvtc)
        return;
    
    m_bIvtc = bIvtc;
    if (m_bIvtc)
    {
        FixFrameRate(fps23976);
        MSDK_TRACE("QsDecoder: entering IVTC\n");
    }
    else
    {
        FixFrameRate(fps2997);
        MSDK_TRACE("QsDecoder: leaving IVTC\n");
    }
}

bool CDecTimeManager::GetSampleTimeStamp(const TFrameVector& frames,
                                         REFERENCE_TIME& rtStart)
{
    if (frames.empty())
        return false;

    const mfxFrameSurface1* pSurface = frames[0];
    if (!m_bCalculatedPts)
    {
        CalcPtsOrder(frames);
    }

    // Check if frame rate has changed
    double tmpFrameRate;
    if (!m_bIvtc && CalcCurrentFrameRate(tmpFrameRate, frames.size()))
    {
        FixFrameRate(tmpFrameRate);
    }

    bool bFieldDoubling = (0 != (pSurface->Info.PicStruct & MFX_PICSTRUCT_FIELD_REPEATED));
    const REFERENCE_TIME rtDecoder = GetSampleRefTime(pSurface);

    ++m_nLastSeenFieldDoubling;

    // Enter inverse telecine mode
    if (bFieldDoubling)
    {
        SetInverseTelecine(true);
        m_nLastSeenFieldDoubling = 0;
    }
    // Return to normal frame rate due to content change
    else if (m_nLastSeenFieldDoubling > 1) //m_dFrameRate)
    {
        SetInverseTelecine(false);
    }

    ++m_nOutputFrames;

    // Can't start the sequence - drop frame
    if (rtDecoder == INVALID_REFTIME && m_rtPrevStart == INVALID_REFTIME)
    {
        return false;
    }

    // Find smallest valid time stamp for first frame
    rtStart = rtDecoder;

    // First frame in a new frame sequence (first frame after a stop or a seek)
    // should always be a keyframe
    if (m_rtPrevStart == INVALID_REFTIME)
    {
        // Presentation time stamps (PTS).
        // Time stamp should be OK (in presentation order)
        if (m_bIsPTS)
        {
            // Easy case - rtDecoder is the time stamp to use
            if (rtDecoder != INVALID_REFTIME)
            {
                rtStart = rtDecoder;
                auto it = m_OutputTimeStamps.find(rtDecoder);
                
                ASSERT(it != m_OutputTimeStamps.end());
                if (it != m_OutputTimeStamps.end())
                {
                    m_OutputTimeStamps.erase(it);
                }
            }
            // Need to calculate time stamp from future frames
            else if (!m_OutputTimeStamps.empty())
            {
                size_t count = 0;

                // Note - m_OutputTimeStamps contains only valid time stamps - take the smallest
                rtStart = *(m_OutputTimeStamps.begin());

                // Find distance from current sample
                for (size_t i = 1; i < frames.size(); ++i)
                {
                    ++count;
                    REFERENCE_TIME t = GetSampleRefTime(frames[i]);
                    if (rtStart == t)
                    {
                        break;
                    }
                }

                // Take negative offset from this future time stamp
                rtStart = rtStart - (REFERENCE_TIME)(0.5 + (1e7 * count) / m_dFrameRate);
            }
            // Can't derive time stamp, frame will be dropped :(
            else
            {
                return false;
            }
        }
        // Decoding time stamps (DTS) - a little tricky, might not be 100%
        else
        {
            // Note - m_OutputTimeStamps contains only valid time stamps - take the smallest
            if (!m_OutputTimeStamps.empty())
            {
                auto it = m_OutputTimeStamps.begin();
                rtStart = *(it);
                m_OutputTimeStamps.erase(it);
            }
            // Can't derive time stamp, frame will be dropped :(
            else
            {
                return false;
            }
        }
    }
    // 2nd and above frames
    else 
    {
        if (m_dFrameRate > 0 || INVALID_REFTIME == rtDecoder)
        {
            rtStart = m_rtPrevStart + (REFERENCE_TIME)(0.5 + 1e7 / m_dFrameRate);

            auto it = m_OutputTimeStamps.begin();
            if (it != m_OutputTimeStamps.end())
            {
                REFERENCE_TIME rtTemp = *it;

                // Check if the lowest timetamp is very far (100ms) than the expected timestamp
                if (!m_bIvtc && abs(rtTemp - rtStart) > 1000000)
                {
                    MSDK_TRACE("QsDecoder: Warning detected long time stamp gap!\n");
                    rtStart = rtTemp;
                    m_OutputTimeStamps.erase(it);
                }
                // Remove lowest timestamp if it's very close to the expected timestamp
                else if (rtTemp < rtStart || abs(rtTemp - rtStart) < 25000) // diff is less than 2.5ms
                {
                    m_OutputTimeStamps.erase(it);
                }
            }
        }
        else
        {
            auto it = m_OutputTimeStamps.begin();
            if (it != m_OutputTimeStamps.end())
            {
                rtStart = *it;
                m_OutputTimeStamps.erase(it);
            }
            else
            {
                rtStart = INVALID_REFTIME;
                return false;
            }
        }
    }   

    m_rtPrevStart = rtStart;
    return true;
}

void CDecTimeManager::AddOutputTimeStamp(mfxFrameSurface1* pSurface)
{
    REFERENCE_TIME rtStart = ConvertMFXTime2ReferenceTime(pSurface->Data.TimeStamp);
    if (rtStart != INVALID_REFTIME)
    {
        m_OutputTimeStamps.insert(rtStart);
    }
}

void CDecTimeManager::OnVideoParamsChanged(double frameRate)
{
    if (frameRate < 1)
        return;

    FixFrameRate(frameRate);

    // When this event happens we leave ivtc.
    SetInverseTelecine(false);
}

void CDecTimeManager::FixFrameRate(double frameRate)
{
    // Too close - probably inaccurate calculations
    if (fabs(m_dFrameRate - frameRate) < 0.001)
        return;

    // Modify previous time stamp to reflect frame rate change
    if (m_dFrameRate > 1 && m_rtPrevStart != INVALID_REFTIME)
    {
        m_rtPrevStart += (REFERENCE_TIME)(0.5 + 1e7 / m_dFrameRate);
        m_rtPrevStart -= (REFERENCE_TIME)(0.5 + 1e7 / frameRate);
    }

    m_dFrameRate = frameRate;
    MSDK_TRACE("QsDecoder: frame rate is %0.3f\n", (float)(m_dFrameRate));
}

bool CDecTimeManager::CalcCurrentFrameRate(double& frameRate, size_t nQueuedFrames)
{
    frameRate = 0;
    size_t len = m_OutputTimeStamps.size();

    // Need enough frames and that the time stamps are all valid.
    if (len < 4 || nQueuedFrames > len)
        return false;

    vector<REFERENCE_TIME> deltaTimes;
    REFERENCE_TIME prev = INVALID_REFTIME;
    for (auto it = m_OutputTimeStamps.begin(); it != m_OutputTimeStamps.end(); ++it)
    {
        if (INVALID_REFTIME == prev)
        {
            prev = *it;
        }
        else
        {
            REFERENCE_TIME t = *it ;
            deltaTimes.push_back(t - prev);
            prev = t;
        }
    }

    // Check for consistency
    len = deltaTimes.size();
    REFERENCE_TIME d = deltaTimes[0];
    bool accurateFR = true;
    for (size_t i = 1; i < len; ++i)
    {
        // Check if less than 1ms apart
        // Note: many times time stamps are rounded to the nearest ms.
        if (abs(d - deltaTimes[i]) > 12000)
        {
            accurateFR = false;
            break;
        }
    }

    // Can't measure accurately...
    if (!accurateFR)
        return false;

    frameRate = (1e7 * len) / (*m_OutputTimeStamps.rbegin() - *m_OutputTimeStamps.begin());
    if (fabs(frameRate - m_dFrameRate) > 1)
    {
        // Fine tune the frame rate
        // Try NTSC ranges
        if (m_dFrameRate == 0 /* no known frame rate */ || InRange(m_dFrameRate, 59.93, 59.95) || InRange(m_dFrameRate, 29.96, 29.98) || InRange(m_dFrameRate, 23.96, 23.98))
        {
            if (InRange(frameRate, 28.0, 32.0))
            {
                frameRate = fps2997;
            }
            else if (InRange(frameRate, 22.0, 26.0))
            {
                frameRate = fps23976;
            }
            else
            {
                frameRate = floor(frameRate + 0.5);
            }
        }
        // PC/PAL ranges
        else
        {
            frameRate = floor(frameRate + 0.5);
        }

        return frameRate <= MAX_FRAME_RATE;
    }

    return false;
}
