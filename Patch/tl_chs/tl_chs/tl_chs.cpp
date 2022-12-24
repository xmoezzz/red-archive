#include "my.h"
#include "KaresekaHook.h"
#include "Se.h"
#include <shellscalingapi.h>
#include <VersionHelpers.h>

#pragma comment(lib, "MyLibrary_x86_static.lib")
#pragma comment(lib, "Shcore.lib")


typedef struct _KSYSTEM_TIME                                                    // 3 / 3 elements; 0x000C / 0x000C Bytes
{
	ULONG32                     LowPart;                                        // 0x0000 / 0x0000; 0x0004 / 0x0004 Bytes
	LONG32                      High1Time;                                      // 0x0004 / 0x0004; 0x0004 / 0x0004 Bytes
	LONG32                      High2Time;                                      // 0x0008 / 0x0008; 0x0004 / 0x0004 Bytes
} KSYSTEM_TIME, *PKSYSTEM_TIME;


typedef enum _NT_PRODUCT_TYPE                                                   // 3 elements; 0x0004 Bytes
{
	NtProductWinNt = 1,
	NtProductLanManNt = 2,
	NtProductServer = 3
} NT_PRODUCT_TYPE, *PNT_PRODUCT_TYPE;

typedef enum _ALTERNATIVE_ARCHITECTURE_TYPE                                     // 3 elements; 0x0004 Bytes
{
	StandardDesign = 0,
	NEC98x86 = 1,
	EndAlternatives = 2
} ALTERNATIVE_ARCHITECTURE_TYPE, *PALTERNATIVE_ARCHITECTURE_TYPE;

typedef struct _KUSER_SHARED_DATA                                               // 95 / 95 elements; 0x0708 / 0x0708 Bytes
{
	ULONG32                     TickCountLowDeprecated;                         // 0x0000 / 0x0000; 0x0004 / 0x0004 Bytes
	ULONG32                     TickCountMultiplier;                            // 0x0004 / 0x0004; 0x0004 / 0x0004 Bytes
	KSYSTEM_TIME                InterruptTime;                                  // 0x0008 / 0x0008; 0x000C / 0x000C Bytes
	KSYSTEM_TIME                SystemTime;                                     // 0x0014 / 0x0014; 0x000C / 0x000C Bytes
	KSYSTEM_TIME                TimeZoneBias;                                   // 0x0020 / 0x0020; 0x000C / 0x000C Bytes
	UINT16                      ImageNumberLow;                                 // 0x002C / 0x002C; 0x0002 / 0x0002 Bytes
	UINT16                      ImageNumberHigh;                                // 0x002E / 0x002E; 0x0002 / 0x0002 Bytes
	WCHAR                       NtSystemRoot[260];                              // 0x0030 / 0x0030; 0x0208 / 0x0208 Bytes
	ULONG32                     MaxStackTraceDepth;                             // 0x0238 / 0x0238; 0x0004 / 0x0004 Bytes
	ULONG32                     CryptoExponent;                                 // 0x023C / 0x023C; 0x0004 / 0x0004 Bytes
	ULONG32                     TimeZoneId;                                     // 0x0240 / 0x0240; 0x0004 / 0x0004 Bytes
	ULONG32                     LargePageMinimum;                               // 0x0244 / 0x0244; 0x0004 / 0x0004 Bytes
	ULONG32                     AitSamplingValue;                               // 0x0248 / 0x0248; 0x0004 / 0x0004 Bytes
	ULONG32                     AppCompatFlag;                                  // 0x024C / 0x024C; 0x0004 / 0x0004 Bytes
	UINT64                      RNGSeedVersion;                                 // 0x0250 / 0x0250; 0x0008 / 0x0008 Bytes
	ULONG32                     GlobalValidationRunlevel;                       // 0x0258 / 0x0258; 0x0004 / 0x0004 Bytes
	LONG32                      TimeZoneBiasStamp;                              // 0x025C / 0x025C; 0x0004 / 0x0004 Bytes
	ULONG32                     NtBuildNumber;                                  // 0x0260 / 0x0260; 0x0004 / 0x0004 Bytes
	NT_PRODUCT_TYPE             NtProductType;                                  // 0x0264 / 0x0264; 0x0004 / 0x0004 Bytes
	UINT8                       ProductTypeIsValid;                             // 0x0268 / 0x0268; 0x0001 / 0x0001 Bytes
	UINT8                       Reserved0[1];                                   // 0x0269 / 0x0269; 0x0001 / 0x0001 Bytes
	UINT16                      NativeProcessorArchitecture;                    // 0x026A / 0x026A; 0x0002 / 0x0002 Bytes
	ULONG32                     NtMajorVersion;                                 // 0x026C / 0x026C; 0x0004 / 0x0004 Bytes
	ULONG32                     NtMinorVersion;                                 // 0x0270 / 0x0270; 0x0004 / 0x0004 Bytes
	UINT8                       ProcessorFeatures[64];                          // 0x0274 / 0x0274; 0x0040 / 0x0040 Bytes
	ULONG32                     Reserved1;                                      // 0x02B4 / 0x02B4; 0x0004 / 0x0004 Bytes
	ULONG32                     Reserved3;                                      // 0x02B8 / 0x02B8; 0x0004 / 0x0004 Bytes
	ULONG32                     TimeSlip;                                       // 0x02BC / 0x02BC; 0x0004 / 0x0004 Bytes
	ALTERNATIVE_ARCHITECTURE_TYPE AlternativeArchitecture;                        // 0x02C0 / 0x02C0; 0x0004 / 0x0004 Bytes
	ULONG32                     BootId;                                         // 0x02C4 / 0x02C4; 0x0004 / 0x0004 Bytes
	LARGE_INTEGER               SystemExpirationDate;                           // 0x02C8 / 0x02C8; 0x0008 / 0x0008 Bytes
	ULONG32                     SuiteMask;                                      // 0x02D0 / 0x02D0; 0x0004 / 0x0004 Bytes
	UINT8                       KdDebuggerEnabled;                              // 0x02D4 / 0x02D4; 0x0001 / 0x0001 Bytes
	union                                                                       // 2 / 2 elements; 0x0001 / 0x0001 Bytes
	{
		UINT8                   MitigationPolicies;                             // 0x02D5 / 0x02D5; 0x0001 / 0x0001 Bytes
		struct                                                                  // 4 / 4 elements; 0x0001 / 0x0001 Bytes
		{
			UINT8               NXSupportPolicy : 2; // 0x02D5 / 0x02D5; Bits:  0 -  1
			UINT8               SEHValidationPolicy : 2; // 0x02D5 / 0x02D5; Bits:  2 -  3
			UINT8               CurDirDevicesSkippedForDlls : 2; // 0x02D5 / 0x02D5; Bits:  4 -  5
			UINT8               Reserved : 2; // 0x02D5 / 0x02D5; Bits:  6 -  7
		};
	};
	UINT8                       Reserved6[2];                                   // 0x02D6 / 0x02D6; 0x0002 / 0x0002 Bytes
	ULONG32                     ActiveConsoleId;                                // 0x02D8 / 0x02D8; 0x0004 / 0x0004 Bytes
	ULONG32                     DismountCount;                                  // 0x02DC / 0x02DC; 0x0004 / 0x0004 Bytes
	ULONG32                     ComPlusPackage;                                 // 0x02E0 / 0x02E0; 0x0004 / 0x0004 Bytes
	ULONG32                     LastSystemRITEventTickCount;                    // 0x02E4 / 0x02E4; 0x0004 / 0x0004 Bytes
	ULONG32                     NumberOfPhysicalPages;                          // 0x02E8 / 0x02E8; 0x0004 / 0x0004 Bytes
	UINT8                       SafeBootMode;                                   // 0x02EC / 0x02EC; 0x0001 / 0x0001 Bytes
	UINT8                       Reserved12[3];                                  // 0x02ED / 0x02ED; 0x0003 / 0x0003 Bytes
	union                                                                       // 2 / 2 elements; 0x0004 / 0x0004 Bytes
	{
		ULONG32                 SharedDataFlags;                                // 0x02F0 / 0x02F0; 0x0004 / 0x0004 Bytes
		struct                                                                  // 10 / 10 elements; 0x0004 / 0x0004 Bytes
		{
			ULONG32             DbgErrorPortPresent : 1; // 0x02F0 / 0x02F0; Bit:   0
			ULONG32             DbgElevationEnabled : 1; // 0x02F0 / 0x02F0; Bit:   1
			ULONG32             DbgVirtEnabled : 1; // 0x02F0 / 0x02F0; Bit:   2
			ULONG32             DbgInstallerDetectEnabled : 1; // 0x02F0 / 0x02F0; Bit:   3
			ULONG32             DbgLkgEnabled : 1; // 0x02F0 / 0x02F0; Bit:   4
			ULONG32             DbgDynProcessorEnabled : 1; // 0x02F0 / 0x02F0; Bit:   5
			ULONG32             DbgConsoleBrokerEnabled : 1; // 0x02F0 / 0x02F0; Bit:   6
			ULONG32             DbgSecureBootEnabled : 1; // 0x02F0 / 0x02F0; Bit:   7
			ULONG32             DbgMultiSessionSku : 1; // 0x02F0 / 0x02F0; Bit:   8
			ULONG32             SpareBits : 23; // 0x02F0 / 0x02F0; Bits:  9 - 31
		};
	};
	ULONG32                     DataFlagsPad[1];                                // 0x02F4 / 0x02F4; 0x0004 / 0x0004 Bytes
	UINT64                      TestRetInstruction;                             // 0x02F8 / 0x02F8; 0x0008 / 0x0008 Bytes
	INT64                       QpcFrequency;                                   // 0x0300 / 0x0300; 0x0008 / 0x0008 Bytes
	ULONG32                     SystemCall;                                     // 0x0308 / 0x0308; 0x0004 / 0x0004 Bytes
	ULONG32                     SystemCallPad0;                                 // 0x030C / 0x030C; 0x0004 / 0x0004 Bytes
	UINT64                      SystemCallPad[2];                               // 0x0310 / 0x0310; 0x0010 / 0x0010 Bytes
	union                                                                       // 3 / 3 elements; 0x000C / 0x000C Bytes
	{
		KSYSTEM_TIME            TickCount;                                      // 0x0320 / 0x0320; 0x000C / 0x000C Bytes
		UINT64                  TickCountQuad;                                  // 0x0320 / 0x0320; 0x0008 / 0x0008 Bytes
		ULONG32                 ReservedTickCountOverlay[3];                    // 0x0320 / 0x0320; 0x000C / 0x000C Bytes
	};
	ULONG32                     TickCountPad[1];                                // 0x032C / 0x032C; 0x0004 / 0x0004 Bytes
	ULONG32                     Cookie;                                         // 0x0330 / 0x0330; 0x0004 / 0x0004 Bytes
	ULONG32                     CookiePad[1];                                   // 0x0334 / 0x0334; 0x0004 / 0x0004 Bytes
	INT64                       ConsoleSessionForegroundProcessId;              // 0x0338 / 0x0338; 0x0008 / 0x0008 Bytes
	UINT64                      TimeUpdateLock;                                 // 0x0340 / 0x0340; 0x0008 / 0x0008 Bytes
	UINT64                      BaselineSystemTimeQpc;                          // 0x0348 / 0x0348; 0x0008 / 0x0008 Bytes
	UINT64                      BaselineInterruptTimeQpc;                       // 0x0350 / 0x0350; 0x0008 / 0x0008 Bytes
	UINT64                      QpcSystemTimeIncrement;                         // 0x0358 / 0x0358; 0x0008 / 0x0008 Bytes
	UINT64                      QpcInterruptTimeIncrement;                      // 0x0360 / 0x0360; 0x0008 / 0x0008 Bytes
	UINT8                       QpcSystemTimeIncrementShift;                    // 0x0368 / 0x0368; 0x0001 / 0x0001 Bytes
	UINT8                       QpcInterruptTimeIncrementShift;                 // 0x0369 / 0x0369; 0x0001 / 0x0001 Bytes
	UINT16                      UnparkedProcessorCount;                         // 0x036A / 0x036A; 0x0002 / 0x0002 Bytes
	ULONG32                     EnclaveFeatureMask[4];                          // 0x036C / 0x036C; 0x0010 / 0x0010 Bytes
	ULONG32                     Reserved8;                                      // 0x037C / 0x037C; 0x0004 / 0x0004 Bytes
	UINT16                      UserModeGlobalLogger[16];                       // 0x0380 / 0x0380; 0x0020 / 0x0020 Bytes
	ULONG32                     ImageFileExecutionOptions;                      // 0x03A0 / 0x03A0; 0x0004 / 0x0004 Bytes
	ULONG32                     LangGenerationCount;                            // 0x03A4 / 0x03A4; 0x0004 / 0x0004 Bytes
	UINT64                      Reserved4;                                      // 0x03A8 / 0x03A8; 0x0008 / 0x0008 Bytes
	UINT64                      InterruptTimeBias;                              // 0x03B0 / 0x03B0; 0x0008 / 0x0008 Bytes
	UINT64                      QpcBias;                                        // 0x03B8 / 0x03B8; 0x0008 / 0x0008 Bytes
	ULONG32                     ActiveProcessorCount;                           // 0x03C0 / 0x03C0; 0x0004 / 0x0004 Bytes
	UINT8                       ActiveGroupCount;                               // 0x03C4 / 0x03C4; 0x0001 / 0x0001 Bytes
	UINT8                       Reserved9;                                      // 0x03C5 / 0x03C5; 0x0001 / 0x0001 Bytes
	union                                                                       // 2 / 2 elements; 0x0002 / 0x0002 Bytes
	{
		UINT16                  QpcData;                                        // 0x03C6 / 0x03C6; 0x0002 / 0x0002 Bytes
		struct                                                                  // 2 / 2 elements; 0x0002 / 0x0002 Bytes
		{
			UINT8               QpcBypassEnabled;                               // 0x03C6 / 0x03C6; 0x0001 / 0x0001 Bytes
			UINT8               QpcShift;                                       // 0x03C7 / 0x03C7; 0x0001 / 0x0001 Bytes
		};
	};
	LARGE_INTEGER               TimeZoneBiasEffectiveStart;                     // 0x03C8 / 0x03C8; 0x0008 / 0x0008 Bytes
	LARGE_INTEGER               TimeZoneBiasEffectiveEnd;                       // 0x03D0 / 0x03D0; 0x0008 / 0x0008 Bytes
	XSTATE_CONFIGURATION        XState;                                         // 0x03D8 / 0x03D8; 0x0330 / 0x0330 Bytes
} KUSER_SHARED_DATA, *PKUSER_SHARED_DATA;

DWORD NTAPI Is64BitOSThread(VOID*)
{
	HRESULT            hr;
	BOOL               Current;
	KUSER_SHARED_DATA* KernelShare;
	
	Current = TRUE;
	if (!IsWow64Process())
	{
		Current = FALSE;
		MessageBoxW(NULL, L"64bit os only", L"Error", MB_OK | MB_ICONERROR);
		Ps::ExitProcess(-1);
	}

	LOOP_FOREVER
	{
		KernelShare = (KUSER_SHARED_DATA*)0x7FFE0000;
		if (KernelShare->KdDebuggerEnabled)
		{
			Ps::ExitProcess(-1);
		}
		Ps::Sleep(10);
	};
	return 0;
}

void InitGodMode()
{
	
}


enum Version {
	VERSION_PRE_XP = 0,  // Not supported.
	VERSION_XP = 1,
	VERSION_SERVER_2003 = 2,  // Also includes XP Pro x64 and Server 2003 R2.
	VERSION_VISTA = 3,        // Also includes Windows Server 2008.
	VERSION_WIN7 = 4,         // Also includes Windows Server 2008 R2.
	VERSION_WIN8 = 5,         // Also includes Windows Server 2012.
	VERSION_WIN8_1 = 6,       // Also includes Windows Server 2012 R2.
	VERSION_WIN10 = 7,        // Threshold 1: Version 1507, Build 10240.
	VERSION_WIN10_TH2 = 8,    // Threshold 2: Version 1511, Build 10586.
	VERSION_WIN10_RS1 = 9,    // Redstone 1: Version 1607, Build 14393.
	VERSION_WIN10_RS2 = 10,   // Redstone 2: Version 1703, Build 15063.
	VERSION_WIN10_RS3 = 11,   // Redstone 3: Version 1709, Build 16299.
	VERSION_WIN10_RS4 = 12,   // Redstone 4: Version 1803, Build 17134.
	// On edit, update tools\metrics\histograms\enums.xml "WindowsVersion" and
	// "GpuBlacklistFeatureTestResultsWindows2".
	VERSION_WIN_LAST,  // Indicates error condition.
};



using DPI_AWARENESS_CONTEXT = UINT;

#define DPI_AWARENESS_CONTEXT_UNAWARE              ((DPI_AWARENESS_CONTEXT)-1)
#define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE         ((DPI_AWARENESS_CONTEXT)-2)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    ((DPI_AWARENESS_CONTEXT)-3)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#define DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED    ((DPI_AWARENESS_CONTEXT)-5)

BOOL NTAPI SetProcessDpiAwarenessContext(
	DPI_AWARENESS_CONTEXT value
	);

API_POINTER(SetProcessDpiAwarenessContext) StubSetProcessDpiAwarenessContext = NULL;
API_POINTER(SetProcessDpiAwareness) StubSetProcessDpiAwareness = NULL;

BOOL FASTCALL KaresekaInit(PVOID hModule)
{
	KaresekaHook*           Kareseka;
	THREAD_START_PARAMETER* StartParameter;

	ml::MlInitialize();
	//AllocConsole();
	Kareseka = GetKareseka();

	if (Nt_CurrentPeb()->OSMajorVersion == 10 && Nt_CurrentPeb()->OSMinorVersion == 0 && Nt_CurrentPeb()->OSBuildNumber >= 16299)
	{
		StubSetProcessDpiAwarenessContext = (API_POINTER(SetProcessDpiAwarenessContext))
			GetProcAddress(GetModuleHandleA("User32.dll"), "SetProcessDpiAwarenessContext");
		if (StubSetProcessDpiAwarenessContext)
			StubSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED);
	}
	if (IsWindows8Point1OrGreater())
	{
		StubSetProcessDpiAwareness = (API_POINTER(SetProcessDpiAwareness))
			GetProcAddress(GetModuleHandleA("Shcore.dll"), "SetProcessDpiAwareness");
		StubSetProcessDpiAwareness(PROCESS_DPI_UNAWARE);
	}

	SetMainThread(Nt_CurrentTeb()->ClientId.UniqueThread);
	StartParameter = AllocateThreadParameter(NULL, (PVOID)((ULONG)NtAddAtom ^ (ULONG)Nt_CurrentPeb()));
	RtlPushFrame(StartParameter);

	Nt_CreateThread(Is64BitOSThread);
	return NT_SUCCESS(Kareseka->Init((HMODULE)hModule));
}


BOOL FASTCALL KaresekaUnInit(PVOID hModule)
{
	UNREFERENCED_PARAMETER(hModule);
	return TRUE;
}


OVERLOAD_CPP_NEW_WITH_HEAP(Nt_CurrentPeb()->ProcessHeap)

BOOL NTAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		return KaresekaInit(hModule);

	case DLL_PROCESS_DETACH:
		return KaresekaUnInit(hModule);
	}
	return TRUE;
}
