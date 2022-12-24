#pragma once

#include <Windows.h>

#define uchar unsigned char // 8-bit byte
#define uint unsigned long // 32-bit word

void aes_encrypt(uchar in[], uchar out[], uint key[], int keysize);
void aes_decrypt(uchar in[], uchar out[], uint key[], int keysize);
