// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define PTCHAR TCHAR*

#include <tchar.h>
#include <stdio.h>
#include <process.h>
#include <stdint.h>

// Platform SDK
#include <sdkddkver.h>
#if (NTDDI_VERSION >= NTDDI_VERSION_FROM_WIN32_WINNT2(0x0602)) // >= _WIN32_WINNT_WIN8
    #define MFX_D3D11_SUPPORT 1 // Enable D3D11 support if SDK allows
    #pragma message("\nIntel QuickSync Decoder is built with D3D11 support\n")
#else
    #define MFX_D3D11_SUPPORT 0
    #pragma message("\nNote: Intel QuickSync Decoder is built without D3D11 support!")
    #pragma message("\tMust have Windows SDK 8.0 installed and environment variable INTELMEDIASDK_WINSDK_PATH")
    #pragma message("\tshould point to it.")
    #pragma message("\te.g. \"C:\\Program Files (x86)\\Windows Kits\\8.0\"")
    // disable depracated warnings from VC10's Windows SDK 7.0
    #pragma warning(disable: 4995)
#endif

#include <d3d9.h>
#include <d3d11.h>
#include <dshow.h>
#include <dvdmedia.h>
#include <initguid.h>
#include <atlbase.h>
#include <dxva2api.h>
#if MFX_D3D11_SUPPORT
    #include <dxgi1_2.h>
#endif
// Intel Media SDK
#include <mfxvideo++.h>

// STL
#include <list>
#include <vector>
#include <deque>
#include <set>
#include <algorithm>

// PPL
#include <ppl.h>

// use our own ASSERT macro
#undef ASSERT
