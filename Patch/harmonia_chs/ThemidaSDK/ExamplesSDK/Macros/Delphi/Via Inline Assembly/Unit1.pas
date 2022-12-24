unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TForm1 = class(TForm)
    GroupBox1: TGroupBox;
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    GroupBox2: TGroupBox;
    Label1: TLabel;
    Label2: TLabel;
    Button4: TButton;
    GroupBox3: TGroupBox;
    Button5: TButton;
    Button6: TButton;
    Button7: TButton;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure Button4Click(Sender: TObject);
    procedure Button5Click(Sender: TObject);
    procedure Button6Click(Sender: TObject);
    procedure Button7Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}

procedure TForm1.Button1Click(Sender: TObject);

var
  Value, i: Integer;

begin
             
    Value := 0;

    // the following code, inside the ENCODE macro, will go encrypted
    // all the time and decrypted/encrypted again when it's executed

    {$I ..\..\..\..\Include\Delphi\Encode_Start.inc}

	  for i := 0 to  10 do
      Value := Value * 2;

    {$I ..\..\..\..\Include\Delphi\Encode_End.inc}

     MessageBox(0, 'The Encode Macro #1 has successfully been executed', 'Encode Macro', MB_OK + MB_ICONINFORMATION);

end;

procedure TForm1.Button2Click(Sender: TObject);

var
  Value, i: Integer;

begin

    Value := 0;

    // the following code, inside the ENCODE macro, will go encrypted
    // all the time and decrypted/encrypted again when it's executed

    {$I ..\..\..\..\Include\Delphi\Encode_Start.inc}

	  for i := 0 to  10 do
      Value := Value * 3;

    {$I ..\..\..\..\Include\Delphi\Encode_End.inc}

     MessageBox(0, 'The Encode Macro #2 has successfully been executed', 'Encode Macro', MB_OK + MB_ICONINFORMATION);

end;
procedure TForm1.Button3Click(Sender: TObject);

var
  Value, i: Integer;

begin

    Value := 0;

    // the following code, inside the ENCODE macro, will go encrypted
    // all the time and decrypted/encrypted again when it's executed

    {$I ..\..\..\..\Include\Delphi\Encode_Start.inc}

	  for i := 0 to  10 do
      Value := Value * 4;

    {$I ..\..\..\..\Include\Delphi\Encode_End.inc}

     MessageBox(0, 'The Encode Macro #3 has successfully been executed', 'Encode Macro', MB_OK + MB_ICONINFORMATION);

end;

procedure TForm1.Button4Click(Sender: TObject);
var
  Value, i: Integer;

begin

    Value := 0;

    // the following code, inside the CLEAR macro, will go encrypted
    // all the time and decrypted/removed  when it's executed

    {$I ..\..\..\..\Include\Delphi\Clear_Start.inc}

	  for i := 0 to  10 do
      Value := Value * 4;

    {$I ..\..\..\..\Include\Delphi\Clear_End.inc}

    MessageBox(0, 'The Clear Macro has successfully been executed', 'Clear Macro', MB_OK + MB_ICONINFORMATION);

    Label1.Visible := True;
    Label2.Visible := True;

end;

procedure TForm1.Button5Click(Sender: TObject);

var
  Value, i: Integer;

begin

    Value := 0;

    // the following code, will be virtualized

    {$I ..\..\..\..\Include\Delphi\VM_TIGER_WHITE_START.inc}

	  for i := 0 to  10 do
      Value := Value * 3;

     MessageBox(0, 'The TIGER WHITE Macro #1 has successfully been executed', 'VM Macro', MB_OK + MB_ICONINFORMATION);

    {$I ..\..\..\..\Include\Delphi\VM_TIGER_WHITE_END.inc}
     
end;

procedure TForm1.Button6Click(Sender: TObject);

var
  Value, i: Integer;

begin

    Value := 0;

    // the following code, will be virtualized

    {$I ..\..\..\..\Include\Delphi\VM_TIGER_WHITE_START.inc}

	  for i := 0 to  10 do
      Value := Value * 3 + i * 2;

     MessageBox(0, 'The TIGER WHITE Macro #2 has successfully been executed', 'VM Macro', MB_OK + MB_ICONINFORMATION);

    {$I ..\..\..\..\Include\Delphi\VM_TIGER_WHITE_END.inc}

end;

procedure TForm1.Button7Click(Sender: TObject);

var
  Value, i: Integer;

begin

    Value := 0;

    // the following code, will be virtualized

    {$I ..\..\..\..\Include\Delphi\VM_TIGER_WHITE_START.inc}

	  for i := 0 to  10 do
      Value := Value * 3 * i;

     MessageBox(0, 'The TIGER WHITE Macro #3 has successfully been executed', 'VM Macro', MB_OK + MB_ICONINFORMATION);

    {$I ..\..\..\..\Include\Delphi\VM_TIGER_WHITE_END.inc}

end;

end.
