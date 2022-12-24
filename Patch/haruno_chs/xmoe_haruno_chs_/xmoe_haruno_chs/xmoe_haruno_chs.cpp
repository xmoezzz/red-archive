// xmoe_haruno_chs.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <windows.h>
#include "ttstr.h"
#include "PackageWrapper.h"
#include "CharHolder.h"
#include "KAGParser.h"
#include "DebugLoad.h"
#include "xKAG.h"
#include "ThreadBuffer.h"
#include "Nop.h"

#include "detours.h"

#pragma comment(lib, "detours.lib")

static HANDLE xmoePack = INVALID_HANDLE_VALUE;

BOOL OpenPack()
{
	xmoePack = CreateFile(L"xmoe.chs", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (xmoePack == INVALID_HANDLE_VALUE)
	{
		MessageBoxW(NULL, L"无法打开汉化文件", L"[X'moe]晴霁之后，定是菜花盛开的好天气", MB_OK);
		return FALSE;
	}
	return TRUE;
}

/*
CPU Disasm
Address   Hex dump          Command                                                 Comments
005D833A  |.  53            push ebx
005D833B  |.  56            push esi
005D833C  |.  57            push edi
005D833D  |.  8955 CC       mov dword ptr [ebp-34],edx
005D8340  |.  8945 D0       mov dword ptr [ebp-30],eax

*/


ThreadBuffer buffer;

tTVPCharHolder* WINAPI xmoeLoad(void* This, ttstr* name, int isStr)
{
	if (isStr)
	{
		return NULL;
	}
	else
	{
		/*
		if (PackageManager::GetPackage()->HasScript(name->str))
		{
			return PackageManager::GetPackage()->GetScript(name->str);
		}
		else
		{
			return NULL;
		}
		*/

		FILE* file = NULL;
		_wfopen_s(&file, name->str, L"rb");
		if (file)
		{
			fseek(file, 0, SEEK_END);
			size_t size = ftell(file);
			rewind(file);
			char* buffer = new char[size];
			fread(buffer, 1, size, file);
			fclose(file);

			tTVPCharHolder* holder = new tTVPCharHolder;
			holder->Buffer = buffer;
			holder->BufferSize = size;

			return holder;
		}
		else
		{
			MessageBoxW(NULL, wstring(L"无法打开脚本 ： " + wstring(name->str)).c_str(), L"Error", MB_OK);
		}
		return NULL;
	}
}


void WINAPI xLoadScenario(void* This, const ttstr *name, bool isstring);

void* HookStart = (void*)0x005D833A;
void* HookEnd = (void*)0x005D8340;
__declspec(naked) void HookLoadScene()
{
	__asm
	{
		;; pushd

		pushad
		push ebx
		push esi
		push edi
		call xLoadScenario
		popad

		push ebx
		push esi
		push edi
		mov dword ptr[ebp - 0x34], edx
		jmp HookEnd
			;; popd
	}
}


void *NextStart = (void*)0x005D8474;
void *NextEnd = (void*)0x005D8550;

static DWORD rEBX = 0;

__declspec(naked) void HookLoadSceneNext()
{
	__asm pushad
	__asm mov ebx, buffer.buffer
	//__asm mov rEBX, ebx

	__asm popad
	__asm jmp NextEnd
}

/**********************************************/


BOOL Install()
{
	MessageBoxW(NULL, L"", L"", MB_OK);
	/*
	if (!OpenPack())
	{
		ExitProcess(-1);
	}
	*/
	//SetNopCode((PBYTE)StartXP3Second, (DWORD)EndXP3Second - (DWORD)StartXP3Second);
	/*
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID)HookStart, xLoadScenario);
	DetourTransactionCommit();
	*/

	SetNopCode((PBYTE)HookStart, (DWORD)HookEnd - (DWORD)HookStart);
	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID)HookStart, HookLoadScene);
	DetourTransactionCommit();

	return TRUE;
}


BOOL UnInstall()
{
	if (xmoePack != INVALID_HANDLE_VALUE)
	{
		return CloseHandle(xmoePack);
	}
	return FALSE;
}


/*
CPU Disasm
Address   Hex dump          Command                                                 Comments
005D8343  |.  B8 F3545700   mov eax,005754F3                                        ; Entry point
*/

void WINAPI xLoadScenario(void* This, const ttstr* name, bool isstring)
{
	void* _this;
	__asm pushad
	__asm mov _this, ebx
	__asm popad

	//char tmp0[20] = { 0 };
	//sprintf(tmp0, "0x%08x", (DWORD)(_this));
	//MessageBoxA(NULL, tmp0, "This", MB_OK);
	// load scenario from file or string to buffer
	//tTVPScenarioCacheItem* obj = (tTVPScenarioCacheItem*)This;

	//005754F3
	char tmp[20] = { 0 };

	void* pName = 0;

	__asm pushad
	__asm mov edx, dword ptr[name]
	__asm mov ecx, dword ptr[edx]
	__asm mov pName, ecx
	__asm popad

	// __asm int3
	//sprintf(tmp, "0x%08x", (DWORD) (pName->str));
	
	//Debug
	if (isstring == false)
		MessageBoxW(NULL, (wstring((wchar_t*)((char*)pName + 8)) +  L" type=false").c_str(), L"Loading", MB_OK);
	else
		MessageBoxW(NULL, (wstring((wchar_t*)((char*)pName + 8)) + L" type=true").c_str(), L"Loading", MB_OK);

	tTVPScenarioCacheItem* obj = (tTVPScenarioCacheItem*)_this;

	size_t size = 0;
	char* out_buff = DebugLoad((wchar_t*)((char*)pName + 8), size);

	buffer.ThreadId = GetCurrentThreadId();
	buffer.buffer = out_buff;
	buffer.size = size;
	/*
	xKAG* o = (xKAG*)_this;
	char tmp3[1024] = { 0 };
	sprintf(tmp3, "%08x\n%08x\n%d", o->dword920, o->dword940, o->byte93C);
	MessageBoxA(NULL, tmp3, "Log", MB_OK);
	*/
	//__asm int 3
	//obj->LoadScenario((wchar_t*)((char*)pName + 8), isstring);
}


/*
CPU Disasm
Address   Hex dump          Command                                                 Comments
005D8334  /$  55            push ebp
005D8335  |.  8BEC          mov ebp,esp
005D8337  |.  83C4 C8       add esp,-38
005D833A  |.  53            push ebx      //this
005D833B  |.  56            push esi      //ttstr:name
005D833C  |.  57            push edi      //isstring
005D833D  |.  8955 CC       mov dword ptr [ebp-34],edx
005D8340  |.  8945 D0       mov dword ptr [ebp-30],eax
005D8343  |.  B8 F3545700   mov eax,005754F3                                        ; Entry point
005D8348  |.  C745 DC 70C87 mov dword ptr [ebp-24],offset 0070C870
005D834F  |.  8965 E0       mov dword ptr [ebp-20],esp
005D8352  |.  8945 D8       mov dword ptr [ebp-28],eax
005D8355  |.  66:C745 E4 00 mov word ptr [ebp-1C],0
005D835B  |.  33D2          xor edx,edx
005D835D  |.  8955 F0       mov dword ptr [ebp-10],edx
005D8360  |.  64:A1 0000000 mov eax,dword ptr fs:[0]
005D8366  |.  8945 D4       mov dword ptr [ebp-2C],eax
005D8369  |.  8D55 D4       lea edx,[ebp-2C]
005D836C  |.  64:8915 00000 mov dword ptr fs:[0],edx

//if isstring
005D8373  |.  84C9          test cl,cl   
005D8375  |.  0F84 8A000000 jz 005D8405
005D837B  |.  8B4D CC       mov ecx,dword ptr [ebp-34]
005D837E  |.  8339 00       cmp dword ptr [ecx],0
005D8381  |.  74 1D         je short 005D83A0
005D8383  |.  8B45 CC       mov eax,dword ptr [ebp-34]
005D8386  |.  8B00          mov eax,dword ptr [eax]
005D8388  |.  85C0          test eax,eax
005D838A  |.  75 04         jnz short 005D8390
005D838C  |.  33DB          xor ebx,ebx
005D838E  |.  EB 16         jmp short 005D83A6
005D8390  |>  8378 04 00    cmp dword ptr [eax+4],0
005D8394  |.  74 05         je short 005D839B
005D8396  |.  8B58 04       mov ebx,dword ptr [eax+4]
005D8399  |.  EB 0B         jmp short 005D83A6
005D839B  |>  8D58 08       lea ebx,[eax+8]
005D839E  |.  EB 06         jmp short 005D83A6
005D83A0  |>  8B1D 84D36C00 mov ebx,dword ptr [6CD384]
005D83A6  |>  8B45 D0       mov eax,dword ptr [ebp-30]
005D83A9  |.  8B30          mov esi,dword ptr [eax]
005D83AB  |.  85F6          test esi,esi
005D83AD  |.  74 0E         jz short 005D83BD
005D83AF  |.  56            push esi                                                ; /Arg1 => [ARG.EAX]
005D83B0  |.  E8 EF86F9FF   call 00570AA4                                           ; \haruno_.00570AA4
005D83B5  |.  59            pop ecx
005D83B6  |.  8B45 D0       mov eax,dword ptr [ebp-30]
005D83B9  |.  33D2          xor edx,edx
005D83BB  |.  8910          mov dword ptr [eax],edx
005D83BD  |>  8B4D D0       mov ecx,dword ptr [ebp-30]
005D83C0  |.  33C0          xor eax,eax
005D83C2  |.  8941 04       mov dword ptr [ecx+4],eax
005D83C5  |.  85DB          test ebx,ebx
005D83C7  |.  0F84 78010000 jz 005D8545
005D83CD  |.  8BC3          mov eax,ebx
005D83CF  |.  E8 D042E6FF   call 0043C6A4                                           ; [haruno_.0043C6A4
005D83D4  |.  8BF0          mov esi,eax
005D83D6  |.  8B45 D0       mov eax,dword ptr [ebp-30]
005D83D9  |.  46            inc esi
005D83DA  |.  8970 04       mov dword ptr [eax+4],esi
005D83DD  |.  03F6          add esi,esi
005D83DF  |.  56            push esi                                                ; /Arg1
005D83E0  |.  E8 7387F9FF   call 00570B58                                           ; \haruno_.00570B58
005D83E5  |.  8BF8          mov edi,eax
005D83E7  |.  8B45 D0       mov eax,dword ptr [ebp-30]
005D83EA  |.  59            pop ecx
005D83EB  |.  8938          mov dword ptr [eax],edi
005D83ED  |.  8B55 D0       mov edx,dword ptr [ebp-30]
005D83F0  |.  8B4A 04       mov ecx,dword ptr [edx+4]
005D83F3  |.  03C9          add ecx,ecx
005D83F5  |.  51            push ecx                                                ; /Arg3
005D83F6  |.  53            push ebx                                                ; |Arg2
005D83F7  |.  57            push edi                                                ; |Arg1
005D83F8  |.  E8 03C9F9FF   call 00574D00                                           ; \haruno_.00574D00
005D83FD  |.  83C4 0C       add esp,0C
005D8400  |.  E9 40010000   jmp 005D8545

else
005D8405  |>  33C0          xor eax,eax
005D8407  |.  8945 C8       mov dword ptr [ebp-38],eax
005D840A  |.  66:C745 E4 08 mov word ptr [ebp-1C],8
005D8410  |.  66:C745 E4 14 mov word ptr [ebp-1C],14
005D8416  |.  B8 08BE7000   mov eax,offset 0070BE08
005D841B  |.  E8 3C08E3FF   call 00408C5C
TVPCreateTextStreamForRead

005D8420  |.  8945 F8       mov dword ptr [ebp-8],eax  //stream
005D8423  |.  FF45 F0       inc dword ptr [ebp-10]
005D8426  |.  8D55 F8       lea edx,[ebp-8]
005D8429  |.  8B45 CC       mov eax,dword ptr [ebp-34]
005D842C  |.  E8 ABBF0200   call 006043DC                                           ; [haruno_.006043DC
005D8431  |.  8945 C8       mov dword ptr [ebp-38],eax
005D8434  |.  FF4D F0       dec dword ptr [ebp-10]
005D8437  |.  8B45 F8       mov eax,dword ptr [ebp-8]

005D843A  |.  85C0          test eax,eax      //if (stream)
005D843C  |.  74 05         jz short 005D8443
005D843E  |.  E8 E504E3FF   call 00408928                                           ; [haruno_.00408928
005D8443  |>  66:C745 E4 08 mov word ptr [ebp-1C],8
005D8449  |.  66:C745 E4 2C mov word ptr [ebp-1C],2C
005D844F  |.  33D2          xor edx,edx
005D8451  |.  8955 FC       mov dword ptr [ebp-4],edx
005D8454  |.  FF45 F0       inc dword ptr [ebp-10]
005D8457  |.  66:C745 E4 38 mov word ptr [ebp-1C],38
005D845D  |.  837D C8 00    cmp dword ptr [ebp-38],0
005D8461  |.  74 11         je short 005D8474
005D8463  |.  6A 00         push 0
005D8465  |.  8D4D FC       lea ecx,[ebp-4]
005D8468  |.  51            push ecx
005D8469  |.  8B45 C8       mov eax,dword ptr [ebp-38]
005D846C  |.  50            push eax
005D846D  |.  8B10          mov edx,dword ptr [eax]
005D846F  |.  FF12          call dword ptr [edx]
005D8471  |.  83C4 0C       add esp,0C


005D8474  |>  837D FC 00    cmp dword ptr [ebp-4],0
005D8478  |.  74 1B         je short 005D8495
005D847A  |.  8B45 FC       mov eax,dword ptr [ebp-4]
005D847D  |.  85C0          test eax,eax
005D847F  |.  75 04         jnz short 005D8485
005D8481  |.  33DB          xor ebx,ebx
005D8483  |.  EB 16         jmp short 005D849B
005D8485  |>  8378 04 00    cmp dword ptr [eax+4],0
005D8489  |.  74 05         je short 005D8490
005D848B  |.  8B58 04       mov ebx,dword ptr [eax+4]
005D848E  |.  EB 0B         jmp short 005D849B
005D8490  |>  8D58 08       lea ebx,[eax+8]
005D8493  |.  EB 06         jmp short 005D849B
005D8495  |>  8B1D 84D36C00 mov ebx,dword ptr [6CD384]
005D849B  |>  8B45 D0       mov eax,dword ptr [ebp-30]
005D849E  |.  8B30          mov esi,dword ptr [eax]
005D84A0  |.  85F6          test esi,esi
005D84A2  |.  74 0E         jz short 005D84B2
005D84A4  |.  56            push esi                                                ; /Arg1
005D84A5  |.  E8 FA85F9FF   call 00570AA4                                           ; \haruno_.00570AA4
005D84AA  |.  59            pop ecx
005D84AB  |.  8B45 D0       mov eax,dword ptr [ebp-30]
005D84AE  |.  33D2          xor edx,edx
005D84B0  |.  8910          mov dword ptr [eax],edx
005D84B2  |>  8B4D D0       mov ecx,dword ptr [ebp-30]
005D84B5  |.  33C0          xor eax,eax
005D84B7  |.  8941 04       mov dword ptr [ecx+4],eax
005D84BA  |.  85DB          test ebx,ebx
005D84BC  |.  74 33         jz short 005D84F1
005D84BE  |.  8BC3          mov eax,ebx
005D84C0  |.  E8 DF41E6FF   call 0043C6A4                                           ; [haruno_.0043C6A4
005D84C5  |.  8BF0          mov esi,eax
005D84C7  |.  8B45 D0       mov eax,dword ptr [ebp-30]
005D84CA  |.  46            inc esi
005D84CB  |.  8970 04       mov dword ptr [eax+4],esi
005D84CE  |.  03F6          add esi,esi
005D84D0  |.  56            push esi                                                ; /Arg1
005D84D1  |.  E8 8286F9FF   call 00570B58                                           ; \haruno_.00570B58
005D84D6  |.  8BF8          mov edi,eax
005D84D8  |.  8B45 D0       mov eax,dword ptr [ebp-30]
005D84DB  |.  59            pop ecx
005D84DC  |.  8938          mov dword ptr [eax],edi
005D84DE  |.  8B55 D0       mov edx,dword ptr [ebp-30]
005D84E1  |.  8B4A 04       mov ecx,dword ptr [edx+4]
005D84E4  |.  03C9          add ecx,ecx
005D84E6  |.  51            push ecx                                                ; /Arg3
005D84E7  |.  53            push ebx                                                ; |Arg2
005D84E8  |.  57            push edi                                                ; |Arg1
005D84E9  |.  E8 12C8F9FF   call 00574D00                                           ; \haruno_.00574D00
005D84EE  |.  83C4 0C       add esp,0C
005D84F1  |>  FF4D F0       dec dword ptr [ebp-10]
005D84F4  |.  8B45 FC       mov eax,dword ptr [ebp-4]
005D84F7  |.  85C0          test eax,eax
005D84F9  |.  74 05         jz short 005D8500
005D84FB  |.  E8 2804E3FF   call 00408928                                           ; [haruno_.00408928
005D8500  |>  66:C745 E4 08 mov word ptr [ebp-1C],8
005D8506  |.  66:C745 E4 00 mov word ptr [ebp-1C],0
005D850C  \.  EB 27         jmp short 005D8535
005D850E  /.  837D C8 00    cmp dword ptr [ebp-38],0
005D8512  |.  74 0A         je short 005D851E
005D8514  |.  8B55 C8       mov edx,dword ptr [ebp-38]
005D8517  |.  52            push edx
005D8518  |.  8B0A          mov ecx,dword ptr [edx]
005D851A  |.  FF51 04       call dword ptr [ecx+4]
005D851D  |.  59            pop ecx
005D851E  |>  6A 00         push 0
005D8520  |.  6A 00         push 0
005D8522  |.  E8 0FC4FAFF   call 00584936
005D8527  |.  83C4 08       add esp,8
005D852A  |.  66:C745 E4 10 mov word ptr [ebp-1C],10
005D8530  |.  E8 EDC5FAFF   call 00584B22
005D8535  |>  837D C8 00    cmp dword ptr [ebp-38],0
005D8539  |.  74 0A         je short 005D8545
005D853B  |.  8B45 C8       mov eax,dword ptr [ebp-38]
005D853E  |.  50            push eax
005D853F  |.  8B10          mov edx,dword ptr [eax]
005D8541  |.  FF52 04       call dword ptr [edx+4]


005D8544  |.  59            pop ecx      //Buffer 的 this？
005D8545  |>  8B4D D0       mov ecx,dword ptr [ebp-30]  
005D8548  |.  8B31          mov esi,dword ptr [ecx]   //Buffer->char_buffer
005D854A  |.  33DB          xor ebx,ebx   tjs_int count = 0;
005D854C  |.  8BD6          mov edx,esi   tjs_char *ls = buffer_p;
005D854E  |.  8BC6          mov eax,esi   tjs_char *p = buffer_p;
005D8550  |.  EB 2A         jmp short 005D857C
005D8552  |>  66:8B08       /mov cx,word ptr [eax]
005D8555  |.  66:83F9 0D    |cmp cx,0D
005D8559  |.  74 06         |je short 005D8561
005D855B  |.  66:83F9 0A    |cmp cx,0A
005D855F  |.  75 18         |jne short 005D8579
005D8561  |>  43            |inc ebx
005D8562  |.  66:8338 0D    |cmp word ptr [eax],0D
005D8566  |.  75 0A         |jne short 005D8572
005D8568  |.  66:8378 02 0A |cmp word ptr [eax+2],0A
005D856D  |.  75 03         |jne short 005D8572
005D856F  |.  83C0 02       |add eax,2
005D8572  |>  83C0 02       |add eax,2
005D8575  |.  8BD0          |mov edx,eax
005D8577  |.  EB 03         |jmp short 005D857C
005D8579  |>  83C0 02       |add eax,2
005D857C  |>  66:8338 00    |cmp word ptr [eax],0
005D8580  |.^ 75 D0         \jne short 005D8552
005D8582  |.  3BD0          cmp edx,eax
005D8584  |.  74 01         je short 005D8587
005D8586  |.  43            inc ebx
005D8587  |>  85DB          test ebx,ebx
005D8589  |.  75 1D         jnz short 005D85A8
005D858B  |.  833D 4C787E00 cmp dword ptr [7E784C],0
005D8592  |.  74 07         je short 005D859B
005D8594  |.  A1 4C787E00   mov eax,dword ptr [7E784C]
005D8599  |.  EB 05         jmp short 005D85A0
005D859B  |>  A1 48787E00   mov eax,dword ptr [7E7848]                              ; ASCII "H's"
005D85A0  |>  8B55 CC       mov edx,dword ptr [ebp-34]
005D85A3  |.  E8 B4290A00   call 0067AF5C                                           ; [haruno_.0067AF5C
005D85A8  |>  8BCB          mov ecx,ebx
005D85AA  |.  C1E1 03       shl ecx,3
005D85AD  |.  51            push ecx                                                ; /Arg1
005D85AE  |.  E8 A585F9FF   call 00570B58                                           ; \haruno_.00570B58
005D85B3  |.  59            pop ecx
005D85B4  |.  8B55 D0       mov edx,dword ptr [ebp-30]
005D85B7  |.  8942 08       mov dword ptr [edx+8],eax
005D85BA  |.  8B45 D0       mov eax,dword ptr [ebp-30]
005D85BD  |.  8958 0C       mov dword ptr [eax+0C],ebx
005D85C0  |.  33DB          xor ebx,ebx
005D85C2  |.  8BD6          mov edx,esi
005D85C4  |.  EB 03         jmp short 005D85C9
005D85C6  |>  83C2 02       /add edx,2
005D85C9  |>  66:833A 09    |cmp word ptr [edx],9
005D85CD  |.^ 74 F7         \je short 005D85C6
005D85CF  |.  8BC2          mov eax,edx
005D85D1  |.  EB 56         jmp short 005D8629
005D85D3  |>  66:8B08       /mov cx,word ptr [eax]
005D85D6  |.  66:83F9 0D    |cmp cx,0D
005D85DA  |.  74 06         |je short 005D85E2
005D85DC  |.  66:83F9 0A    |cmp cx,0A
005D85E0  |.  75 44         |jne short 005D8626
005D85E2  |>  8B4D D0       |mov ecx,dword ptr [ebp-30]
005D85E5  |.  8BF0          |mov esi,eax
005D85E7  |.  2BF2          |sub esi,edx
005D85E9  |.  D1FE          |sar esi,1
005D85EB  |.  8B49 08       |mov ecx,dword ptr [ecx+8]
005D85EE  |.  8914D9        |mov dword ptr [ebx*8+ecx],edx
005D85F1  |.  79 03         |jns short 005D85F6
005D85F3  |.  83D6 00       |adc esi,0
005D85F6  |>  8974D9 04     |mov dword ptr [ebx*8+ecx+4],esi
005D85FA  |.  43            |inc ebx
005D85FB  |.  8BC8          |mov ecx,eax
005D85FD  |.  66:8338 0D    |cmp word ptr [eax],0D
005D8601  |.  75 0A         |jne short 005D860D
005D8603  |.  66:8378 02 0A |cmp word ptr [eax+2],0A
005D8608  |.  75 03         |jne short 005D860D
005D860A  |.  83C0 02       |add eax,2
005D860D  |>  83C0 02       |add eax,2
005D8610  |.  8BD0          |mov edx,eax
005D8612  |.  EB 03         |jmp short 005D8617
005D8614  |>  83C2 02       |/add edx,2
005D8617  |>  66:833A 09    ||cmp word ptr [edx],9
005D861B  |.^ 74 F7         |\je short 005D8614
005D861D  |.  8BC2          |mov eax,edx
005D861F  |.  66:C701 0000  |mov word ptr [ecx],0
005D8624  |.  EB 03         |jmp short 005D8629
005D8626  |>  83C0 02       |add eax,2
005D8629  |>  66:8338 00    |cmp word ptr [eax],0
005D862D  |.^ 75 A4         \jne short 005D85D3
005D862F  |.  3BD0          cmp edx,eax
005D8631  |.  74 17         je short 005D864A
005D8633  |.  8B4D D0       mov ecx,dword ptr [ebp-30]
005D8636  |.  2BC2          sub eax,edx
005D8638  |.  D1F8          sar eax,1
005D863A  |.  8B49 08       mov ecx,dword ptr [ecx+8]
005D863D  |.  8914D9        mov dword ptr [ebx*8+ecx],edx
005D8640  |.  79 03         jns short 005D8645
005D8642  |.  83D0 00       adc eax,0
005D8645  |>  8944D9 04     mov dword ptr [ebx*8+ecx+4],eax
005D8649  |.  43            inc ebx
005D864A  |>  8B45 D0       mov eax,dword ptr [ebp-30]
005D864D  |.  8958 0C       mov dword ptr [eax+0C],ebx
005D8650  |.  8B55 D4       mov edx,dword ptr [ebp-2C]
005D8653  |.  64:8915 00000 mov dword ptr fs:[0],edx
005D865A  |.  5F            pop edi
005D865B  |.  5E            pop esi
005D865C  |.  5B            pop ebx
005D865D  |.  8BE5          mov esp,ebp
005D865F  |.  5D            pop ebp
005D8660  \.  C3            retn

*/

