#pragma once
#include <windows.h>
#include <winternl.h>
#include <ntstatus.h>
typedef struct TEB_ACTIVE_FRAME_CONTEXT
{
	/* 0x000 */ ULONG   Flags;
	/* 0x004 */ PSTR    FrameName;

} TEB_ACTIVE_FRAME_CONTEXT, *PTEB_ACTIVE_FRAME_CONTEXT;

typedef struct TEB_ACTIVE_FRAME
{
	/* 0x000 */ ULONG                       Context;  // Flags;
	/* 0x004 */ struct TEB_ACTIVE_FRAME    *Previous;
	/* 0x008 */  PTEB_ACTIVE_FRAME_CONTEXT   pContext;
} TEB_ACTIVE_FRAME, *PTEB_ACTIVE_FRAME;
typedef struct _THREADPARAM_
{
	PVOID Context;
	LPTHREAD_START_ROUTINE Routine;
}THREADPARAM, *PTHREADPARAM;
struct _OWN_THREAD_CONTEXT_ : public TEB_ACTIVE_FRAME
{
	DWORD dwThreadId;
	struct _OWN_THREAD_CONTEXT_ *Own;
};
typedef  VOID(NTAPI *T_RtlPushFrame)(
	PTEB_ACTIVE_FRAME Frame
	);
typedef PTEB_ACTIVE_FRAME(NTAPI *T_RtlGetFrame)(
	VOID
	);

typedef VOID(NTAPI *T_RtlPopFrame)(
	PTEB_ACTIVE_FRAME Frame
	);
#define OWN_THREAD_FLAGS ULONG('Xmoe')
VOID SetOwnThread();//设置当前线程标志
BOOL IsOwnThread();//检测当前线程是否有标志，无标志返回FALSE有返回TRUE
VOID CleanUpOwnThread();//对当前线程清空标志
BOOL _CreateThreadEx(LPTHREAD_START_ROUTINE lpThreadRoutine, LPVOID lparam, PHANDLE pThreadHandle, PDWORD pThreadId);//创建新线程封装函数
