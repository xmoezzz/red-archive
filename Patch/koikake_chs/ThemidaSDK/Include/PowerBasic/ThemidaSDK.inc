
' ThemidaSDK.inc

' Power Basic Macro Definitions for Oreans Technlogies 
' Converted from C by Mike Trader 2007
                
       
'#COMPILE EXE  "CV.exe" ' Example use
'#INCLUDE "ThemidaSDK.inc"
'FUNCTION PBMAIN() AS LONG
'  LOCAL i AS LONG
'    VM_START      
'    i = 22 / 2
'    i = i * 8  
'    VM_END
'  MSGBOX STR$(i)
'END FUNCTION

             
MACRO CODEREPLACE_START
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO 


MACRO CODEREPLACE_END
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H01
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO 


MACRO ENCODE_START
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H04
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO 


MACRO ENCODE_END
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H05
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO


MACRO CLEAR_START
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H06
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO 


MACRO CLEAR_END
  !db &HEB
  !db &H15
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H07
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
END MACRO  


MACRO VM_START
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H0C
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO 


MACRO VM_END
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H0D
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO


MACRO MUTATE_START
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H10
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO 


MACRO MUTATE_END
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H11
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO 


MACRO STR_ENCRYPT_START
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H12
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO 


MACRO STR_ENCRYPT_END
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H13
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO 


MACRO STR_ENCRYPTW_START
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H22
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO 


MACRO STR_ENCRYPTW_END
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H23
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO 



MACRO UNPROTECTED_START
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H20
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO 


MACRO UNPROTECTED_END
  !db &HEB
  !db &H10
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
  !db &H21
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H00
  !db &H57
  !db &H4C
  !db &H20
  !db &H20
END MACRO
  



