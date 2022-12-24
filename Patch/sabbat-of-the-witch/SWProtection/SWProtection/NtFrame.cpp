#include "NtFrame.h"

T_RtlPushFrame fRtlPushFrame = NULL;
T_RtlGetFrame fRtlGetFrame = NULL;
T_RtlPopFrame fRtlPopFrame = NULL;
#define OWN_THREAD_FLAGS ULONG('MJMJ')
BOOL InitOwnThread()
{
	HMODULE hNtdll = GetModuleHandle(TEXT("ntdll.dll"));
	if (!fRtlGetFrame)
	{
		if (!(fRtlGetFrame = (T_RtlGetFrame)GetProcAddress(hNtdll, "RtlGetFrame"))) return FALSE;
	}
	if (!fRtlPopFrame)
	{
		if (!(fRtlPopFrame = (T_RtlPopFrame)GetProcAddress(hNtdll, "RtlPopFrame"))) return FALSE;
	}
	if (!fRtlPushFrame)
	{
		if (!(fRtlPushFrame = (T_RtlPushFrame)GetProcAddress(hNtdll, "RtlPushFrame"))) return FALSE;
	}
	return TRUE;
}
VOID SetOwnThread()
{
	_OWN_THREAD_CONTEXT_ *pContext = new _OWN_THREAD_CONTEXT_;
	if (pContext)
	{
		RtlZeroMemory(pContext, sizeof(_OWN_THREAD_CONTEXT_));
		pContext->Context = OWN_THREAD_FLAGS;
		pContext->dwThreadId = GetCurrentThreadId();
		pContext->Own = pContext;
		fRtlPushFrame(pContext);
	}
}
VOID CleanUpOwnThread()
{
	PTEB_ACTIVE_FRAME Frame;
	if (!InitOwnThread())
	{
		return;
	}
	Frame = fRtlGetFrame();
	while (Frame != NULL && Frame->Context != OWN_THREAD_FLAGS)
		Frame = Frame->Previous;
	if (Frame)
	{
		_OWN_THREAD_CONTEXT_ *pCtx = (_OWN_THREAD_CONTEXT_ *)Frame;
		pCtx = pCtx->Own;
		fRtlPopFrame(Frame);
		delete pCtx;
	}
	return;
}
DWORD WINAPI _ThreadRoutineEx(LPVOID lparam)
{
	DWORD dwRet = 0;
	PTHREADPARAM pThread;
	SetOwnThread();
	pThread = (PTHREADPARAM)lparam;
	if (pThread&&pThread->Routine)
	{
		dwRet = pThread->Routine(pThread->Context);
	}
	CleanUpOwnThread();
	return dwRet;
}

BOOL _CreateThreadEx(LPTHREAD_START_ROUTINE lpThreadRoutine, LPVOID lparam, PHANDLE pThreadHandle, PDWORD pThreadId)
{
	HANDLE hThread;
	PTHREADPARAM threadparam;
	if (!InitOwnThread())
	{
		return FALSE;
	}
	threadparam = new THREADPARAM;
	if (threadparam)
	{
		RtlZeroMemory(threadparam, sizeof(THREADPARAM));
		threadparam->Routine = lpThreadRoutine;
		threadparam->Context = lparam;
		hThread = CreateThread(NULL, 0, _ThreadRoutineEx, threadparam, 0, pThreadId);
		if (pThreadHandle)
		{
			*pThreadHandle = hThread;
		}
		if (hThread)
		{
			return TRUE;
		}
	}
	return FALSE;
}
BOOL IsOwnThread()
{
	PTEB_ACTIVE_FRAME Frame;
	if (!InitOwnThread())
	{
		return FALSE;
	}
	Frame = fRtlGetFrame();
	while (Frame != NULL && Frame->Context != OWN_THREAD_FLAGS)
		Frame = Frame->Previous;
	if (Frame)
	{
		_OWN_THREAD_CONTEXT_ *pContext = (_OWN_THREAD_CONTEXT_ *)Frame;
		if (pContext->dwThreadId == GetCurrentThreadId())
		{
			return TRUE;
		}
	}
	return FALSE;
}
