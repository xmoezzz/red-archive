/****************************************************************************** 
/* Module: Vc_example
/* Description: Shows how to call SecureEngine Macros in Visual C++ language
/*
/* Authors: Rafael Ahucha  
/* (c) 2011 Oreans Technologies
/*****************************************************************************/ 


/****************************************************************************** 
/*                   Libraries used by this module
/*****************************************************************************/ 

#include "stdafx.h"
#include <stdio.h>
#include "Resource.h"
#include <commctrl.h>
#include <windows.h>
#include "..\..\..\..\Include\C\Via ASM Module\SecureEngineMacros.h"


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


#pragma optimize("", off)     // disable optimizations to avoid that END macros are removed from compiler

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
   int value = 0; 

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

        if (LOWORD(wParam) == IDC_BUTTON_ENCODE1)
        {
 
            // the following code, inside the ENCODE macro, will go encrypted
            // all the time and decrypted/encrypted again when it's executed

            ENCODE_START

            for (int i = 0; i < 100; i++)
            {
                value += value * i;
            }

            ENCODE_END

            MessageBox(NULL, "The Encode Macro #1 has successfully been executed", "Encode Macro", MB_OK + MB_ICONINFORMATION);

		}
        else if (LOWORD(wParam) == IDC_BUTTON_ENCODE2)
        {
 
            // the following code, inside the ENCODE macro, will go encrypted
            // all the time and decrypted/encrypted again when it's executed
      
            ENCODE_START

            for (int i = 0; i < 100; i++)
            {
                value += value * i * 3;
            }

            ENCODE_END

            MessageBox(NULL, "The Encode Macro #2 has successfully been executed", "Encode Macro", MB_OK + MB_ICONINFORMATION);

		}
        else if (LOWORD(wParam) == IDC_BUTTON_ENCODE3)
        {
 
            // the following code, inside the ENCODE macro, will go encrypted
            // all the time and decrypted/encrypted again when it's executed
      
            ENCODE_START

            for (int i = 0; i < 100; i++)
            {
                value += (value * i * 4) << 1;
            }

            ENCODE_END

            MessageBox(NULL, "The Encode Macro #3 has successfully been executed", "Encode Macro", MB_OK + MB_ICONINFORMATION);

		}
        else if (LOWORD(wParam) == IDC_CLEAR)
        {
 
            // the following code, inside the CLEAR macro, will go encrypted
            // all the time and decrypted/deleted when it's executed
      
            CLEAR_START

            for (int i = 0; i < 100; i++)
            {
                value += (value * i * 4) << 2;
            }
			
            MessageBox(NULL, "The Clear Macro has successfully been executed", "Encode Macro", MB_OK + MB_ICONINFORMATION);

            MessageBox(NULL, "Note: The code inside the Clear macro has been DELETED.\nIf you run the macro again you might get an exception", "Warning", MB_OK + MB_ICONWARNING);

			CLEAR_END
        }
        else if (LOWORD(wParam) == IDC_REMOVED1)
        {
            // the following code is virtualized
      
            VM_TIGER_WHITE_START

            for (int i = 0; i < 100; i++)
            {
                value += (value * i * 4) << 1;
            }

            MessageBox(NULL, "The Tiger White Macro #1 has successfully been executed", "VM Macro", MB_OK + MB_ICONINFORMATION);

            VM_TIGER_WHITE_END
		}
        else if (LOWORD(wParam) == IDC_REMOVED2)
        {
            // the following code is virtualized
      
            VM_TIGER_WHITE_START

            for (int i = 0; i < 100; i++)
            {
                value += (value * i) >> 2;
            }
            
            MessageBox(NULL, "The Tiger White Macro #2 has successfully been executed", "VM Macro", MB_OK + MB_ICONINFORMATION);

            VM_TIGER_WHITE_END
		}
        else if (LOWORD(wParam) == IDC_REMOVED3)
        {
            // the following code is virtualized
      
            VM_TIGER_WHITE_START

            for (int i = 0; i < 100; i++)
            {
                value += (value * i / 4) >> 2;
            }
           
            MessageBox(NULL, "The Tiger White Macro #3 has successfully been executed", "VM Macro", MB_OK + MB_ICONINFORMATION);

            VM_TIGER_WHITE_END
		}

	}
   
    return FALSE;
}

#pragma optimize("", on)   // enable optimizations again
