#include <stdlib.h>
#include <my.h>
#include <string>

extern "C"{
#include "ecrypt-sync.h"
}

#pragma comment(lib, "MyLibrary_x86_static.lib")

int wmain(int argc, WCHAR* argv[])
{
	NtFileDisk File;
	NTSTATUS   Status;
	ULONG      Size;
	PBYTE      Buffer;
	PBYTE      Buffer2;

	BYTE       Iv [32];
	BYTE       Key[32];

	if (argc < 2)
		return 0;

	srand(time(NULL));
	ml::MlInitialize();
	Status = File.Open(argv[1]);
	if (NT_FAILED(Status))
		return 0;

	Size = File.GetSize32();
	Buffer  = (BYTE*)AllocateMemoryP(ROUND_UP(Size, 32), HEAP_ZERO_MEMORY);
	Buffer2 = (BYTE*)AllocateMemoryP(ROUND_UP(Size, 32), HEAP_ZERO_MEMORY);
	File.Read(Buffer, Size);
	File.Close();

	DWORD Sign = TAG4('NANA');
	BYTE  Padding[12];
	for (ULONG i = 0; i < 12; i++)
		Padding[i] = (BYTE)rand();
	for (ULONG i = 0; i < 32; i++)
		Iv[i] = (BYTE)rand();
	for (ULONG i = 0; i < 32; i++)
		Key[i] = (BYTE)rand();

	ECRYPT_ctx ctx = { 0 };

	ECRYPT_keysetup(&ctx, Key, 256, 256);
	ECRYPT_ivsetup(&ctx, Iv);
	ECRYPT_encrypt_bytes(&ctx, Buffer, Buffer2, ROUND_UP(Size, 32));

	NtFileDisk Writer;
	Writer.Create((std::wstring(argv[1]) + L".out").c_str());
	Writer.Write((BYTE*)&Sign, 4);
	Writer.Write(Padding, 12);
	Writer.Write(Key, 32);
	Writer.Write(Iv,  32);
	Writer.Write(Buffer2, ROUND_UP(Size, 32));
	Writer.Close();
	return 0;
}


