//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include "ThemidaSDK.h"

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

void __fastcall TForm1::Button1Click(TObject *Sender)
{
     int        value = 0;

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
//---------------------------------------------------------------------------

void __fastcall TForm1::Button2Click(TObject *Sender)
{
    int        value = 0;

     // the following code, inside the ENCODE macro, will go encrypted
     // all the time and decrypted/encrypted again when it's executed

     ENCODE_START

     for (int i = 0; i < 100; i++)
     {
         value += value + i * 2;
     }

     ENCODE_END

     MessageBox(NULL, "The Encode Macro #2 has successfully been executed", "Encode Macro", MB_OK + MB_ICONINFORMATION);

}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button3Click(TObject *Sender)
{
     int        value = 0;

     // the following code, inside the ENCODE macro, will go encrypted
     // all the time and decrypted/encrypted again when it's executed

     ENCODE_START

     for (int i = 0; i < 100; i++)
     {
         value += value * i * 2;
     }

     ENCODE_END

     MessageBox(NULL, "The Encode Macro #3 has successfully been executed", "Encode Macro", MB_OK + MB_ICONINFORMATION);

}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button4Click(TObject *Sender)
{
    int        value = 0;

     // the following code, inside the CLEAR macro, will go encrypted
     // all the time and decrypted/deleted when it's executed

     CLEAR_START

     for (int i = 0; i < 100; i++)
     {
         value += value * i >> 2;
     }

     CLEAR_END

     MessageBox(NULL, "The Clear Macro has successfully been executed", "Clear Macro", MB_OK + MB_ICONINFORMATION);

     Label1->Visible = true;
     Label2->Visible = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button5Click(TObject *Sender)
{
    int        value = 0;

     // the following code will be virtualized

     VM_TIGER_WHITE_START

     for (int i = 0; i < 100; i++)
     {
         value += value * i * 2;
     }

     MessageBox(NULL, "The Tiger White Macro #1 has successfully been executed", "CodeReplace Macro", MB_OK + MB_ICONINFORMATION);

     VM_TIGER_WHITE_END
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button6Click(TObject *Sender)
{
   int        value = 0;

     // the following code will be virtualized

     VM_TIGER_WHITE_START

     for (int i = 0; i < 100; i++)
     {
         value += value * i;
     }

     MessageBox(NULL, "The Tiger White Macro #2 has successfully been executed", "CodeReplace Macro", MB_OK + MB_ICONINFORMATION);

     VM_TIGER_WHITE_END
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button7Click(TObject *Sender)
{
   int        value = 0;

     // the following code will be virtualized

     VM_TIGER_WHITE_START

     for (int i = 0; i < 100; i++)
     {
         value += value * i * 2;
     }

      MessageBox(NULL, "The Tiger White Macro #3 has successfully been executed", "CodeReplace Macro", MB_OK + MB_ICONINFORMATION);

     VM_TIGER_WHITE_END
}
//---------------------------------------------------------------------------
