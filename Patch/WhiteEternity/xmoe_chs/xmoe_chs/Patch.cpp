#include "stdafx.h"

#include "Patch.h"
#include "Common.h"
#include <string.h>

#include "zlib128\zlib-1.2.8\zlib.h"

#pragma comment(lib,"zlib1.lib")
#pragma comment(lib, "User32.lib")

//Global
char *pFileBuffer = 0;//hcb Real Buffer

BYTE WhiteNameCHS[] =       {0x1F, 0x66, 0xB0, 0x8F, 0x4B,
							 0x60, 0xF2, 0x66, 0x84, 0x76,
							 0x7D, 0x76, 0x72, 0x82, 0x38,
							 0x6C, 0x52, 0x60, 0x00, 0x00};

static const DWORD WhiteName_Len = 20;

BYTE WhiteNameEn[] = {0x57, 0x00, 0x68, 0x00, 0x69, 0x00, 0x74, 0x00, 0x65, 0x00, 0x45, 0x00, 0x74, 0x00, 0x65, 0x00, 
                      0x00, 0x00};

void memcopy(void* dest,void*src,size_t size)
{
    DWORD oldProtect;
    VirtualProtect(dest,size,PAGE_EXECUTE_READWRITE,&oldProtect);
    memcpy(dest,src,size);
}

void SetNopCode(BYTE* pnop,size_t size)
{
    DWORD oldProtect;
    VirtualProtect((PVOID)pnop,size,PAGE_EXECUTE_READWRITE,&oldProtect);
    for(size_t i=0;i<size;i++)
    {
        pnop[i] = 0x90;
    }
}

//0049BB02
//0049BC88
//0049BDA0
void* HookStringTableName = (void*)0x004B044E;
void* HookString2 = (void*)0x0049BB02;
void* HookString3 = (void*)0x0049BC88;
void* HookString4 = (void*)0x0049BDA0;

void HookStringTable()
{
	memcopy(HookStringTableName, WhiteNameCHS, 20);
}

void HookStringTable2()
{
	memcopy(HookStringTableName, WhiteNameEn, 18);
	memcopy(HookString2, WhiteNameEn, 18);
	memcopy(HookString3, WhiteNameEn, 18);
	memcopy(HookString4, WhiteNameEn, 18);
}
/*
Main Loop

004454B0   > 8B86 1C080000  MOV EAX,DWORD PTR DS:[ESI+81C]
004454B6   . 8B4E 04        MOV ECX,DWORD PTR DS:[ESI+4]
004454B9   . 0FB60C01       MOVZX ECX,BYTE PTR DS:[ECX+EAX]
004454BD   . 0FB6D1         MOVZX EDX,CL
004454C0   . 40             INC EAX
004454C1   . 8986 1C080000  MOV DWORD PTR DS:[ESI+81C],EAX
004454C7   . 8B0495 E015480>MOV EAX,DWORD PTR DS:[EDX*4+4815E0]
004454CE   . 8BCE           MOV ECX,ESI
004454D0   . FFD0           CALL EAX
004454D2   . 80BE 10080000 >CMP BYTE PTR DS:[ESI+810],0
004454D9   .^74 D5          JE SHORT WhiteEte.004454B0
004454DB   > 5E             POP ESI
*/


//PushString
/*
00445B50   . 8B81 20080000  MOV EAX,DWORD PTR DS:[ECX+820]
00445B56   . 8D50 01        LEA EDX,DWORD PTR DS:[EAX+1]
00445B59   . 8991 20080000  MOV DWORD PTR DS:[ECX+820],EDX
00445B5F   . C644C1 08 04   MOV BYTE PTR DS:[ECX+EAX*8+8],4
00445B64   . 8B91 1C080000  MOV EDX,DWORD PTR DS:[ECX+81C]
00445B6A   . 56             PUSH ESI
00445B6B   . 8B71 04        MOV ESI,DWORD PTR DS:[ECX+4]
00445B6E   . 0FB63416       MOVZX ESI,BYTE PTR DS:[ESI+EDX]
00445B72   . 42             INC EDX
00445B73   . 8991 1C080000  MOV DWORD PTR DS:[ECX+81C],EDX      //EIP
00445B79   . 8954C1 0C      MOV DWORD PTR DS:[ECX+EAX*8+C],EDX  //(char*)EDX-->String>>文件中的位置,需要Hook SetFont
00445B7D   . 01B1 1C080000  ADD DWORD PTR DS:[ECX+81C],ESI      //EIP += StringLength>>
00445B83   . 5E             POP ESI
00445B84   . C3             RETN
*/


void* EndPushString = (void*) 0x00445B83;
void* StartPushString = (void*) 0x00445B79;
//DWORD StringAdd;
__declspec(naked) void MyPushString()
{
	__asm
	{
		pushad
		push edx
		call EncodeText
		popad
		mov  dword ptr [ecx + eax*8 + 0xC], edx
		add  dword ptr [ecx + 0x81C], esi
		jmp  EndPushString
	}
}

//Debug
/*
004454C0   . 40             INC EAX
004454C1   . 8986 1C080000  MOV DWORD PTR DS:[ESI+81C],EAX
004454C7   . 8B0495 E015480>MOV EAX,DWORD PTR DS:[EDX*4+4815E0]
004454CE   . 8BCE           MOV ECX,ESI
004454D0   . FFD0           CALL EAX
004454D2   . 80BE 10080000 >CMP BYTE PTR DS:[ESI+810],0
004454D9   .^74 D5          JE SHORT WhiteEte.004454B0
004454DB   > 5E             POP ESI
*/

DWORD ScrPos;

void __stdcall GetAddr(DWORD p)
{
	FILE *fin = NULL;
	fopen_s(&fin, "Main.txt", "a");
	fprintf_s(fin, "[%08x]\r\n", p);
	fclose(fin);
}

#include <vector>

typedef struct DebugInfo
{
	DWORD Pos;
	DWORD ThreadId;
};

std::vector<DebugInfo> info;

void __stdcall TextCall(DWORD offset, DWORD Pos, unsigned char code)
{
	/*
	DebugInfo _info;
	_info.Pos = Pos;
	_info.ThreadId = GetCurrentThreadId();
	info.push_back(_info);
	*/
	if(offset == 0)
	{
		char msg[260];
		memset(msg, 0, sizeof(msg));
		sprintf_s(msg, "Invalid Call[%08x], ScriptPos = [%08x]\nCode = [%02x]", offset, ScrPos, code);

		/*
		FILE *dump = fopen("Crash.log", "wb");
		for(std::vector<DebugInfo>::iterator it = info.begin(); it != info.end(); it++)
		{
			fprintf(dump, "********************\nThread ID[%08x]\nCall Offset[%08x]\n", it->ThreadId, it->Pos);
		}
		fclose(dump);
		*/
		MessageBoxA(NULL, msg, "Debugger", MB_OK);
	}
}

void* MainLoopPoint = (void*) 0x004454C1;
void* MainLoopEnd   = (void*) 0x004454D9;
void* ret_inner     = (void*) 0x004454B0;
__declspec(naked) void HookMainLoop()
{
	__asm
	{
		MOV DWORD PTR DS:[ESI+ 0x81C],EAX
		mov ScrPos,EAX
		mov eax, dword ptr DS:[EDX*4 + 0x4815E0]
		pushad
		mov ecx,edx
		mov edx, ScrPos
		push ecx
		push edx
		push eax
		call TextCall
		popad
		MOV ECX,ESI
		CALL EAX
		CMP BYTE PTR DS:[ESI+0x810],0
		jmp MainLoopEnd
	}
}

//00438EF0  |. 68 B4DB4500    PUSH WhiteEte.0045DBB4                   ; |FileName = "*.hcb"
void* hcb     = (void*)0x0045DBB4;
void* hcb_end = (void*)0x00438EF5;
void SetFile()
{
	BYTE pck[] = {'*', '.', 'p', 'c', 'k'};
	memcopy(hcb, pck, sizeof(pck));
}

//00438337  |. 68 64DA4500    PUSH WhiteEte.0045DA64                   ;  ASCII "%s/save/s%03d.bin"
void* SaveFile1_Start = (void*)0x0045DA64;
void* SaveFile2_Start = (void*)0x0045DBBC;
void* SaveFile3_Start = (void*)0x0045DBCC;

//const char* save = "%s/sav/s%03d.bin";
const char* save = "%s/sav/s%03d.bin";
const char* save2 = "/sav/save.bin";
const char* save3 = "/sav";
void HookSaveFile1()
{
	memcopy(SaveFile1_Start, (PBYTE)save, strlen(save)+1);
	memcopy(SaveFile2_Start, (PBYTE)save2,strlen(save2)+1);
	memcopy(SaveFile3_Start, (PBYTE)save3,strlen(save3)+1);
}

/*
00438326  |. A1 68134800    MOV EAX,DWORD PTR DS:[481368]
0043832B  |. 8B88 647E6A00  MOV ECX,DWORD PTR DS:[EAX+6A7E64]
*/

void* GameSave_Start=(void*)0x00438326;
void* GameSave_End = (void*)0x00438331;
__declspec(naked) void HookGameSave()
{
	__asm
	{
		lea ecx, save2
		jmp GameSave_End
	}
}

char pck[] = "*.pck";
__declspec(naked) void HookFile()
{
	__asm
	{
		push pck
		jmp hcb_end
	}
}

//0042D22D  |. 68 86000000    PUSH 86                                  ; |CharSet = 134.
void SetCHS()
{
	void* ByteCode = (void*)0x0042D22D;
	BYTE code[] = {0x68, 0x86, 0x00, 0x00, 0x00};
	memcopy(ByteCode, code, sizeof(code));
}

void __stdcall MsgFailedToAlloc()
{
	MessageBoxA(NULL, "Failed to allocate memory!", "WhiteEternityCHS", MB_OK);
}

void __stdcall MsgFailedToOpen()
{
	MessageBoxA(NULL, "Failed to read file!", "WhiteEternityCHS", MB_OK);
}

void __stdcall MsgNullFile()
{
	char tmp[1024];
	sprintf(tmp, "Bad file pointer![%08x]", GetLastError());
	MessageBoxA(NULL, tmp, "WhiteEternityCHS", MB_OK);
}

void __stdcall ErrorExit()
{
	exit(-1);
}

void __stdcall OpenHcbDirectly(const char *str)
{
	HANDLE hOpenFile = CreateFileA(str,
								   GENERIC_READ,
								   FILE_SHARE_READ,
								   NULL,
								   OPEN_EXISTING,
								   0x8000000,
								   NULL);
								   
								   
	if(hOpenFile == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL, "Failed to CreateFile", "WhiteEternityCHS", MB_OK);
		CloseHandle(hOpenFile);
		ErrorExit();
	}
	HANDLE hHeap = GetProcessHeap();
	DWORD FileSize = GetFileSize(hOpenFile, NULL);
	if(hHeap == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL, "Failed to get process heap", "WhiteEternityCHS", MB_OK);
		CloseHandle(hHeap);
		ErrorExit();
	}
	pFileBuffer = (char*)HeapAlloc(hHeap, NULL, FileSize);
	if(pFileBuffer == NULL)
	{
		MessageBoxA(NULL, "Failed to allocate memory", "WhiteEternityCHS", MB_OK);
		HeapFree(hHeap, NULL, pFileBuffer);
		ErrorExit();
	}
	DWORD ReadedCount = 0;
	ReadFile(hOpenFile, pFileBuffer, FileSize, &ReadedCount, NULL);
	if(ReadedCount != FileSize)
	{
		MessageBoxA(NULL, "Failed to read file", "WhiteEternityCHS", MB_OK);
		ErrorExit();
	}
}

void *Proc_ReadHcb = (void*)0x00438F4C;
void *End_ReadHcb  = (void*)0x00438F51;


__declspec(naked) void HookReadHcbFile2()
{
	__asm
	{
		pushad
		call OpenHcbDirectly
		popad
		jmp End_ReadHcb
	}
}

__declspec(naked) void MyHeapAlloc()
{
	__asm
	{
		push esi
		push edi
		mov  edi, dword ptr ss:[esp + 0xC]
		mov  esi, ecx
		mov  eax, dword ptr ds:[esi]
		test edi, edi
		jg HandleOk
		pushad
		call MsgFailedToOpen
		popad
		pushad
		call ErrorExit
		popad

		HandleOk:
		push edi
		test eax, eax
		jnz  bFlagReAlloc

		push eax
		call dword ptr ds:[GetProcessHeap]
		push eax
		call dword ptr ds:[HeapAlloc]
		jmp  bFlagAllocDone

		bFlagReAlloc:
		push eax
		push 0
		call dword ptr ds:[GetProcessHeap]

		push eax
		call dword ptr ds:[HeapReAlloc]

		bFlagAllocDone:
		mov dword ptr ds:[esi], eax
		jmp Final

		Final:
		mov dword ptr ds:[esi + 4], edi
		pop edi
		pop esi
		retn 4
		
	}
}

void __stdcall MsgShow(const char *str)
{
	MessageBoxA(NULL, str, "WhiteEternityCHS", MB_OK);
}

char* __stdcall ReadHcbFile(const char *str)
{
	if(!str)
		return 0;
	FILE *fin = fopen(str, "rb");
	fseek(fin,0,SEEK_END);
    unsigned __int32 dwFileSize=ftell(fin);
    rewind(fin);
    char *wFile = new char[dwFileSize];
   	fread(wFile,dwFileSize,1,fin);
	fclose(fin);

	return wFile;
}

__declspec(naked) void MyReadhcbFile()//filename
{
	__asm
	{
		push ecx
		push ebx
		mov  ebx, dword ptr ss:[esp + 0xC]

		;;Debug-- No Error
		;;pushad
		;;push edx
		;;call MsgShow
		;;popad

		push esi
		push edi
		push 0
		push 0x8000000
		push 3
		push 0
		push 1
		push 0x80000000
		push ebx
		mov esi,ecx
		call dword ptr ds:[CreateFileA]
		mov edi, eax
		cmp edi,-1
		jnz ReadNoError
		;;--
		pushad
		call MsgNullFile
		popad
		pushad
		call ErrorExit
		popad

		ReadNoError:
		mov eax, dword ptr ds:[esi]
		;;mov dword ptr [esi], 0
		push 0
		push edi
		mov  dword ptr ds:[esi + 4], 0
		call dword ptr ds:[GetFileSize]
		push eax
		mov  ecx, esi
		call MyHeapAlloc

		mov edx, dword ptr ds:[esi + 4]
		mov eax, dword ptr ds:[esi]
		
		push 0
		lea ecx, dword ptr ss:[esp + 0x10]
		push ecx
		push edx

		mov pFileBuffer, eax
		push eax
		push edi
		call dword ptr ds:[ReadFile]

		push edi
		call dword ptr ds:[CloseHandle]

		pushad
		call Decompress
		popad
		mov eax, pFileBuffer
		mov dword ptr ds:[esi], eax

		pop edi
		pop esi
		pop ebx
		pop ecx ;;
		retn 4
	}
}


__declspec(naked) void HookReadHcbFile()
{
	__asm
	{
		call MyReadhcbFile
		jmp End_ReadHcb
	}
}

void __stdcall Decompress()
{
	//CalculateXORTable();
	//extern char XORTable_Real[1024];
	//DecodeHeader(pFileBuffer);
	DWORD OriLen = *(DWORD*)pFileBuffer;
	DWORD CompressedLen = *(DWORD*)(pFileBuffer + 4);
	/*
	DWORD iPos = 0;
	char* ppFile = pFileBuffer + 8;
	for(unsigned int i = 0; i < CompressedLen; i++)
	{
		ppFile[i] ^= XORTable_Real[1024];
		iPos++;
		iPos %= 1024;
	}
	*/
	char *pBuffer = new char[*(DWORD*)pFileBuffer];
	uncompress((PBYTE)pBuffer, &OriLen, (PBYTE)(pFileBuffer+8), CompressedLen);

	pFileBuffer = pBuffer;
}

void* Font_start = (void*)0x00443B6A;
void __stdcall SetFont()
{
	SetNopCode((PBYTE)Font_start, 2);
}


/********************************************/
//Hook -- Patch
BOOL InstallPatch()
{
	//CalculateTextXorTable();
	//HookFile
	HookStringTable2();
	HookSaveFile1();

	SetFont();

	/*
	SetNopCode((PBYTE)GameSave_Start, (DWORD)GameSave_End - (DWORD)GameSave_Start);
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&GameSave_Start, HookGameSave);
	DetourTransactionCommit();
	*/
	//HookStringTable();
	SetFile();
	/*
	SetNopCode((PBYTE)hcb, (DWORD)hcb_end - (DWORD)hcb);
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&hcb, HookFile);
	DetourTransactionCommit();
	*/

	SetCHS();

	//FileDecoding
	
	
	SetNopCode((PBYTE)Proc_ReadHcb, (DWORD)End_ReadHcb - (DWORD)Proc_ReadHcb);
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&Proc_ReadHcb, HookReadHcbFile);
	DetourTransactionCommit();
	
	//PushString
	
	/*
	SetNopCode((PBYTE)StartPushString, (DWORD)EndPushString - (DWORD)StartPushString);
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&StartPushString, MyPushString);
	DetourTransactionCommit();
	*/

	/*
	SetNopCode((PBYTE)MainLoopPoint, (DWORD)MainLoopEnd - (DWORD)MainLoopPoint);
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&MainLoopPoint, HookMainLoop);
	DetourTransactionCommit();
	*/

	return TRUE;
}
