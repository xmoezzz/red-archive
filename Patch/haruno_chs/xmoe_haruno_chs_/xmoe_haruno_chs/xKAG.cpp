#include "stdafx.h"
#include "xKAG.h"

/*
void __stdcall sub_5D8334(struct_v29 *a1, int a2, char a3)
{
	int *v3; // edx@1
	int v4; // eax@3
	const void *v5; // ebx@4
	struct_v29 *v6; // ecx@11
	int v7; // eax@12
	void *v8; // eax@12
	int v9; // edx@15
	const void *Buffer; // ebx@19
	struct_v29 *v11; // ecx@26
	int v12; // eax@27
	void *v13; // eax@27
	int v14; // eax@32
	int v15; // edx@32
	int v16; // ebx@32
	int v17; // esi@32
	int v18; // eax@45
	int i; // edx@48
	int v20; // ebx@48
	int v21; // eax@51
	int v22; // ecx@54
	int v23; // ecx@54
	int v24; // ecx@64
	struct_v29 *result; // eax@65
	char v26; // [sp+0h] [bp-44h]@1
	int stream; // [sp+Ch] [bp-38h]@13
	int v28; // [sp+10h] [bp-34h]@1
	struct_v29 *v29; // [sp+14h] [bp-30h]@1
	int(*v30)(); // [sp+18h] [bp-2Ch]@1
	int(*v31)(); // [sp+1Ch] [bp-28h]@1
	int v32; // [sp+20h] [bp-24h]@1
	char *v33; // [sp+24h] [bp-20h]@1
	__int16 v34; // [sp+28h] [bp-1Ch]@1
	int v35; // [sp+34h] [bp-10h]@1
	int v36; // [sp+3Ch] [bp-8h]@13

	v28 = a2;
	v29 = a1;
	v33 = &v26;
	v34 = 0;
	v35 = 0;
	v3 = (int *)&v30;

	stream = 0;

		if (tmp)
		{
			if (tmp)
			{
				if (tmp->len)
					Buffer = (const void *)tmp->len;
				else
					Buffer = &tmp->str;
			}
			else
			{
				Buffer = 0;
			}
		}


		v11 = v29;
		v29->dword4 = 0;
		if (Buffer)
		{
			v12 = sub_43C6A4(Buffer, v9, v11);
			v29->dword4 = v12 + 1;
			v13 = (void *)operator new__(2 * (v12 + 1));
			v29->dword0 = v13;
			memcpy(v13, Buffer, 2 * v29->dword4);
		}
		--v35;
		if (tmp)
			sub_408928(tmp);
		v34 = 0;
		if (stream)
			(*(void(__cdecl **)(int))(*(_DWORD *)stream + 4))(stream);
	}
	v17 = v29->dword0;
	v16 = 0;
	v15 = v29->dword0;
	v14 = v29->dword0;
	while (*(_WORD *)v14)
	{
		if (*(_WORD *)v14 != 13 && *(_WORD *)v14 != 10)
		{
			v14 += 2;
		}
		else
		{
			++v16;
			if (*(_WORD *)v14 == 13)
			{
				if (*(_WORD *)(v14 + 2) == 10)
					v14 += 2;
			}
			v14 += 2;
			v15 = v14;
		}
	}
	if (v15 != v14)
		++v16;
	if (!v16)
	{
		if (dword_7E784C)
			v18 = dword_7E784C;
		else
			v18 = dword_7E7848;
		sub_67AF5C(v18, v28);
	}
	v29->dword8 = operator new__(8 * v16);
	v29->dwordC = v16;
	v20 = 0;
	for (i = v17; *(_WORD *)i == 9; i += 2)
		;
	v21 = i;
	while (*(_WORD *)v21)
	{
		if (*(_WORD *)v21 != 13 && *(_WORD *)v21 != 10)
		{
			v21 += 2;
		}
		else
		{
			v23 = v29->dword8;
			*(_DWORD *)(v23 + 8 * v20) = i;
			*(_DWORD *)(v23 + 8 * v20++ + 4) = (v21 - i) / 2;
			v22 = v21;
			if (*(_WORD *)v21 == 13)
			{
				if (*(_WORD *)(v21 + 2) == 10)
					v21 += 2;
			}
			for (i = v21 + 2; *(_WORD *)i == 9; i += 2)
				;
			v21 = i;
			*(_WORD *)v22 = 0;
		}
	}
	if (i != v21)
	{
		v24 = v29->dword8;
		*(_DWORD *)(v24 + 8 * v20) = i;
		*(_DWORD *)(v24 + 8 * v20++ + 4) = (v21 - i) / 2;
	}
	result = v29;
	v29->dwordC = v20;
	return result;
}

*/