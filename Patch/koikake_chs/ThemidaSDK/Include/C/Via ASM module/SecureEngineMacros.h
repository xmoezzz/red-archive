/****************************************************************************** 
  Header: SecureEngineMacros.h
  Description: Definition of SecureEngine macros via ASM module

  Author/s: Oreans Technologies  
  (c) 2013 Oreans Technologies
*****************************************************************************/ 

#pragma once


// ***********************************************
// Specify platform
// ***********************************************

#ifdef _WIN64
#define PLATFORM_X64
#else
#define PLATFORM_X32
#endif


// ***********************************************
// Include files
// ***********************************************

#include "SecureEngineCustomVMsMacros.h"


// ***********************************************
// Declaration of common macros
// ***********************************************

#ifdef __cplusplus
  extern "C" {
#endif

#if defined(PLATFORM_X32)

void __stdcall VM_START_ASM32();
void __stdcall VM_END_ASM32();
void __stdcall REGISTERED_START_ASM32();
void __stdcall REGISTERED_END_ASM32();
void __stdcall ENCODE_START_ASM32();
void __stdcall ENCODE_END_ASM32();
void __stdcall CLEAR_START_ASM32();
void __stdcall CLEAR_END_ASM32();
void __stdcall MUTATE_START_ASM32();
void __stdcall MUTATE_END_ASM32();
void __stdcall STR_ENCRYPT_START_ASM32();
void __stdcall STR_ENCRYPT_END_ASM32();
void __stdcall STR_ENCRYPTW_START_ASM32();
void __stdcall STR_ENCRYPTW_END_ASM32();
void __stdcall UNREGISTERED_START_ASM32();
void __stdcall UNREGISTERED_END_ASM32();
void __stdcall REGISTEREDVM_START_ASM32();
void __stdcall REGISTEREDVM_END_ASM32();
void __stdcall UNPROTECTED_START_ASM32();
void __stdcall UNPROTECTED_END_ASM32();
void __stdcall CHECK_PROTECTION_ASM32(int *, int);
void __stdcall CHECK_CODE_INTEGRITY_ASM32(int *, int);
void __stdcall CHECK_REGISTRATION_ASM32(int *, int);
void __stdcall CHECK_VIRTUAL_PC_ASM32(int *, int);

#define VM_START VM_START_ASM32();
#define VM_END VM_END_ASM32();
#define REGISTERED_START REGISTERED_START_ASM32();
#define REGISTERED_END REGISTERED_END_ASM32();
#define ENCODE_START ENCODE_START_ASM32();
#define ENCODE_END ENCODE_END_ASM32();
#define CLEAR_START CLEAR_START_ASM32();
#define CLEAR_END CLEAR_END_ASM32();
#define MUTATE_START MUTATE_START_ASM32();
#define MUTATE_END MUTATE_END_ASM32();
#define STR_ENCRYPT_START STR_ENCRYPT_START_ASM32();
#define STR_ENCRYPT_END STR_ENCRYPT_END_ASM32();
#define STR_ENCRYPTW_START STR_ENCRYPTW_START_ASM32();
#define STR_ENCRYPTW_END STR_ENCRYPTW_END_ASM32();
#define UNREGISTERED_START UNREGISTERED_START_ASM32();
#define UNREGISTERED_END UNREGISTERED_END_ASM32();
#define REGISTEREDVM_START REGISTEREDVM_START_ASM32();
#define REGISTEREDVM_END REGISTEREDVM_END_ASM32();
#define UNPROTECTED_START UNPROTECTED_START_ASM32();
#define UNPROTECTED_END UNPROTECTED_END_ASM32();
#define CHECK_PROTECTION(user_variable, user_value) CHECK_PROTECTION_ASM32(user_variable, user_value);
#define CHECK_CODE_INTEGRITY(user_variable, user_value) CHECK_CODE_INTEGRITY_ASM32(user_variable, user_value);
#define CHECK_REGISTRATION(user_variable, user_value) CHECK_REGISTRATION_ASM32(user_variable, user_value);
#define CHECK_VIRTUAL_PC(user_variable, user_value) CHECK_VIRTUAL_PC_ASM32(user_variable, user_value);

#endif

#if defined(PLATFORM_X64)

void __stdcall VM_START_ASM64();
void __stdcall VM_END_ASM64();
void __stdcall REGISTERED_START_ASM64();
void __stdcall REGISTERED_END_ASM64();
void __stdcall ENCODE_START_ASM64();
void __stdcall ENCODE_END_ASM64();
void __stdcall CLEAR_START_ASM64();
void __stdcall CLEAR_END_ASM64();
void __stdcall MUTATE_START_ASM64();
void __stdcall MUTATE_END_ASM64();
void __stdcall STR_ENCRYPT_START_ASM64();
void __stdcall STR_ENCRYPT_END_ASM64();
void __stdcall STR_ENCRYPTW_START_ASM64();
void __stdcall STR_ENCRYPTW_END_ASM64();
void __stdcall UNREGISTERED_START_ASM64();
void __stdcall UNREGISTERED_END_ASM64();
void __stdcall REGISTEREDVM_START_ASM64();
void __stdcall REGISTEREDVM_END_ASM64();
void __stdcall UNPROTECTED_START_ASM64();
void __stdcall UNPROTECTED_END_ASM64();
void __stdcall CHECK_PROTECTION_ASM64(int *, int);
void __stdcall CHECK_CODE_INTEGRITY_ASM64(int *, int);
void __stdcall CHECK_REGISTRATION_ASM64(int *, int);
void __stdcall CHECK_VIRTUAL_PC_ASM64(int *, int);

#define VM_START VM_START_ASM64();
#define VM_END VM_END_ASM64();
#define REGISTERED_START REGISTERED_START_ASM64();
#define REGISTERED_END REGISTERED_END_ASM64();
#define ENCODE_START ENCODE_START_ASM64();
#define ENCODE_END ENCODE_END_ASM64();
#define CLEAR_START CLEAR_START_ASM64();
#define CLEAR_END CLEAR_END_ASM64();
#define MUTATE_START MUTATE_START_ASM64();
#define MUTATE_END MUTATE_END_ASM64();
#define STR_ENCRYPT_START STR_ENCRYPT_START_ASM64();
#define STR_ENCRYPT_END STR_ENCRYPT_END_ASM64();
#define STR_ENCRYPTW_START STR_ENCRYPTW_START_ASM64();
#define STR_ENCRYPTW_END STR_ENCRYPTW_END_ASM64();
#define UNREGISTERED_START UNREGISTERED_START_ASM64();
#define UNREGISTERED_END UNREGISTERED_END_ASM64();
#define REGISTEREDVM_START REGISTEREDVM_START_ASM64();
#define REGISTEREDVM_END REGISTEREDVM_END_ASM64();
#define UNPROTECTED_START UNPROTECTED_START_ASM64();
#define UNPROTECTED_END UNPROTECTED_END_ASM64();
#define CHECK_PROTECTION(user_variable, user_value) CHECK_PROTECTION_ASM64(user_variable, user_value);
#define CHECK_CODE_INTEGRITY(user_variable, user_value) CHECK_CODE_INTEGRITY_ASM64(user_variable, user_value);
#define CHECK_REGISTRATION(user_variable, user_value) CHECK_REGISTRATION_ASM64(user_variable, user_value);
#define CHECK_VIRTUAL_PC(user_variable, user_value) CHECK_VIRTUAL_PC_ASM64(user_variable, user_value);

#endif

#ifdef __cplusplus
}
#endif


