; ****************************************************************************
; Module: asm_macros_x32.asm
; Description: Another way to link with the SecureEngine SDK via an ASM module
;
; Authors: Oreans Technologies 
; (c) 2013 Oreans Technologies
; ****************************************************************************

IFDEF RAX 

ELSE 

.586
.model flat,stdcall
option casemap:none

ENDIF


; ****************************************************************************
;                               Data Segment
; ****************************************************************************

.DATA             


; ****************************************************************************
;                                 Constants
; ****************************************************************************

.CONST 

VM_START_ID             =   1
VM_END_ID               =   2
CODEREPLACE_START_ID    =   3
CODEREPLACE_END_ID      =   4
REGISTERED_START_ID     =   5
REGISTERED_END_ID       =   6
ENCODE_START_ID         =   7
ENCODE_END_ID           =   8
CLEAR_START_ID          =   9
CLEAR_END_ID            =   10
UNREGISTERED_START_ID   =   11
UNREGISTERED_END_ID     =   12
REGISTEREDVM_START_ID   =   13
REGISTEREDVM_END_ID     =   14
UNPROTECTED_START_ID    =   15   
UNPROTECTED_END_ID      =   16   
CHECK_PROTECTION_ID     =   17
CHECK_CODE_INTEGRITY_ID =   18
CHECK_REGISTRATION_ID   =   19
CHECK_VIRTUAL_PC_ID     =   20
MUTATE_START_ID         =   21
MUTATE_END_ID           =   22
STR_ENCRYPT_START_ID    =   23
STR_ENCRYPT_END_ID      =   24
STR_ENCRYPTW_START_ID   =   27
STR_ENCRYPTW_END_ID     =   28


; ****************************************************************************
;                               Code Segment
; ****************************************************************************
                  
.CODE        

IFDEF RAX 

VM_START_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, VM_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

VM_START_ASM64 ENDP

VM_END_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, VM_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

VM_END_ASM64 ENDP

REGISTERED_START_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, REGISTERED_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

REGISTERED_START_ASM64 ENDP

REGISTERED_END_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, REGISTERED_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

REGISTERED_END_ASM64 ENDP

ENCODE_START_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, ENCODE_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

ENCODE_START_ASM64 ENDP

ENCODE_END_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, ENCODE_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

ENCODE_END_ASM64 ENDP

CLEAR_START_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, CLEAR_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

CLEAR_START_ASM64 ENDP

CLEAR_END_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, CLEAR_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

CLEAR_END_ASM64 ENDP

MUTATE_START_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, MUTATE_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

MUTATE_START_ASM64 ENDP

MUTATE_END_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, MUTATE_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

MUTATE_END_ASM64 ENDP

STR_ENCRYPT_START_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, STR_ENCRYPT_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

STR_ENCRYPT_START_ASM64 ENDP

STR_ENCRYPT_END_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, STR_ENCRYPT_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

STR_ENCRYPT_END_ASM64 ENDP

STR_ENCRYPTW_START_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, STR_ENCRYPTW_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

STR_ENCRYPTW_START_ASM64 ENDP

STR_ENCRYPTW_END_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, STR_ENCRYPTW_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

STR_ENCRYPTW_END_ASM64 ENDP

UNREGISTERED_START_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, UNREGISTERED_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

UNREGISTERED_START_ASM64 ENDP

UNREGISTERED_END_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, UNREGISTERED_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

UNREGISTERED_END_ASM64 ENDP

REGISTEREDVM_START_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, REGISTEREDVM_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

REGISTEREDVM_START_ASM64 ENDP

REGISTEREDVM_END_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, REGISTEREDVM_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

REGISTEREDVM_END_ASM64 ENDP

UNPROTECTED_START_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, UNPROTECTED_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

UNPROTECTED_START_ASM64 ENDP

UNPROTECTED_END_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, UNPROTECTED_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    ret

UNPROTECTED_END_ASM64 ENDP

CHECK_PROTECTION_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, CHECK_PROTECTION_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    mov     dword ptr [rcx], edx
    ret

CHECK_PROTECTION_ASM64 ENDP

CHECK_CODE_INTEGRITY_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, CHECK_CODE_INTEGRITY_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    mov     dword ptr [rcx], edx
    ret

CHECK_CODE_INTEGRITY_ASM64 ENDP

CHECK_REGISTRATION_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, CHECK_REGISTRATION_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    mov     dword ptr [rcx], edx
    ret

CHECK_REGISTRATION_ASM64 ENDP

CHECK_VIRTUAL_PC_ASM64 PROC

    push    rax
    push    rbx
    push    rcx 

    mov     eax, 'TMWL'
    mov     ebx, CHECK_VIRTUAL_PC_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     rcx
    pop     rbx
    pop     rax
    mov     dword ptr [rcx], edx
    ret

CHECK_VIRTUAL_PC_ASM64 ENDP

ELSE
                  
VM_START_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, VM_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

VM_START_ASM32 ENDP

VM_END_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, VM_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

VM_END_ASM32 ENDP

REGISTERED_START_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, REGISTERED_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

REGISTERED_START_ASM32 ENDP

REGISTERED_END_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, REGISTERED_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

REGISTERED_END_ASM32 ENDP

ENCODE_START_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, ENCODE_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

ENCODE_START_ASM32 ENDP

ENCODE_END_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, ENCODE_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

ENCODE_END_ASM32 ENDP

CLEAR_START_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, CLEAR_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

CLEAR_START_ASM32 ENDP

CLEAR_END_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, CLEAR_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

CLEAR_END_ASM32 ENDP

MUTATE_START_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, MUTATE_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

MUTATE_START_ASM32 ENDP

MUTATE_END_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, MUTATE_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

MUTATE_END_ASM32 ENDP

STR_ENCRYPT_START_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, STR_ENCRYPT_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

STR_ENCRYPT_START_ASM32 ENDP

STR_ENCRYPT_END_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, STR_ENCRYPT_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

STR_ENCRYPT_END_ASM32 ENDP

STR_ENCRYPTW_START_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, STR_ENCRYPTW_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

STR_ENCRYPTW_START_ASM32 ENDP

STR_ENCRYPTW_END_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, STR_ENCRYPTW_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

STR_ENCRYPTW_END_ASM32 ENDP

UNREGISTERED_START_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, UNREGISTERED_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

UNREGISTERED_START_ASM32 ENDP

UNREGISTERED_END_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, UNREGISTERED_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

UNREGISTERED_END_ASM32 ENDP

REGISTEREDVM_START_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, REGISTEREDVM_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

REGISTEREDVM_START_ASM32 ENDP

REGISTEREDVM_END_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, REGISTEREDVM_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

REGISTEREDVM_END_ASM32 ENDP

UNPROTECTED_START_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, UNPROTECTED_START_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

UNPROTECTED_START_ASM32 ENDP

UNPROTECTED_END_ASM32 PROC

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, UNPROTECTED_END_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax
    ret

UNPROTECTED_END_ASM32 ENDP

CHECK_PROTECTION_ASM32 PROC var_ptr, var_value

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, CHECK_PROTECTION_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax

    ; set fake value in unprotected state
    push    eax
    push    ebx
    mov     eax, [var_ptr]
    mov     ebx, [var_value]
    mov     dword ptr [eax], ebx
    pop     ebx
    pop     eax

    ret

CHECK_PROTECTION_ASM32 ENDP

CHECK_CODE_INTEGRITY_ASM32 PROC var_ptr, var_value

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, CHECK_CODE_INTEGRITY_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax

    ; set fake value in unprotected state
    push    eax
    push    ebx
    mov     eax, [var_ptr]
    mov     ebx, [var_value]
    mov     dword ptr [eax], ebx
    pop     ebx
    pop     eax

    ret

CHECK_CODE_INTEGRITY_ASM32 ENDP

CHECK_REGISTRATION_ASM32 PROC var_ptr, var_value
 
    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, CHECK_REGISTRATION_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax

    ; set fake value in unprotected state
    push    eax
    push    ebx
    mov     eax, [var_ptr]
    mov     ebx, [var_value]
    mov     dword ptr [eax], ebx
    pop     ebx
    pop     eax

    ret

CHECK_REGISTRATION_ASM32 ENDP

CHECK_VIRTUAL_PC_ASM32 PROC var_ptr, var_value

    push    eax
    push    ebx
    push    ecx 

    mov     eax, 'TMWL'
    mov     ebx, CHECK_VIRTUAL_PC_ID
    mov     ecx, 'TMWL'
    add     ebx, eax
    add     ecx, eax

    pop     ecx
    pop     ebx
    pop     eax

    ; set fake value in unprotected state
    push    eax
    push    ebx
    mov     eax, [var_ptr]
    mov     ebx, [var_value]
    mov     dword ptr [eax], ebx
    pop     ebx
    pop     eax

    ret

CHECK_VIRTUAL_PC_ASM32 ENDP

ENDIF 

include SecureEngineCustomVmMacros.inc

END 

