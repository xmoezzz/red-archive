#include <Windows.h>
#include <stdio.h>
#include <string>
#include "sha512.h"
#include <random>


#include "cryptopp/rsa.h"
#include "cryptopp/osrng.h"
#include "cryptopp/base64.h"
#include "cryptopp/files.h"
using namespace CryptoPP;

#pragma comment(lib, "cryptlib.lib")

void GetCpuInfo()
{
	int    Result[4];
	BYTE   CpuSha[64] = {0};
	::SHA512 Sha512;
	std::mt19937 rand;
	
	rand.seed(GetTickCount());
	Sha512.init();
	RtlZeroMemory(Result, sizeof(Result));
	__cpuid(Result, 1);
	Sha512.update((BYTE*)Result, sizeof(Result));

	RtlZeroMemory(Result, sizeof(Result));
	__cpuid(Result, 0x80000002); 
	Sha512.update((BYTE*)Result, sizeof(Result));

	RtlZeroMemory(Result, sizeof(Result));
	__cpuid(Result, 0x80000003);
	Sha512.update((BYTE*)Result, sizeof(Result));

	RtlZeroMemory(Result, sizeof(Result));
	__cpuid(Result, 0x80000004);
	Sha512.update((BYTE*)Result, sizeof(Result));
	Sha512.final(CpuSha);

	char buf[2 * 64 + 1];
	buf[2 * 64] = 0;
	for (int i = 0; i < 64; i++)
		sprintf(buf + i * 2, "%02x", CpuSha[i]);

	//pk
	DWORD Key[16];
	for (DWORD i = 0; i < 16; i++)
		Key[i] = rand();
	
	auto res = std::string(buf);
	printf("HW : %s\n", res.c_str());

	Base64Encoder machinesink(new FileSink("./mac.txt"));
	machinesink.Put(CpuSha, sizeof(CpuSha));
	machinesink.MessageEnd();

		// InvertibleRSAFunction is used directly only because the private key
		// won't actually be used to perform any cryptographic operation;
		// otherwise, an appropriate typedef'ed type from rsa.h would have been used.
		AutoSeededRandomPool rng;
		InvertibleRSAFunction privkey;
		privkey.Initialize(rng, 1024);

		// With the current version of Crypto++, MessageEnd() needs to be called
		// explicitly because Base64Encoder doesn't flush its buffer on destruction.
		Base64Encoder privkeysink(new FileSink("./privkey.txt"));
		privkey.DEREncode(privkeysink);
		privkeysink.MessageEnd();

		// Suppose we want to store the public key separately,
		// possibly because we will be sending the public key to a third party.
		RSAFunction pubkey(privkey);

		Base64Encoder pubkeysink(new FileSink("./pubkey.txt"));
		pubkey.DEREncode(pubkeysink);
		pubkeysink.MessageEnd();
}


int wmain(int argc, WCHAR* argv[])
{
	printf("---X'moe test machine key gen---\n");
	GetCpuInfo();
	printf("pls keep private key secret and send public key to me.\n");
	printf("press any key to exit.\n");
	getchar();
	return 0;
}

