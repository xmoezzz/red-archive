///White Eternity Text Decode

#pragma once

#include "stdio.h"
#include "memory.h"
#include "time.h"
#include "stdlib.h"

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")

#define PLAIN_FILE_OPEN_ERROR -1
#define KEY_FILE_OPEN_ERROR -2
#define CIPHER_FILE_OPEN_ERROR -3
#define OK 1

typedef char ElemType;

int ByteToBit(ElemType ch,ElemType bit[8]);
int BitToByte(ElemType bit[8],ElemType *ch);
int Char8ToBit64(ElemType ch[8],ElemType bit[64]);
int Bit64ToChar8(ElemType bit[64],ElemType ch[8]);
int DES_MakeSubKeys(ElemType key[64],ElemType subKeys[16][48]);
int DES_PC1_Transform(ElemType key[64], ElemType tempbts[56]);
int DES_PC2_Transform(ElemType key[56], ElemType tempbts[48]);
int DES_ROL(ElemType data[56], int time);
int DES_IP_Transform(ElemType data[64]);
int DES_IP_1_Transform(ElemType data[64]);
int DES_E_Transform(ElemType data[48]);
int DES_P_Transform(ElemType data[32]);
int DES_SBOX(ElemType data[48]);
int DES_XOR(ElemType R[48], ElemType L[48],int count);
int DES_Swap(ElemType left[32],ElemType right[32]);
int DES_EncryptBlock(ElemType plainBlock[8], ElemType subKeys[16][48], ElemType cipherBlock[8]);
int DES_DecryptBlock(ElemType cipherBlock[8], ElemType subKeys[16][48], ElemType plainBlock[8]);
int DES_Encrypt(char *plainFile, char *keyStr,char *cipherFile);
int DES_Decrypt(char *cipherFile, char *keyStr,char *plainFile);

int DES_Encrypt_Stream(char *plainFile, unsigned int in_len,char *keyStr,char *cipherFile, unsigned int &out_len);
int DES_Decrypt_Stream(char *cipherFile, unsigned int in_len, char *keyStr,char *plainFile, unsigned int &out_len);



