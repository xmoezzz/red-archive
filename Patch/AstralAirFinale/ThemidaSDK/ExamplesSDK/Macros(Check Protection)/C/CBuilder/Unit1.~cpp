//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include "..\..\..\..\Include\C\ThemidaSDK.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
}

//---------------------------------------------------------------------------
void __fastcall TForm1::Button4Click(TObject *Sender)
{

    int StatusProtection = 0;;

    VM_START

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

    VM_END

}

//---------------------------------------------------------------------------

