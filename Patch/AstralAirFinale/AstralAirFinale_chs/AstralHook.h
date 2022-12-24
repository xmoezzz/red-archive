#pragma once

#include "my.h"
#include "HandleTable.h"

class AstralHook
{
public:
	AstralHook(PVOID Module);
	~AstralHook();

	static AstralHook* GetHook(PVOID Module);
	static AstralHook* GetHook();
	static AstralHook* Handle;


	NTSTATUS Init();
	PCSTR    LookupString(ULONG Index);
	NTSTATUS LookupFile(PCSTR FileName, ULONG& Size, PBYTE& Buffer);

private:
	NTSTATUS CompileScript();

protected:
	NTSTATUS LoadBinaryScriptInternal(PBYTE Buffer, ULONG Size);
private:
	NTSTATUS CompileTextScriptInternal(PCWSTR FileName);
	NTSTATUS InitFileSystem();
	NTSTATUS LookupFileInternal(PCSTR FileName, ULONG& Size, PBYTE& Buffer);
	
public:
	MlHandleTable StringHashMap;
	MlHandleTable FileSystemEntry;
	PVOID         DllModule;
	BOOL          FileSystemInited;
	BOOL          UseTraditionalChinese;
	HWND          MainWindow;
	NtFileDisk    AsFile;
};

