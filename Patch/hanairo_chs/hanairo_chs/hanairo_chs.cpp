#include <Windows.h>
#include "HanairoHook.h"

HRESULT WINAPI Init(HMODULE hSelf)
{
	if (!HanairoHook::GetGlobalData())
	{
		HanairoHook::ExitMessage(L"³õÊ¼»¯Ê§°Ü");
		return S_FALSE;
	}
	DisableThreadLibraryCalls(hSelf);
	return HanairoHook::GetGlobalData()->Init(hSelf);
}

HRESULT WINAPI UnInit(HMODULE hSelf)
{
	if (!HanairoHook::GetGlobalData())
	{
		//ignored some errors
		return S_FALSE;
	}
	return HanairoHook::GetGlobalData()->UnInit(hSelf);
}
