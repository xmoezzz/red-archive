// DLL Control.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

extern "C" __declspec(dllexport) 
bool __stdcall MsgHandler( int MsgId,
						   char* MsgBody)
{
	if (MsgId == -1) // This is the Initialization MsgID 
	{
		MessageBoxA(NULL, "Protection Initialization", "Message from DLL", MB_OK | MB_ICONINFORMATION);
	}
	else if (MsgId == -2) // This is the Finalization MsgID 
	{
		MessageBoxA(NULL, "Application is about to start", "Message from DLL", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBoxA(NULL, MsgBody, "Message from DLL", MB_OK | MB_ICONINFORMATION);
	}

	// When we return "true", we tell WinLicense that we have processed the message, so, WinLicense will not display
	// the custom message 

	return true;
}
 


