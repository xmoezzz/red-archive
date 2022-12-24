#include <Windows.h>
#include "FriendToLoverHook.h"

HRESULT WINAPI Init(HMODULE hSelf)
{
	if (!FriendToLoverHook::GetGlobalData())
	{
		FriendToLoverHook::ExitMessage(L"��ʼ��ʧ��");
		return S_FALSE;
	}
	return FriendToLoverHook::GetGlobalData()->Init(hSelf);
}

HRESULT WINAPI UnInit(HMODULE hSelf)
{
	if (!FriendToLoverHook::GetGlobalData())
	{
		//ignored some errors
		return S_FALSE;
	}
	return FriendToLoverHook::GetGlobalData()->UnInit(hSelf);
}

