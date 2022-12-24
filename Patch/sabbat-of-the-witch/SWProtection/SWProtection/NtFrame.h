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
VOID SetOwnThread();//���õ�ǰ�̱߳�־
BOOL IsOwnThread();//��⵱ǰ�߳��Ƿ��б�־���ޱ�־����FALSE�з���TRUE
VOID CleanUpOwnThread();//�Ե�ǰ�߳���ձ�־
BOOL _CreateThreadEx(LPTHREAD_START_ROUTINE lpThreadRoutine, LPVOID lparam, PHANDLE pThreadHandle, PDWORD pThreadId);//�������̷߳�װ����
