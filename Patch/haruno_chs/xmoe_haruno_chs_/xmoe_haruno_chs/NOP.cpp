#include "stdafx.h"
#include "Nop.h"


void SetNopCode(BYTE* pnop, size_t size)
{
	DWORD oldProtect;
	VirtualProtect((PVOID)pnop, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	for (size_t i = 0; i<size; i++)
	{
		pnop[i] = 0x90;
	}
}

//If - ReadStream
/*
CPU Disasm
Address   Hex dump          Command                                                 Comments
005D845D  |.  837D C8 00    cmp dword ptr [ebp-38],0                                ; if(stream)
005D8461  |.  74 11         je short 005D8474
005D8463  |.  6A 00         push 0
005D8465  |.  8D4D FC       lea ecx,[ebp-4]
005D8468  |.  51            push ecx
005D8469  |.  8B45 C8       mov eax,dword ptr [ebp-38]                              ; 38--Stream
005D846C  |.  50            push eax
005D846D  |.  8B10          mov edx,dword ptr [eax]
005D846F  |.  FF12          call dword ptr [edx]                                    ; stream->Read(tmp, 0);
005D8471  |.  83C4 0C       add esp,0C
005D8474
*/

//CreateStream (This is the first step of Reading scripts
/*
CPU Disasm
Address   Hex dump          Command                                                 Comments
005D8407  |.  8945 C8       mov dword ptr [ebp-38],eax
005D840A  |.  66:C745 E4 08 mov word ptr [ebp-1C],8
005D8410  |.  66:C745 E4 14 mov word ptr [ebp-1C],14
005D8416  |.  B8 08BE7000   mov eax,offset 0070BE08
005D841B  |.  E8 3C08E3FF   call 00408C5C                                           ; TVPCreateTextStreamForRead
005D8420  |.  8945 F8       mov dword ptr [ebp-8],eax
005D8423  |.  FF45 F0       inc dword ptr [ebp-10]
005D8426  |.  8D55 F8       lea edx,[ebp-8]
005D8429  |.  8B45 CC       mov eax,dword ptr [ebp-34]
005D842C  |.  E8 ABBF0200   call 006043DC                                           ; [haruno_.006043DC
005D8431  |.  8945 C8       mov dword ptr [ebp-38],eax
005D8434  |.  FF4D F0       dec dword ptr [ebp-10]
005D8437  |.  8B45 F8       mov eax,dword ptr [ebp-8]
005D843A  |.  85C0          test eax,eax
005D843C  |.  74 05         jz short 005D8443
005D843E  |.  E8 E504E3FF   call 00408928                                           ; [haruno_.00408928
005D8443  |>  66:C745 E4 08 mov word ptr [ebp-1C],8
005D8449  |.  66:C745 E4 2C mov word ptr [ebp-1C],2C
005D844F  |.  33D2          xor edx,edx
005D8451  |.  8955 FC       mov dword ptr [ebp-4],edx
005D8454  |.  FF45 F0       inc dword ptr [ebp-10]
005D8457  |.  66:C745 E4 38 mov word ptr [ebp-1C],38
005D845D
*/

void FuckReadStream()
{
	SetNopCode((PBYTE)0x005D845D, 0x005D8474 - 0x005D845D);
}


void FuckCreateStream()
{
	SetNopCode((PBYTE)0x005D8407, 0x005D845D - 0x005D8407);
}


void FuckWriteBack()
{
	SetNopCode((PBYTE)0x005D8474, 0x005D8550 - 0x005D8474);
}
/*
CPU Disasm
Address   Hex dump          Command                                                 Comments
005D8474 | > \837D FC 00    cmp dword ptr[ebp - 4], 0; ebp - 4 -->ttstr tmp   nop
005D8478 | .  74 1B         je short 005D8495                                 nop
005D847A | .  8B45 FC       mov eax, dword ptr[ebp - 4]                       nop
005D847D | .  85C0          test eax, eax                                     nop
005D847F | .  75 04         jnz short 005D8485                                nop
005D8481 | .  33DB          xor ebx, ebx                                      nop
005D8483 | .EB 16         jmp short 005D849B                                  nop
005D8485 | >  8378 04 00    cmp dword ptr[eax + 4], 0; Buffer  
005D8489 | .  74 05         je short 005D8490
005D848B | .  8B58 04       mov ebx, dword ptr[eax + 4]     

													这个Offset - 4就是文件大小(memcpy)
													去掉UTF16-le的sign 最后两个字节补0就行
													Sign : 0xFF 0xFE
													new Buffer
													Buffer + 0(DWORD) 文件大小
													Buffer + 4(char*) 数据段
*/

/*
CPU Disasm
Address   Hex dump          Command                                                 Comments
005D848E  |. /EB 0B         jmp short 005D849B
005D8490  |> |8D58 08       lea ebx,[eax+8]                                         ; ebx-->Buffer
005D8493  |. |EB 06         jmp short 005D849B
005D8495  |> |8B1D 84D36C00 mov ebx,dword ptr [6CD384]
005D849B  |> \8B45 D0       mov eax,dword ptr [ebp-30]
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
005D84E9  |.  E8 12C8F9FF   call 00574D00                                           ; \haruno_.00574D00, memcpy
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
005D8541  |.  FF52 04       call dword ptr [edx+4]                                  ; call stream + 4
005D8544  |.  59            pop ecx
005D8545  |>  8B4D D0       mov ecx,dword ptr [ebp-30]
005D8548  |.  8B31          mov esi,dword ptr [ecx]
005D854A  |.  33DB          xor ebx,ebx                                             ; conut = 0
005D854C  |.  8BD6          mov edx,esi                                             ; Buffer
005D854E  |.  8BC6          mov eax,esi                                             ; Buffer


End:
CPU Disasm
Address   Hex dump          Command                                                 Comments
005D8550  |. /EB 2A         jmp short 005D857C

*/
