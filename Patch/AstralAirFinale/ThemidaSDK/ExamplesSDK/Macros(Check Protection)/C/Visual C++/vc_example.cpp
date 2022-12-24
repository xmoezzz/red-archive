/****************************************************************************** 
/* Module: Vc_example
/* Description: Shows how to call SecureEngine Macros in Visual C++ language
/*
/* (c) 2008 Oreans Technologies
/*****************************************************************************/ 


/****************************************************************************** 
/*                   Libraries used by this module
/*****************************************************************************/ 

#include "stdafx.h"
#include <stdio.h>
#include "Resource.h"
#include <commctrl.h>
#include <windows.h>
#include "ThemidaSDK.h"

/****************************************************************************** 
/*                          Function prototypes
/*****************************************************************************/ 

LRESULT CALLBACK  MainHandler(HWND, UINT, WPARAM, LPARAM);

/****************************************************************************** 
/*                          Global variables
/*****************************************************************************/ 


/*****************************************************************************
* WinMain
*
*  Main program function
*
* Inputs
*  Standard WinMain parameters
* 
* Outputs
*  None
*
* Returns
*  Exit Code
******************************************************************************/

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
    DialogBox(GetModuleHandle(NULL), (LPCTSTR)IDD_ABOUTBOX, NULL, (DLGPROC)MainHandler);

    return 0;
}


/*****************************************************************************
* MainHandler
*
*  Message handler for dialog box
*
* Inputs
*  Standard Dlgbox message handle parameters
* 
* Outputs
*  None
*
* Returns
*  Exit Code
******************************************************************************/

LRESULT CALLBACK MainHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   int StatusProtection = 0; 

    switch (message)
    {

    case WM_INITDIALOG:
        
        return TRUE;

    case WM_COMMAND:

        if (LOWORD(wParam) == IDCANCEL) 
        {
			EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }

        if (LOWORD(wParam) == IDC_CHECK_PROTECTION)
        {
      
            VM_TIGER_WHITE_START

			// Check status of protection. Return value will be stored in "StatusProtection" variable. In this example, our correct return value
			// will be "0x33333333" (of course, you can put any value here instead of 0x33333333)

			CHECK_PROTECTION(StatusProtection, 0x33333333)		
			
			// check if correct return value
			// NOTE: In this example, we are just displaying a message box if application has been tampered. You should follow the 
			// recomendations in the Help File (See CHECK_PROTECTION macro in help file)

			if (StatusProtection == 0x33333333)
			{
	            MessageBox(NULL, "Protection OK.", "Check Protection Macro", MB_OK + MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(NULL, "Application has been tampered!!!", "Alert", MB_OK + MB_ICONERROR);
			}
			
			VM_TIGER_WHITE_END
        }

	}
   
    return FALSE;
}
