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

// Uncomment for extra debug traces
//#define VERBOSE

// MSDK min version
#define MIN_REQUIRED_API_VER_MINOR 1
#define MIN_REQUIRED_API_VER_MAJOR 1

// VS2012 provides AVX and AVX2 intrinsics
#if 0// _MSC_VER >= 1700  // AVX2 copy is not faster than SSE2, it's slower. Copy function is left for reference only.
#   define AVX_ENABLED
#   define AVX2_ENABLED
#endif

#define MSDK_MAX_SURFACES 256

#define MSDK_ARRAY_LEN(A)                        (sizeof(A) / sizeof(A[0]))
#define MSDK_PRINT_RET_MSG(ERR) \
{\
    TCHAR msg[1024];    \
    _stprintf_s(msg, MSDK_ARRAY_LEN(msg), _T("\nReturn on error: error code %d,\t%s\t%d\n\n"), ERR, _T(__FILE__), __LINE__); \
    OutputDebugString(msg); \
    _tperror(msg);\
}
#define MSDK_SUCCEEDED(P)                        (MFX_ERR_NONE == (P))
#define MSDK_FAILED(P)                           (MFX_ERR_NONE != (P))
#define MSDK_CHECK_ERROR(P, X, ERR)              { if ((X) == (P)) { MSDK_PRINT_RET_MSG(ERR); return ERR; } }
#define MSDK_CHECK_NOT_EQUAL(P, X, ERR)          { if ((X) != (P)) { MSDK_PRINT_RET_MSG(ERR); return ERR; } }
#define MSDK_CHECK_RESULT_P_RET(P, X)            { if ((X) != (P)) { return P; } }
#define MSDK_CHECK_POINTER(P, ERR)               { if (!(P))       { return ERR;}}
#define MSDK_CHECK_POINTER_NO_RET(P)             { if (!(P))       { return; } }
#define MSDK_IGNORE_MFX_STS(P, X)                { if ((X) == (P)) { P = MFX_ERR_NONE; } }

#define MSDK_SAFE_DELETE(P)                      { delete P; P = NULL; }
#define MSDK_SAFE_DELETE_ARRAY(P)                { delete[] P; P = NULL; }
#define MSDK_SAFE_RELEASE(P)                     { if (P) { P->Release(); P = NULL; } }

#define MSDK_ZERO_MEMORY(P, S)                   { memset(P, 0, S);}
#define MSDK_ZERO_VAR(VAR)                       { memset(&VAR, 0, sizeof(VAR)); }
#define MSDK_ALIGN16(SZ)                         ((SZ + 15) & (~15)) // round up to a multiple of 16
#define MSDK_ALIGN32(SZ)                         ((SZ + 31) & (~31)) // round up to a multiple of 32
#define MSDK_ALIGN64(SZ)                         ((SZ + 63) & (~63)) // round up to a multiple of 64
#define MSDK_ALIGN128(SZ)                        ((SZ + 127) & (~127)) // round up to a multiple of 128

#ifdef _DEBUG
#   define ASSERT(_x_)  { if (!(_x_)) DebugAssert(_T(#_x_),_T(__FILE__),__LINE__); }
#else
#   define ASSERT(x)
#endif

#ifdef _DEBUG
#   define MSDK_TRACE(_format, ...)                                                        \
{                                                                                          \
    char msg[256];                                                                         \
    _snprintf_s(msg, MSDK_ARRAY_LEN(msg), MSDK_ARRAY_LEN(msg) - 1, _format, __VA_ARGS__);  \
    OutputDebugStringA(msg);                                                               \
}
// MSDK_VTRACE should be used for very frequent prints
#   ifdef VERBOSE
#       define MSDK_VTRACE MSDK_TRACE
#   else
#       define MSDK_VTRACE
#   endif
#else
#   define MSDK_TRACE(_format, ...)
#   define MSDK_VTRACE(_format, ...)
#endif

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);             \
    void operator=(const TypeName&);   
