unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TForm1 = class(TForm)
    GroupBox2: TGroupBox;
    Button4: TButton;
    procedure Button4Click(Sender: TObject);

  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}

procedure TForm1.Button4Click(Sender: TObject);
var
  StatusProtection: Integer;

begin
         
    {$I ..\..\..\..\Include\Delphi\VM_TIGER_WHITE_START.inc}

    // Check status of protection. Return value will be stored in "StatusProtection" variable. In this example, our correct return value
    // will be "0x33333333" (of course, you can put any value here instead of 0x33333333)

    {$I ..\..\..\..\Include\Delphi\CheckProtection_Prolog.inc}
    asm
      push $33333333
      pop  StatusProtection
    end;
    {$I ..\..\..\..\Include\Delphi\CheckProtection_Epilog.inc}

    // check if correct return value
    // NOTE: In this example, we are just displaying a message box if application has been tampered. You should follow the
    // recomendations in the Help File (See CHECK_PROTECTION macro in help file)

    if StatusProtection = $33333333 then
      MessageBox(0, 'Protection OK.', 'Check Protection Macro', MB_OK + MB_ICONINFORMATION)
    else
      MessageBox(0, 'Application has been tampered!!!', 'Alert', MB_OK + MB_ICONERROR);

    {$I ..\..\..\..\Include\Delphi\VM_TIGER_WHITE_END.inc}

end;

end.
