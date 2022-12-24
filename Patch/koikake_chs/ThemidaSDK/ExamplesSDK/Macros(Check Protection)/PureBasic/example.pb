;
; ------------------------------------------------------------
;
;   PureBasic - WinLicense macros example file
;
;    (c) 2006 - Oreans Technologies
;
; ------------------------------------------------------------
;

XIncludeFile "WinLicenseSDKMacros.pbi"
 
Global StatusProtection.l = 0
Global StatusVirtualPC.l = 0


; ---- CHECK_PROTECTION test ----

; Add "v_" prefix to our global variable, so it can be accessed from inline assembly

CHECK_PROTECTION(v_StatusProtection,3333) 

If StatusProtection = 3333
  MessageRequester("CHECK_PROTECTION", "Protection OK!", 0)
Else
  MessageRequester("CHECK_PROTECTION", "Protection fail!", 0)
EndIf


; ---- CHECK_CODE_INTEGRITY test ----

CHECK_CODE_INTEGRITY(v_StatusProtection,1234) 

If StatusProtection = 1234
  MessageRequester("CHECK_CODE_INTEGRITY", "Code Integrity OK!", 0)
Else
  MessageRequester("CHECK_CODE_INTEGRITY", "Code Integrity fail!", 0)
EndIf  


; ---- CHECK_VIRTUAL_PC test ----
  
CHECK_VIRTUAL_PC(v_StatusVirtualPC, 12345678)

If StatusVirtualPC = 12345678
  MessageRequester("CHECK_VIRTUAL_PC", "Not running under Virtual PC", 0)
Else
  MessageRequester("CHECK_VIRTUAL_PC", "Running under Virtual PC", 0)
EndIf  
  
; IDE Options = PureBasic 4.31 (Windows - x86)
; CursorPosition = 47
; FirstLine = 16
; Executable = Example.exe