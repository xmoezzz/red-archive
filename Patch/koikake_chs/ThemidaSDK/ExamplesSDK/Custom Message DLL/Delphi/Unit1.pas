unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ComCtrls;

type
  TForm1 = class(TForm)
    Button1: TButton;
    labelStatus: TLabel;
    lbMessage: TLabel;
    procedure Button1Click(Sender: TObject);

  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

function CustMsg_ShowMessage(MsgId: Integer; pMsg:PChar):Boolean; stdcall;

exports
  CustMsg_ShowMessage;

implementation

{$R *.dfm}


function CustMsg_ShowMessage(MsgId: Integer; pMsg:PChar):Boolean;
begin

  Form1 := TForm1.Create(application);
  Form1.LabelStatus.Caption := 'Calling Message: ' + IntToStr(MsgId);

  if pMsg <> nil then
    Form1.lbMessage.Caption := 'Original Message: ' + pMsg;

  Form1.labelStatus.Width := Form1.Width;
  Form1.ShowModal;

  Result := True;                

end;


procedure TForm1.Button1Click(Sender: TObject);
begin

  Close;

end;

end.
