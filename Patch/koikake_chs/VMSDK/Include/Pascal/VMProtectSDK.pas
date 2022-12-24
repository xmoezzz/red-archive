unit VMProtectSDK;

{$IFDEF DARWIN}   // Lazarus in MAC OS X
 {$DEFINE MACOSX}
{$ENDIF}

{$IFDEF MACOS}   // Windows XE cross compile for MAC
 {$DEFINE MACOSX}
{$ENDIF}

interface

// protection
  procedure VMProtectBegin(MarkerName: PAnsiChar); {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  procedure VMProtectBeginVirtualization(MarkerName: PAnsiChar); {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  procedure VMProtectBeginMutation(MarkerName: PAnsiChar); {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  procedure VMProtectBeginUltra(MarkerName: PAnsiChar); {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  procedure VMProtectBeginVirtualizationLockByKey(MarkerName: PAnsiChar); {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  procedure VMProtectBeginUltraLockByKey(MarkerName: PAnsiChar); {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  procedure VMProtectEnd; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};

// utils
  function VMProtectIsProtected: Boolean; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  function VMProtectIsDebuggerPresent(CheckKernelMode: Boolean): Boolean; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  function VMProtectIsVirtualMachinePresent: Boolean; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  function VMProtectIsValidImageCRC: Boolean; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  function VMProtectDecryptStringA(Value: PAnsiChar): PAnsiChar; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  function VMProtectDecryptStringW(Value: PWideChar): PWideChar; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  function VMProtectFreeString(Value: Pointer): Boolean; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};

// licensing
type
  TVMProtectDate = packed record
   wYear: Word;
   bMonth: Byte;
   bDay: Byte;
  end;

  PVMProtectSerialNumberData = ^TVMProtectSerialNumberData;
  TVMProtectSerialNumberData = packed record
   nState: Longword;
   wUserName: array [0..255] of WideChar;
   wEMail: array [0..255] of WideChar;
   dtExpire: TVMProtectDate;
   dtMaxBuild: TVMProtectDate;
   bRunningTime: Longword;
   nUserDataLength: Byte;
   bUserData: array [0..254] of Byte;
  end;

const
  SERIAL_STATE_SUCCESS					= 0;
  SERIAL_STATE_FLAG_CORRUPTED			= $00000001;
  SERIAL_STATE_FLAG_INVALID				= $00000002;
  SERIAL_STATE_FLAG_BLACKLISTED			= $00000004;
  SERIAL_STATE_FLAG_DATE_EXPIRED		= $00000008;
  SERIAL_STATE_FLAG_RUNNING_TIME_OVER	= $00000010;
  SERIAL_STATE_FLAG_BAD_HWID			= $00000020;
  SERIAL_STATE_FLAG_MAX_BUILD_EXPIRED	= $00000040;

  function VMProtectSetSerialNumber(SerialNumber: PAnsiChar): Longword; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  function VMProtectGetSerialNumberState: Longword; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  function VMProtectGetSerialNumberData(Data: PVMProtectSerialNumberData; DataSize: Integer): Boolean; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  function VMProtectGetCurrentHWID(Buffer: PAnsiChar; BufferLen: Integer): Integer; {$IFDEF MACOSX} cdecl; {$ELSE} {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; {$ENDIF}

// activation
const
  ACTIVATION_OK             = 0;
  ACTIVATION_SMALL_BUFFER   = 1;
  ACTIVATION_NO_CONNECTION  = 2;
  ACTIVATION_BAD_REPLY      = 3;
  ACTIVATION_BANNED         = 4;
  ACTIVATION_CORRUPTED      = 5;
  ACTIVATION_BAD_CODE       = 6;
  ACTIVATION_ALREADY_USED   = 7;
  ACTIVATION_SERIAL_UNKNOWN = 8;
  ACTIVATION_EXPIRED        = 9;
  ACTIVATION_NOT_AVAILABLE  = 10;

  function VMProtectActivateLicense(ActivationCode: PAnsiChar; Buffer: PAnsiChar; BufferLen: Integer): Integer; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  function VMProtectDeactivateLicense(SerialNumber: PAnsiChar): Integer; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  function VMProtectGetOfflineActivationString(ActivationCode: PAnsiChar; Buffer: PAnsiChar; BufferLen: Integer): Integer; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};
  function VMProtectGetOfflineDeactivationString(SerialNumber: PAnsiChar; Buffer: PAnsiChar; BufferLen: Integer): Integer; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF};

implementation

{$IFDEF MACOSX}
  {$IFDEF DARWIN}{$LINKLIB libVMProtectSDK.dylib}{$ENDIF}
{$ELSE}
{$IFDEF WIN64}
  const VMProtectDLLName = 'VMProtectSDK64.dll';
{$ELSE}
{$IFDEF WIN32}
  const VMProtectDLLName = 'VMProtectSDK32.dll';
{$ELSE}
  {$FATAL Unsupported OS!!!}
{$ENDIF}
{$ENDIF}
{$ENDIF}

procedure VMProtectBegin(MarkerName: PAnsiChar); {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectBegin'{$ENDIF};
procedure VMProtectBeginVirtualization(MarkerName: PAnsiChar); {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectBeginVirtualization'{$ENDIF};
procedure VMProtectBeginMutation(MarkerName: PAnsiChar); {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectBeginMutation'{$ENDIF};
procedure VMProtectBeginUltra(MarkerName: PAnsiChar); {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectBeginUltra'{$ENDIF};
procedure VMProtectBeginVirtualizationLockByKey(MarkerName: PAnsiChar); {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectBeginVirtualizationLockByKey'{$ENDIF};
procedure VMProtectBeginUltraLockByKey(MarkerName: PAnsiChar); {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectBeginUltraLockByKey'{$ENDIF};
procedure VMProtectEnd; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectEnd'{$ENDIF};
function VMProtectIsProtected: Boolean; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectIsProtected'{$ENDIF};
function VMProtectIsDebuggerPresent(CheckKernelMode: Boolean): Boolean; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectIsDebuggerPresent'{$ENDIF};
function VMProtectIsVirtualMachinePresent: Boolean; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectIsVirtualMachinePresent'{$ENDIF};
function VMProtectIsValidImageCRC: Boolean; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectIsValidImageCRC'{$ENDIF};
function VMProtectDecryptStringA(Value: PAnsiChar): PAnsiChar; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectDecryptStringA'{$ENDIF};
function VMProtectDecryptStringW(Value: PWideChar): PWideChar; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectDecryptStringW'{$ENDIF};
function VMProtectFreeString(Value: Pointer): Boolean; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectFreeString'{$ENDIF};
function VMProtectSetSerialNumber(SerialNumber: PAnsiChar): Longword; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectSetSerialNumber'{$ENDIF};
function VMProtectGetSerialNumberState: Longword; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectGetSerialNumberState'{$ENDIF};
function VMProtectGetSerialNumberData(Data: PVMProtectSerialNumberData; DataSize: Integer): Boolean; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectGetSerialNumberData'{$ENDIF};
function VMProtectGetCurrentHWID(Buffer: PAnsiChar; BufferLen: Integer): Integer; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectGetCurrentHWID'{$ENDIF};
function VMProtectActivateLicense(ActivationCode: PAnsiChar; Buffer: PAnsiChar; BufferLen: Integer): Integer; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectActivateLicense'{$ENDIF};
function VMProtectDeactivateLicense(SerialNumber: PAnsiChar): Integer; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectDeactivateLicense'{$ENDIF};
function VMProtectGetOfflineActivationString(ActivationCode: PAnsiChar; Buffer: PAnsiChar; BufferLen: Integer): Integer; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectGetOfflineActivationString'{$ENDIF};
function VMProtectGetOfflineDeactivationString(SerialNumber: PAnsiChar; Buffer: PAnsiChar; BufferLen: Integer): Integer; {$IFDEF MACOSX} cdecl {$ELSE} stdcall {$ENDIF}; external {$IFNDEF DARWIN}VMProtectDLLName{$ENDIF} {$IFDEF MACOS} name '_VMProtectGetOfflineDeactivationString'{$ENDIF};

end.