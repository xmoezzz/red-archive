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

#define QS_DEC_DLL_NAME "IntelQuickSyncDecoder.dll"
#define QS_DEC_VERSION  "v0.45"

// Forward declarations
struct IDirect3DDeviceManager9;
struct IMediaSample;

// Return value of the check function.
// Caps are bitwise OR of the following values
enum QsCaps
{
    QS_CAP_UNSUPPORTED      = 0,
    QS_CAP_HW_ACCELERATION  = 1,
    QS_CAP_SW_EMULATION     = 2,
    QS_CAP_DEINTERLACING    = 4,
    QS_CAP_DETAIL           = 8,
    QS_CAP_DENOISE          = 16,
    QS_CAP_PROCAMP          = 32,
    QS_CAP_SCALING          = 64,
    QS_CAP_DX11
};

enum QsFieldOrder
{
    QS_FIELD_AUTO = 0,
    QS_FIELD_TFF  = 1,
    QS_FIELD_BFF  = 2
};

enum QsOutputSurfaceType
{
    QS_SURFACE_SYSTEM              = 0, // surface is pointers to yuv in system memory
    QS_SURFACE_GPU                 = 1, // surface is pointers to yuv in GPU memory, must use gpu_memcpy
    //QS_SURFACE_DXVA_MEDIA_SAMPLE   = 2, // pMediaSample pointer is active, uv pointers are NULL.
};

// This struct holds an output frame + meta data
struct QsFrameData
{
    QsFrameData() { memset(this, 0, sizeof(QsFrameData)); }

    enum QsFrameType
    {
        Invalid = 0,
        I       = 1,
        P       = 2,
        B       = 3
    };

    // QsFrameStructure affects how color information is stored within a frame
    // 4:2:0 example:
    // fsProgressiveFrame: uv[0][0] affects y[0-1][0-1]
    // fsInterlacedFrame   uv[0][0] affects y[0][0-1] and y[2][0-1]
    enum QsFrameStructure
    {
        fsProgressiveFrame = 0,  // Note: a progressive frame can hold interlaced content
        fsInterlacedFrame  = 1,  // Two fields
        fsField            = 2   // Single field
    };

    // Pointers to data buffer
    // Memory should not be freed externally!
    // Packed UV surfaces (NV12) should use the 'u' pointer.
    union { unsigned char* y; unsigned char* red; IMediaSample* pMediaSample; };
    union { unsigned char* u; unsigned char* green; };
    union { unsigned char* v; unsigned char* blue;  };
    union { unsigned char* a; unsigned char* alpha; };

    DWORD            fourCC;             // Standard fourCC codes. Limited to NV12 in this version.
    RECT             rcFull;             // Note: these RECTs are according to WIN32 API standard (not DirectShow)
    RECT             rcClip;             // They hold the coordinates of the top-left and bottom right pixels
                                         // So expect values like {0, 0, 1919, 1079} for 1080p.
    DWORD            dwStride;           // Line width + padding (in bytes). Distance between lines. Always modulu 16.
    REFERENCE_TIME   rtStart, rtStop;    // Start and stop time stamps. Ther latter is always rtStart+1
    DWORD            dwInterlaceFlags;   // Same as dwTypeSpecificFlags (in AM_SAMPLE2_PROPERTIES)
    bool             bFilm;              // true only when a frame has a double field attribute (AM_VIDEO_FLAG_REPEAT_FIELD)
    DWORD            dwPictAspectRatioX; // Display aspect ratio (NOT pixel aspect ratio)
    DWORD            dwPictAspectRatioY;
    QsFrameType      frameType;          // Mainly used for future capability. Will always return I.
    QsFrameStructure frameStructure;     // See QsFrameStructure enum comments
    bool             bReadOnly;          // If true, the frame's content can be overwritten (most likely bReadOnly will remain false forever)
    bool             bCorrupted;         // If true, the HW decoder reported corruption in this frame
    unsigned reserved[20];
};

// config for QuickSync component
struct CQsConfig
{
    CQsConfig()
    {
        memset(this, 0, sizeof(CQsConfig));
    }

    // misc
    union
    {
        unsigned misc;
        struct
        {
            unsigned nOutputQueueLength       :  6; // use a minimum of 8 frame for more accurate frame rate calculations
            bool     bMod16Width              :  1; // deprecated
            bool     bEnableMultithreading    :  1; // enable worker threads for low latency decode (better performance, more power)
            bool     bTimeStampCorrection     :  1; // True: time stamp will be generated.
                                                    // False: DS filter may do this.
            bool     bEnableMtCopy            :  1; // enables MT frame copy
            bool     depr_bEnableMtDecode     :  1; // deprecated
            bool     depr_bEnableMtProcessing :  1; // deprecated
            bool     bEnableVideoProcessing   :  1; // Master switch for DI, denoise, detail, etc.
            bool     bEnableSwEmulation       :  1; // When true, a SW version of the decoder will be used (if possible) if HW fails
            bool     bForceFieldOrder         :  1; // When true decoder interlacing flags are overwriten
            unsigned eFieldOrder              :  2; // When forced DI is used, this will mark if the progressive frames as TFF or BFF
            bool     bDropDuplicateFrames     :  1; // True: duplicated frames will be dropped
                                                    // False: when a frame is marked as duplicated it will be sent out with a modified time
                                                    //        stamp based on the first frame and frame rate. Useful for transcoding.
            bool     bEnableD3D11             :  1; // Enable use of Direct3D 11.1 for HW acceleration (Windows 8 and newer OS)
            bool     bDefaultToD3D11          :  1; // Prefare D3D11 over D3D9.
            unsigned reserved1                : 12;
        };
    };

    // Codec support
    union
    {
        unsigned codecs;
        struct
        {
            bool  bEnableDvdDecoding :  1;
            bool  bEnableH264        :  1;
            bool  bEnableMPEG2       :  1;
            bool  bEnableVC1         :  1;
            bool  bEnableWMV9        :  1;
            unsigned reserved2       : 27;
        };
    };

    // Video post processing options
    union
    {
        unsigned vpp;
        struct
        {
            bool     bVppEnableDeinterlacing             :  1; // When true, DI will be used on interlaced content
            bool     bVppEnableFullRateDI                :  1; // true-> double frame rate
            unsigned nVppDetailStrength                  :  7; // Values are [0-64], 0 - disabled, 64 - full.
            unsigned nVppDenoiseStrength                 :  7; // Values are [0-64], 0 - disabled, 64 - full.
            bool     bVppEnableDITimeStampsInterpolation :  1; // Make sure deinterlaced frames have proper time stamps.
                                                               // MSDK will not produce time stamps for interpolated frames by default.
            bool     bVppEnableForcedDeinterlacing       :  1; // DI will always work - forces interlacing flags on decoded progressive frames and deinterlaces them,
                                                               // uses eFieldOrder as hint:
                                                               // QS_FIELD_AUTO - turn progressive frames' flags to the last encountered interlaced flag. Default to TFF
                                                               // QS_FIELD_TFF - deinterlace progressive frames using TFF flags
                                                               // QS_FIELD_BFF - deinterlace progressive frames using BFF flags

            unsigned reserved3                           : 14;
        };
    };
};

// Interafce to QuickSync component
struct IQuickSyncDecoder
{
    typedef HRESULT (*TQS_DeliverSurfaceCallback) (void* obj, QsFrameData* data);

    // Useless constructor to keep several compilers happy...
    IQuickSyncDecoder() {}

    // Object is OK
    virtual bool getOK() = 0;
    
    // Test if the decoder supports the media type
    // Return codes are:
    // S_OK - support with HW acceleration.
    // S_FALSE - support with SW implementation
    // E_FAIL - no support
    // Other errors indicate bad parameters.
    virtual HRESULT TestMediaType(const AM_MEDIA_TYPE* mtIn, FOURCC fourCC) = 0;

    // Initialize decoder
    virtual HRESULT InitDecoder(const AM_MEDIA_TYPE* mtIn, FOURCC fourCC) = 0;

    // Decode a sample. Decoded frames are delivered via a callback.
    // Call SetDeliverSurfaceCallback to specify the callback
    virtual HRESULT Decode(IMediaSample* pIn) = 0;

    // Flush decoder
    // When deliverFrames == true, the frames will be delivered via a callback (same as Decode).
    virtual HRESULT Flush(bool deliverFrames = true) = 0;

    // OnSeek/NewSegment - marks the decoding of a new segment.
    // Resets the decoder discarding old data.
    virtual HRESULT OnSeek(REFERENCE_TIME segmentStart) = 0;

    // Marks the start of a flush. All calls to Decode will be ignored.
    // Usually called asynchroniously from the application thread.
    virtual HRESULT BeginFlush() = 0;
    
    // Marks the end of a flush. All calls to Decode after this function is called will be accepted.
    // Usually called asynchroniously from the application thread.
    // An implicit OnSeek call will be generated after the next Decode call.
    virtual HRESULT EndFlush() = 0;

    // Call this function to pass a D3D device manager to the decoder from the EVR
    // Must be used in full screen exlusive mode
    virtual void SetD3DDeviceManager(IDirect3DDeviceManager9* pDeviceManager) = 0;

    // Sets the callback funtion for a DeliverSurface event. This the only method for frame delivery.
    virtual void SetDeliverSurfaceCallback(void* obj, TQS_DeliverSurfaceCallback func) = 0;

    // Fills the pConfig struct with current config.
    // If called after construction will contain the defaults.
    virtual void GetConfig(CQsConfig* pConfig) = 0;

    // Call this function to modify the decoder config.
    // Must be called before calling the InitDecoder method.
    virtual void SetConfig(CQsConfig* pConfig) = 0;

    // Change output surface type dynamically
    virtual void SetOutputSurfaceType(QsOutputSurfaceType surfaceType) = 0;

    virtual const char* GetCodecName() const = 0;
    virtual bool IsHwAccelerated() const = 0;
protected:
    // Ban copying!
    IQuickSyncDecoder& operator=(const IQuickSyncDecoder&);
    IQuickSyncDecoder(const IQuickSyncDecoder&);
    // Ban destruction of interface
    ~IQuickSyncDecoder() {}
};

// exported functions
extern "C" 
{
    IQuickSyncDecoder* __stdcall createQuickSync();
    void               __stdcall destroyQuickSync(IQuickSyncDecoder*);
    void               __stdcall getVersion(char* ver, const char** license);
    DWORD              __stdcall check();
}
