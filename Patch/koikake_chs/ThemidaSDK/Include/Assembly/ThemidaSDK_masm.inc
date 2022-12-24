; ****************************************************************************
; Header: ThemidaSDK.inc
;
; Description: SecureEngine macros definition for Themida
;
; Author/s: Oreans Technologies 
;
; (c) 2011 Oreans Technologies
; ****************************************************************************


; ****************************************************************************
;                                General Constants
; ****************************************************************************

ID_CODEREPLACE_START    EQU     0h
ID_CODEREPLACE_END      EQU     1h
ID_REGISTERED_START     EQU     2h
ID_REGISTERED_END       EQU     3h
ID_ENCODE_START         EQU     4h
ID_ENCODE_END           EQU     5h
ID_CLEAR_START          EQU     6h
ID_CLEAR_END            EQU     7h
ID_UNREGISTERED_START   EQU     8h
ID_UNREGISTERED_END     EQU     9h
ID_VM_START             EQU     0Ch
ID_VM_END               EQU     0Dh
ID_MUTATE_START         EQU     10h
ID_MUTATE_END           EQU     11h
ID_UNPROTECTED_START    EQU     20h
ID_UNPROTECTED_END      EQU     21h


; ****************************************************************************
;                                 Macros definition
; ****************************************************************************

CODEREPLACE_START MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_CODEREPLACE_START
    dd      0
    db      'WL  '

    @@:

ENDM    

CODEREPLACE_END MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_CODEREPLACE_END
    dd      0
    db      'WL  '

    @@:

ENDM    

ENCODE_START MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_ENCODE_START
    dd      0
    db      'WL  '

    @@:

ENDM    

ENCODE_END MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_ENCODE_END
    dd      0
    db      'WL  '

    @@:

ENDM    

CLEAR_START MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_CLEAR_START
    dd      0
    db      'WL  '

    @@:

ENDM    

CLEAR_END MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_CLEAR_END
    dd      0
    db      'WL  '
    dd      0
    db      0
    @@:

ENDM    

REGISTERED_START MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_REGISTERED_START
    dd      0
    db      'WL  '

    @@:

ENDM    

REGISTERED_END MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_REGISTERED_END
    dd      0
    db      'WL  '

    @@:

ENDM    

UNREGISTERED_START MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_UNREGISTERED_START
    dd      0
    db      'WL  '

    @@:

ENDM    

UNREGISTERED_END MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_UNREGISTERED_END
    dd      0
    db      'WL  '

    @@:

ENDM    

VM_START MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_VM_START
    dd      0
    db      'WL  '

    @@:

ENDM    

VM_END MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_VM_END
    dd      0
    db      'WL  '

    @@:

ENDM   

MUTATE_START MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_MUTATE_START
    dd      0
    db      'WL  '

    @@:

ENDM    

MUTATE_END MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_MUTATE_END
    dd      0
    db      'WL  '

    @@:

ENDM    


UNPROTECTED_START MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_UNPROTECTED_START
    dd      0
    db      'WL  '

    @@:

ENDM    

UNPROTECTED_END MACRO 

    jmp     @F

    db      'WL  '
    dd      ID_UNPROTECTED_END
    dd      0
    db      'WL  '

    @@:

ENDM   


