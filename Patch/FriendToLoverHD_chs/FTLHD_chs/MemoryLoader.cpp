#include "MemoryLoader.h"
#include "NtDefine.h"
#include <ntstatus.h>
#include "APIHash.h"
#include "MinHook.h"

BOOL IsValidImage(PVOID ImageBase, ULONG_PTR Flags);

typedef struct
{
	PVOID                       ImageBase;
	PIMAGE_IMPORT_DESCRIPTOR    ImportDescriptor;

	union
	{
		PIMAGE_THUNK_DATA       ThunkData;
		PIMAGE_THUNK_DATA32     ThunkData32;
		PIMAGE_THUNK_DATA64     ThunkData64;
	};

	PCSTR                       DllName;
	PCSTR                       FunctionName;
	ULONG_PTR                   Ordinal;
	PVOID                       Context;

} WALK_IMPORT_TABLE_DATA, *PWALK_IMPORT_TABLE_DATA;

typedef struct
{
	PVOID       ImageBase;
	PULONG      AddressOfFunction;
	PCSTR       DllName;
	ULONG_PTR   Ordinal;
	PCSTR       FunctionName;
	PVOID       Context;
	BOOL        IsForward;

} WALK_EXPORT_TABLE_DATA, *PWALK_EXPORT_TABLE_DATA;

typedef NTSTATUS(*WalkImportTableCallback)(PWALK_IMPORT_TABLE_DATA Data);

#define WalkIATCallbackM(Data) [&] (PWALK_IMPORT_TABLE_DATA Data) -> NTSTATUS

typedef NTSTATUS(*WalkExportTableCallback)(PWALK_EXPORT_TABLE_DATA Data);

#define WalkEATCallbackM(Data) [&] (PWALK_EXPORT_TABLE_DATA Data) -> NTSTATUS


#define LOAD_MEM_DLL_INFO_MAGIC  TAG4('LMDI')

#define LMD_REMOVE_PE_HEADER        0x00000001
#define LMD_REMOVE_IAT              0x00000002
#define LMD_REMOVE_EAT              0x00000004
#define LMD_REMOVE_RES              0x00000008
#define LMD_IGNORE_IAT_DLL_MISSING  0x00000010
#define LMD_MAPPED_DLL              0x10000000

typedef struct LOAD_MEM_DLL_INFO : public TEB_ACTIVE_FRAME
{
	ULONG           Flags;
	PVOID           MappedBase;
	PVOID           MemDllBase;
	SIZE_T          DllBufferSize;
	SIZE_T          ViewSize;
	UNICODE_STRING  Lz32Path;

	union
	{
		HANDLE DllFileHandle;
		HANDLE SectionHandle;
	};

	UNICODE_STRING  MemDllFullPath;

	API_POINTER(NtQueryAttributesFile)  NtQueryAttributesFile;
	API_POINTER(NtOpenFile)             NtOpenFile;
	API_POINTER(NtCreateSection)        NtCreateSection;
	API_POINTER(NtMapViewOfSection)     NtMapViewOfSection;
	API_POINTER(NtClose)                NtClose;
	API_POINTER(NtQuerySection)         NtQuerySection;
	API_POINTER(LdrLoadDll)             LdrLoadDll;

} LOAD_MEM_DLL_INFO, *PLOAD_MEM_DLL_INFO;

PTEB_ACTIVE_FRAME
FindThreadFrame(
ULONG_PTR Context
)
{
	PTEB_ACTIVE_FRAME Frame;

	Frame = RtlGetFrame();
	while (Frame != NULL && Frame->Context != Context)
		Frame = Frame->Previous;

	return Frame;
}


PLOAD_MEM_DLL_INFO GetLaodMemDllInfo()
{
	return (PLOAD_MEM_DLL_INFO)FindThreadFrame(LOAD_MEM_DLL_INFO_MAGIC);
}


NTSTATUS
NTAPI
LoadMemoryDll_NtQueryAttributesFile(
POBJECT_ATTRIBUTES      ObjectAttributes,
PFILE_BASIC_INFORMATION FileInformation
)
{
	PLOAD_MEM_DLL_INFO MemDllInfo;

	MemDllInfo = GetLaodMemDllInfo();

	if (RtlCompareUnicodeString(ObjectAttributes->ObjectName,
		&MemDllInfo->MemDllFullPath, TRUE) != 0)
	{
		return MemDllInfo->NtQueryAttributesFile(ObjectAttributes, FileInformation);
	}

	return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
LoadMemoryDll_NtOpenFile(
PHANDLE             FileHandle,
ACCESS_MASK         DesiredAccess,
POBJECT_ATTRIBUTES  ObjectAttributes,
PIO_STATUS_BLOCK    IoStatusBlock,
ULONG               ShareAccess,
ULONG               OpenOptions
)
{
	NTSTATUS            Status;
	PLOAD_MEM_DLL_INFO  MemDllInfo;

	MemDllInfo = GetLaodMemDllInfo();
	if (RtlCompareUnicodeString(ObjectAttributes->ObjectName,
		&MemDllInfo->MemDllFullPath, TRUE))
	{
		return MemDllInfo->NtOpenFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions);
	}

	ObjectAttributes->ObjectName = &MemDllInfo->Lz32Path;
	Status = MemDllInfo->NtOpenFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions);
	if (NT_SUCCESS(Status))
	{
		MemDllInfo->DllFileHandle = *FileHandle;
	}

	return Status;
}

NTSTATUS
NTAPI
LoadMemoryDll_NtCreateSection(
PHANDLE             SectionHandle,
ACCESS_MASK         DesiredAccess,
POBJECT_ATTRIBUTES  ObjectAttributes,
PLARGE_INTEGER      MaximumSize,
ULONG               SectionPageProtection,
ULONG               AllocationAttributes,
HANDLE              FileHandle
)
{
	BOOL                IsDllHandle;
	NTSTATUS            Status;
	LARGE_INTEGER       SectionSize;
	PLOAD_MEM_DLL_INFO  MemDllInfo;

	IsDllHandle = FALSE;
	MemDllInfo = GetLaodMemDllInfo();

	if (FileHandle != nullptr)
	{
		if (MemDllInfo->DllFileHandle == FileHandle)
		{
			//            if (MaximumSize == NULL)
			MaximumSize = &SectionSize;

			MaximumSize->QuadPart = MemDllInfo->ViewSize;
			DesiredAccess = SECTION_MAP_READ | SECTION_MAP_WRITE | SECTION_MAP_EXECUTE;
			SectionPageProtection = PAGE_EXECUTE_READWRITE;
			AllocationAttributes = SEC_COMMIT;
			FileHandle = nullptr;
			IsDllHandle = TRUE;
		}
	}

	Status = MemDllInfo->NtCreateSection(
		SectionHandle,
		DesiredAccess,
		ObjectAttributes,
		MaximumSize,
		SectionPageProtection,
		AllocationAttributes,
		FileHandle
		);

	if (!NT_SUCCESS(Status) || !IsDllHandle)
	{
		return Status;
	}

	MemDllInfo->SectionHandle = *SectionHandle;

	return Status;
}


#if !defined(_IA64)
#define MEMORY_PAGE_SIZE (4 * 1024)
#else
#define MEMORY_PAGE_SIZE (8 * 1024)
#endif


NTSTATUS
NTAPI
LoadMemoryDll_NtClose(
HANDLE Handle
)
{
	PLOAD_MEM_DLL_INFO MemDllInfo;

	MemDllInfo = GetLaodMemDllInfo();
	if (Handle != nullptr)
	{
		if (MemDllInfo->DllFileHandle == Handle)
		{
			MemDllInfo->DllFileHandle = nullptr;
		}
		else if (MemDllInfo->SectionHandle == Handle)
		{
			MemDllInfo->SectionHandle = nullptr;
		}
	}

	return MemDllInfo->NtClose(Handle);
}

inline PIMAGE_NT_HEADERS ImageNtHeadersFast(PVOID ImageBase, PULONG_PTR NtHeadersVersion = nullptr)
{
	PIMAGE_NT_HEADERS32 NtHeaders32;

	NtHeaders32 = (PIMAGE_NT_HEADERS32)PtrAdd(ImageBase, ((PIMAGE_DOS_HEADER)ImageBase)->e_lfanew);

	if (NtHeadersVersion != nullptr) switch (NtHeaders32->OptionalHeader.Magic)
	{
	case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
	case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
		*NtHeadersVersion = NtHeaders32->OptionalHeader.Magic;
		break;

	default:
		return nullptr;
	}

	return (PIMAGE_NT_HEADERS)NtHeaders32;
}

inline PIMAGE_NT_HEADERS ImageNtHeaders(PVOID ImageBase, PULONG_PTR NtHeadersVersion = nullptr)
{
	if (!IsValidImage(ImageBase, 0))
		return nullptr;

	return ImageNtHeadersFast(ImageBase, NtHeadersVersion);
}

typedef struct
{
	PVOID                       ImageBase;
	PIMAGE_IMPORT_DESCRIPTOR    ImportDescriptor;
	PVOID                       EndOfImage;
	PVOID                       EndOfTable;
	ULONG_PTR                   SizeOfImage;
	ULONG_PTR                   SizeOfTable;
	ULONG_PTR                   NtHeadersVersion;

} WALK_IMPORT_TABLE_INTERNAL_DATA, *PWALK_IMPORT_TABLE_INTERNAL_DATA;



template<class CallbackRoutine, class CallbackContext, class PIMAGE_THUNK_DATA_TYPE>
inline NTSTATUS WalkImportTableInternal(PWALK_IMPORT_TABLE_INTERNAL_DATA InternalData, CallbackRoutine Callback, CallbackContext Context)
{
	NTSTATUS                    Status;
	PIMAGE_THUNK_DATA_TYPE      OriginalThunk, FirstThunk;
	PIMAGE_IMPORT_DESCRIPTOR    ImportDescriptor;

	ImportDescriptor = InternalData->ImportDescriptor;

	for (; ImportDescriptor->Name != NULL && ImportDescriptor->FirstThunk != NULL; ++ImportDescriptor)
	{
		LONG_PTR DllName;

		if (ImportDescriptor->FirstThunk > InternalData->SizeOfImage)
			continue;

		if (*(PULONG_PTR)PtrAdd(InternalData->ImageBase, ImportDescriptor->FirstThunk) == NULL)
			continue;

		OriginalThunk = (PIMAGE_THUNK_DATA_TYPE)InternalData->ImageBase;
		if (ImportDescriptor->OriginalFirstThunk != NULL)
		{
			OriginalThunk = PtrAdd(OriginalThunk, ImportDescriptor->OriginalFirstThunk);
		}
		else
		{
			OriginalThunk = PtrAdd(OriginalThunk, ImportDescriptor->FirstThunk);
		}

		if (OriginalThunk >= InternalData->EndOfImage)
			continue;

		DllName = PtrAdd((LONG_PTR)InternalData->ImageBase, ImportDescriptor->Name);
		if ((PVOID)DllName >= InternalData->EndOfImage)
			continue;

		FirstThunk = (PIMAGE_THUNK_DATA_TYPE)PtrAdd(InternalData->ImageBase, ImportDescriptor->FirstThunk);
		while (OriginalThunk->u1.AddressOfData != NULL)
		{
			LONG_PTR    FunctionName;
			ULONG_PTR   Ordinal;

			FunctionName = (LONG_PTR)OriginalThunk->u1.AddressOfData;
			if (FunctionName < 0)
			{
				Ordinal = (USHORT)FunctionName;
				FunctionName = NULL;
			}
			else
			{
				Ordinal = IMAGE_INVALID_ORDINAL;
				FunctionName += (LONG_PTR)PtrAdd(InternalData->ImageBase, 2);
			}

			WALK_IMPORT_TABLE_DATA Data;

			Data.ImageBase = InternalData->ImageBase;
			Data.ImportDescriptor = ImportDescriptor;
			Data.ThunkData = (PIMAGE_THUNK_DATA)FirstThunk;
			Data.DllName = (PCSTR)DllName;
			Data.Ordinal = Ordinal;
			Data.FunctionName = (PCSTR)FunctionName;
			Data.Context = (PVOID)(ULONG_PTR)Context;

			Status = Callback(&Data);
			if (Status == STATUS_VALIDATE_CONTINUE)
				break;

			FAIL_RETURN(Status);

			++OriginalThunk;
			++FirstThunk;
		}
	}

	return STATUS_SUCCESS;
}


template<class CallbackRoutine, class CallbackContext>
inline NTSTATUS WalkImportTableT(PVOID ImageBase, CallbackRoutine Callback, CallbackContext Context)
{
	ULONG_PTR                   NtHeadersVersion;
	NTSTATUS                    Status;
	PIMAGE_NT_HEADERS32         NtHeaders32;
	PIMAGE_NT_HEADERS64         NtHeaders64;

	WALK_IMPORT_TABLE_INTERNAL_DATA InternalData;

	if (!IsValidImage(ImageBase, IMAGE_VALID_IMPORT_ADDRESS_TABLE))
		return STATUS_INVALID_IMAGE_FORMAT;

	NtHeaders32 = (PIMAGE_NT_HEADERS32)ImageNtHeaders(ImageBase, &NtHeadersVersion);
	NtHeaders64 = (PIMAGE_NT_HEADERS64)NtHeaders32;

	InternalData.ImageBase = ImageBase;
	InternalData.ImportDescriptor = nullptr;

	switch (NtHeadersVersion)
	{
	case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
		InternalData.ImportDescriptor = PtrAdd(InternalData.ImportDescriptor, NtHeaders32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		InternalData.SizeOfTable = NtHeaders32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		InternalData.SizeOfImage = NtHeaders32->OptionalHeader.SizeOfImage;
		break;

	case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
		InternalData.ImportDescriptor = PtrAdd(InternalData.ImportDescriptor, NtHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		InternalData.SizeOfTable = NtHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		InternalData.SizeOfImage = NtHeaders64->OptionalHeader.SizeOfImage;
		break;

	default:
		return STATUS_INVALID_IMAGE_FORMAT;
	}

	InternalData.ImportDescriptor = PtrAdd(InternalData.ImportDescriptor, ImageBase);
	InternalData.EndOfImage = PtrAdd(ImageBase, InternalData.SizeOfImage);
	InternalData.EndOfTable = PtrAdd(InternalData.ImportDescriptor, InternalData.SizeOfTable);

	switch (NtHeadersVersion)
	{
	case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
		return WalkImportTableInternal<CallbackRoutine, CallbackContext, PIMAGE_THUNK_DATA32>(&InternalData, Callback, Context);

	case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
		return WalkImportTableInternal<CallbackRoutine, CallbackContext, PIMAGE_THUNK_DATA64>(&InternalData, Callback, Context);
	}

	return STATUS_INVALID_IMAGE_FORMAT;
}


NTSTATUS
NTAPI
LoadMemoryDll_NtMapViewOfSection(
HANDLE              SectionHandle,
HANDLE              ProcessHandle,
PVOID              *BaseAddress,
ULONG_PTR           ZeroBits,
SIZE_T              CommitSize,
PLARGE_INTEGER      SectionOffset,
PSIZE_T             ViewSize,
SECTION_INHERIT     InheritDisposition,
ULONG               AllocationType,
ULONG               Win32Protect
)
{
	NTSTATUS                    Status;
	PLOAD_MEM_DLL_INFO          MemDllInfo;
	PIMAGE_DOS_HEADER           DosHeader;
	PIMAGE_NT_HEADERS           NtHeader;
	PIMAGE_SECTION_HEADER       SectionHeader;
	PBYTE                       DllBase, ModuleBase;

	MemDllInfo = GetLaodMemDllInfo();

	if (SectionHandle == nullptr)
		goto CALL_ORIGINAL;

	if (MemDllInfo == nullptr)
		goto CALL_ORIGINAL;

	if (SectionHandle != MemDllInfo->SectionHandle)
		goto CALL_ORIGINAL;

	if (SectionOffset != nullptr)
		SectionOffset->QuadPart = 0;

	*ViewSize = MemDllInfo->ViewSize;
	Status = MemDllInfo->NtMapViewOfSection(
		SectionHandle,
		ProcessHandle,
		BaseAddress,
		0,
		0,
		nullptr,
		ViewSize,
		ViewShare,
		0,
		PAGE_EXECUTE_READWRITE
		);
	if (!NT_SUCCESS(Status))
		return Status;

	MemDllInfo->MappedBase = *BaseAddress;

	ModuleBase = (PBYTE)*BaseAddress;
	DllBase = (PBYTE)MemDllInfo->MemDllBase;
	DosHeader = (PIMAGE_DOS_HEADER)DllBase;
	NtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)DllBase + DosHeader->e_lfanew);

	if (FLAG_ON(MemDllInfo->Flags, LMD_MAPPED_DLL))
	{
		CopyMemory(ModuleBase, DllBase, MemDllInfo->ViewSize);
	}
	else
	{
		//        DosHeader       = (PIMAGE_DOS_HEADER)DllBase;
		//        NtHeader        = (PIMAGE_NT_HEADERS)((ULONG_PTR)DllBase + DosHeader->e_lfanew);
		SectionHeader = (PIMAGE_SECTION_HEADER)((ULONG_PTR)&NtHeader->OptionalHeader + NtHeader->FileHeader.SizeOfOptionalHeader);
		for (ULONG NumberOfSections = NtHeader->FileHeader.NumberOfSections; NumberOfSections; ++SectionHeader, --NumberOfSections)
		{
			CopyMemory(
				ModuleBase + SectionHeader->VirtualAddress,
				DllBase + SectionHeader->PointerToRawData,
				SectionHeader->SizeOfRawData
				);
		}
		CopyMemory(ModuleBase, DllBase, MEMORY_PAGE_SIZE);
	}

	if (FLAG_ON(MemDllInfo->Flags, LMD_IGNORE_IAT_DLL_MISSING))
	{
		WalkImportTableT(ModuleBase,
			WalkIATCallbackM(Data)
		{
			return STATUS_VALIDATE_CONTINUE;
		},
			nullptr
			);
	}

	Status = (ULONG_PTR)ModuleBase != NtHeader->OptionalHeader.ImageBase ? STATUS_IMAGE_NOT_AT_BASE : STATUS_SUCCESS;
	return Status;

CALL_ORIGINAL:
	return MemDllInfo->NtMapViewOfSection(SectionHandle, ProcessHandle, BaseAddress, ZeroBits, CommitSize, SectionOffset, ViewSize, InheritDisposition, AllocationType, Win32Protect);
}


ULONG Nt_GetSystemDirectory(PWCHAR Buffer, ULONG BufferCount)
{
	ULONG Length;
	PLDR_MODULE LdrModule;

	LdrModule = FIELD_BASE(Nt_CurrentPeb()->Ldr->InInitializationOrderModuleList.Flink, LDR_MODULE, InInitializationOrderLinks);
	Length = LdrModule->FullDllName.Length - LdrModule->BaseDllName.Length;

	if (Buffer == NULL)
		return Length / sizeof(WCHAR);

	Length = MIN(Length, BufferCount * sizeof(WCHAR));
	RtlCopyMemory(Buffer, LdrModule->FullDllName.Buffer, Length);
	Length /= sizeof(WCHAR);
	if (Length < BufferCount)
		Buffer[Length] = 0;

	return Length;
}


NTSTATUS
NTAPI
LoadMemoryDll_NtQuerySection(
HANDLE                      SectionHandle,
SECTION_INFORMATION_CLASS   SectionInformationClass,
PVOID                       SectionInformation,
SIZE_T                      Length,
PULONG                      ReturnLength
)
{
	PIMAGE_DOS_HEADER           DosHeader;
	PIMAGE_NT_HEADERS           NtHeaders;
	PIMAGE_OPTIONAL_HEADER      OptionalHeader;
	PLOAD_MEM_DLL_INFO          MemDllInfo;
	SECTION_IMAGE_INFORMATION  *ImageInfo;
	SECTION_BASIC_INFORMATION  *BasicInfo;

	MemDllInfo = GetLaodMemDllInfo();
	if (SectionHandle == nullptr || MemDllInfo->SectionHandle != SectionHandle)
		goto DEFAULT_PROC;

	DosHeader = (PIMAGE_DOS_HEADER)MemDllInfo->MemDllBase;
	NtHeaders = (PIMAGE_NT_HEADERS)((ULONG_PTR)DosHeader + DosHeader->e_lfanew);
	OptionalHeader = &NtHeaders->OptionalHeader;

	switch (SectionInformationClass)
	{
	case SectionBasicInformation:
		BasicInfo = (SECTION_BASIC_INFORMATION *)SectionInformation;
		BasicInfo->BaseAddress = MemDllInfo->MappedBase;
		BasicInfo->Attributes = 0;
		BasicInfo->Size.QuadPart = MemDllInfo->ViewSize;
		break;

	case SectionImageInformation:
		if (ReturnLength != nullptr)
			*ReturnLength = sizeof(*ImageInfo);

		if (Length < sizeof(*ImageInfo))
			return STATUS_BUFFER_TOO_SMALL;

		if (SectionInformation == nullptr)
			break;

		ImageInfo = (SECTION_IMAGE_INFORMATION *)SectionInformation;
		ImageInfo->TransferAddress = (PVOID)((ULONG_PTR)DosHeader + OptionalHeader->AddressOfEntryPoint);
		ImageInfo->ZeroBits = 0;
		ImageInfo->MaximumStackSize = OptionalHeader->SizeOfStackReserve;
		ImageInfo->CommittedStackSize = OptionalHeader->SizeOfStackCommit;
		ImageInfo->SubSystemType = OptionalHeader->Subsystem;
		ImageInfo->SubSystemMinorVersion = OptionalHeader->MinorSubsystemVersion;
		ImageInfo->SubSystemMajorVersion = OptionalHeader->MajorSubsystemVersion;
		ImageInfo->GpValue = 0;
		ImageInfo->ImageCharacteristics = NtHeaders->FileHeader.Characteristics;
		ImageInfo->DllCharacteristics = OptionalHeader->DllCharacteristics;
		ImageInfo->Machine = NtHeaders->FileHeader.Machine;
		ImageInfo->ImageContainsCode = 0; // OptionalHeader->SizeOfCode;
		ImageInfo->LoaderFlags = OptionalHeader->LoaderFlags;
		ImageInfo->ImageFileSize = (TYPE_OF(ImageInfo->ImageFileSize))MemDllInfo->DllBufferSize;
		ImageInfo->CheckSum = (TYPE_OF(ImageInfo->CheckSum))OptionalHeader->CheckSum;
		break;

	case SectionRelocationInformation:
		if (SectionInformation != nullptr)
			*(PULONG_PTR)SectionInformation = (ULONG_PTR)MemDllInfo->MappedBase - (ULONG_PTR)OptionalHeader->ImageBase;

		if (ReturnLength != nullptr)
			*ReturnLength = sizeof(ULONG_PTR);

		break;

	default:
		goto DEFAULT_PROC;
	}

	return STATUS_SUCCESS;

DEFAULT_PROC:
	return MemDllInfo->NtQuerySection(SectionHandle, SectionInformationClass, SectionInformation, Length, ReturnLength);
}

FORCEINLINE ULONG HashAPI(PCSTR pszName)
{
	ULONG Hash = 0;

	while (*(PBYTE)pszName)
	{
		Hash = _rotl(Hash, 0x0D) ^ *(PBYTE)pszName++;
	}
	return Hash;
}



BOOL ValidateDataDirectory(PIMAGE_DATA_DIRECTORY DataDirectory, ULONG_PTR SizeOfImage)
{
	if (DataDirectory->Size == 0)
		return FALSE;

	if ((ULONG64)DataDirectory->VirtualAddress + DataDirectory->Size > ULONG_MAX)
		return FALSE;

	return DataDirectory->VirtualAddress <= SizeOfImage &&
		DataDirectory->VirtualAddress + DataDirectory->Size <= SizeOfImage;
}


BOOL IsValidImage(PVOID ImageBase, ULONG_PTR Flags)
{
	PVOID                       Base, End, EndOfImage;
	ULONG_PTR                   Size, SizeOfImage;
	PIMAGE_DOS_HEADER           DosHeader;
	PIMAGE_NT_HEADERS           NtHeader;
	PIMAGE_NT_HEADERS64         NtHeader64;
	PIMAGE_DATA_DIRECTORY       DataDirectory, Directory;
	PIMAGE_IMPORT_DESCRIPTOR    ImportDescriptor;

	DosHeader = (PIMAGE_DOS_HEADER)ImageBase;
	if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return FALSE;

	NtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)ImageBase + DosHeader->e_lfanew);
	if (NtHeader->Signature != IMAGE_NT_SIGNATURE)
		return FALSE;

	if (Flags == 0)
		return TRUE;

	switch (NtHeader->OptionalHeader.Magic)
	{
	case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
		DataDirectory = NtHeader->OptionalHeader.DataDirectory;
		SizeOfImage = NtHeader->OptionalHeader.SizeOfImage;
		break;

	case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
		NtHeader64 = (PIMAGE_NT_HEADERS64)NtHeader;
		DataDirectory = NtHeader64->OptionalHeader.DataDirectory;
		SizeOfImage = NtHeader64->OptionalHeader.SizeOfImage;
		break;

	default:
		return FALSE;
	}

	if (FLAG_ON(Flags, IMAGE_VALID_IMPORT_ADDRESS_TABLE))
	{
		if (!ValidateDataDirectory(&DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT], SizeOfImage))
			return FALSE;

		ImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((ULONG_PTR)ImageBase + DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		if (ImportDescriptor->Name == 0)
			return FALSE;
	}

	if (FLAG_ON(Flags, IMAGE_VALID_EXPORT_ADDRESS_TABLE))
	{
		PIMAGE_EXPORT_DIRECTORY ExportDirectory;

		if (!ValidateDataDirectory(&DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT], SizeOfImage))
			return FALSE;

		ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((ULONG_PTR)ImageBase + DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		if (ExportDirectory->AddressOfFunctions >= SizeOfImage)
			return FALSE;
	}

	if (FLAG_ON(Flags, IMAGE_VALID_RELOC))
	{
		if (!ValidateDataDirectory(&DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC], SizeOfImage))
			return FALSE;
	}

	if (FLAG_ON(Flags, IMAGE_VALID_RESOURCE))
	{
		if (!ValidateDataDirectory(&DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE], SizeOfImage))
			return FALSE;
	}

	return TRUE;
}

PVOID EATLookupRoutineByHashNoFix64(PVOID ImageBase, ULONG_PTR Hash)
{
	PIMAGE_DOS_HEADER           DosHeader;
	PIMAGE_NT_HEADERS64         NtHeaders;
	PIMAGE_EXPORT_DIRECTORY     ExportDirectory;
	ULONG_PTR                   NumberOfNames;
	PULONG                      AddressOfFuntions;
	PULONG                      AddressOfNames;
	PUSHORT                     AddressOfNameOrdinals;

	if (Hash != 0 && !IsValidImage(ImageBase, IMAGE_VALID_EXPORT_ADDRESS_TABLE))
		return nullptr;

	DosHeader = (PIMAGE_DOS_HEADER)ImageBase;
	NtHeaders = (PIMAGE_NT_HEADERS64)((ULONG_PTR)ImageBase + DosHeader->e_lfanew);

	if (Hash == 0)
	{
		return &NtHeaders->OptionalHeader.AddressOfEntryPoint;
	}

	ExportDirectory = nullptr;
	ExportDirectory = PtrAdd(ExportDirectory, ImageBase);
	ExportDirectory = PtrAdd(ExportDirectory, NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	NumberOfNames = ExportDirectory->NumberOfNames;
	AddressOfFuntions = PtrAdd((PULONG)ImageBase, (ULONG_PTR)ExportDirectory->AddressOfFunctions);
	AddressOfNames = PtrAdd((PULONG)ImageBase, (ULONG_PTR)ExportDirectory->AddressOfNames);
	AddressOfNameOrdinals = PtrAdd((PUSHORT)ImageBase, (ULONG_PTR)ExportDirectory->AddressOfNameOrdinals);

	do
	{
		if (!(HashAPI(PtrAdd((PCSTR)(ImageBase), *AddressOfNames)) ^ Hash))
		{
			return AddressOfFuntions + *AddressOfNameOrdinals;
		}

		++AddressOfNameOrdinals;
		++AddressOfNames;
	} while (--NumberOfNames);

	return nullptr;
}


PVOID EATLookupRoutineByHashNoFix(PVOID ImageBase, ULONG_PTR Hash)
{
	PIMAGE_DOS_HEADER           DosHeader;
	PIMAGE_NT_HEADERS           NtHeaders;
	PIMAGE_EXPORT_DIRECTORY     ExportDirectory;
	ULONG_PTR                   NumberOfNames;
	PULONG_PTR                  AddressOfFuntions;
	PCSTR                      *AddressOfNames;
	PUSHORT                     AddressOfNameOrdinals;

	if (Hash != 0 && !IsValidImage(ImageBase, IMAGE_VALID_EXPORT_ADDRESS_TABLE))
		return nullptr;

	DosHeader = (PIMAGE_DOS_HEADER)ImageBase;
	NtHeaders = (PIMAGE_NT_HEADERS)((ULONG_PTR)ImageBase + DosHeader->e_lfanew);

	switch (NtHeaders->OptionalHeader.Magic)
	{
	case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
		break;

	case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
		return EATLookupRoutineByHashNoFix64(ImageBase, Hash);
	}

	if (Hash == 0)
	{
		return &NtHeaders->OptionalHeader.AddressOfEntryPoint;
	}

	ExportDirectory = nullptr;
	ExportDirectory = PtrAdd(ExportDirectory, ImageBase);
	ExportDirectory = PtrAdd(ExportDirectory, NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	NumberOfNames = ExportDirectory->NumberOfNames;
	AddressOfFuntions = PtrAdd((PULONG_PTR)ImageBase, ExportDirectory->AddressOfFunctions);
	AddressOfNames = PtrAdd((PCSTR *)ImageBase, ExportDirectory->AddressOfNames);
	AddressOfNameOrdinals = PtrAdd((PUSHORT)ImageBase, ExportDirectory->AddressOfNameOrdinals);

	do
	{
		if (!(HashAPI(PtrAdd((PCSTR)(ImageBase), *AddressOfNames)) ^ Hash))
		{
			return AddressOfFuntions + *AddressOfNameOrdinals;
		}

		++AddressOfNameOrdinals;
		++AddressOfNames;
	} while (--NumberOfNames);

	return nullptr;
}



PVOID __fastcall EATLookupRoutineByHashPNoFix(PVOID ImageBase, ULONG_PTR Hash)
{
	PVOID Pointer;

	Pointer = EATLookupRoutineByHashNoFix(ImageBase, Hash);
	if (Pointer != nullptr)
		Pointer = PtrAdd(ImageBase, *(PULONG)Pointer);

	return Pointer;
}

NTSTATUS WINAPI
LoadDllFromMemory(
PVOID           DllBuffer,
ULONG           DllBufferSize,
PUNICODE_STRING ModuleFileName,
PVOID*          ModuleHandle,
ULONG           Flags
)
{
	NTSTATUS            Status;
	ULONG               Length;
	PVOID               ModuleBase, ShadowNtdll;
	PLDR_MODULE        Ntdll;
	LOAD_MEM_DLL_INFO   MemDllInfo;
	PIMAGE_DOS_HEADER   DosHeader;
	PIMAGE_NT_HEADERS   NtHeader;
	WCHAR               Lz32DosPath[MAX_PATH];

	API_POINTER(NtQueryAttributesFile)  NtQueryAttributesFile;
	API_POINTER(NtOpenFile)             NtOpenFile;
	API_POINTER(NtCreateSection)        NtCreateSection;
	API_POINTER(NtMapViewOfSection)     NtMapViewOfSection;
	API_POINTER(NtClose)                NtClose;
	API_POINTER(NtQuerySection)         NtQuerySection;
	API_POINTER(LdrLoadDll)             LdrLoadDll;

	Ntdll = GetNtdllLdrModule();

	*(PVOID *)&NtQueryAttributesFile = EATLookupRoutineByHashPNoFix(GetNtdllHandle(), NTDLL_NtQueryAttributesFile);
	*(PVOID *)&NtOpenFile = EATLookupRoutineByHashPNoFix(GetNtdllHandle(), NTDLL_NtOpenFile);
	*(PVOID *)&NtCreateSection = EATLookupRoutineByHashPNoFix(GetNtdllHandle(), NTDLL_NtCreateSection);
	*(PVOID *)&NtMapViewOfSection = EATLookupRoutineByHashPNoFix(GetNtdllHandle(), NTDLL_NtMapViewOfSection);
	*(PVOID *)&NtClose = EATLookupRoutineByHashPNoFix(GetNtdllHandle(), NTDLL_NtClose);
	*(PVOID *)&NtQuerySection = EATLookupRoutineByHashPNoFix(GetNtdllHandle(), NTDLL_NtQuerySection);
	*(PVOID *)&LdrLoadDll = EATLookupRoutineByHashPNoFix(GetNtdllHandle(), NTDLL_LdrLoadDll);

	ZeroMemory(&MemDllInfo, sizeof(MemDllInfo));
	MemDllInfo.Context = LOAD_MEM_DLL_INFO_MAGIC;
	RtlPushFrame(&MemDllInfo);

	Status = STATUS_UNSUCCESSFUL;
	do
	{
		if (!RtlDosPathNameToNtPathName_U(ModuleFileName->Buffer,
			&MemDllInfo.MemDllFullPath, nullptr, nullptr))
			break;

		Length = Nt_GetSystemDirectory(Lz32DosPath, _countof(Lz32DosPath));

		*(PULONG64)(Lz32DosPath + Length) = TAG4W('lz32');
		*(PULONG64)(Lz32DosPath + Length + 4) = TAG4W('.dll');
		Lz32DosPath[Length + 8] = 0;
		if (!RtlDosPathNameToNtPathName_U(Lz32DosPath,
			&MemDllInfo.Lz32Path, nullptr, nullptr))
			break;

		MemDllInfo.Flags = Flags;
		MemDllInfo.MemDllBase = DllBuffer;
		MemDllInfo.DllBufferSize = DllBufferSize;
		DosHeader = (PIMAGE_DOS_HEADER)DllBuffer;
		NtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)DllBuffer + DosHeader->e_lfanew);
		MemDllInfo.ViewSize = NtHeader->OptionalHeader.SizeOfImage;

#if 0
		Mp::PATCH_MEMORY_DATA p[] =
		{
			Mp::FunctionJumpVa(NtQueryAttributesFile, LoadMemoryDll_NtQueryAttributesFile, &MemDllInfo.NtQueryAttributesFile),
			Mp::FunctionJumpVa(NtOpenFile, LoadMemoryDll_NtOpenFile, &MemDllInfo.NtOpenFile),
			Mp::FunctionJumpVa(NtCreateSection, LoadMemoryDll_NtCreateSection, &MemDllInfo.NtCreateSection),
			Mp::FunctionJumpVa(NtMapViewOfSection, LoadMemoryDll_NtMapViewOfSection, &MemDllInfo.NtMapViewOfSection),
			Mp::FunctionJumpVa(NtClose, LoadMemoryDll_NtClose, &MemDllInfo.NtClose),
			Mp::FunctionJumpVa(NtQuerySection, LoadMemoryDll_NtQuerySection, &MemDllInfo.NtQuerySection),
		};

#else

		MH_CreateHook(NtQueryAttributesFile, LoadMemoryDll_NtQueryAttributesFile, (PVOID*)&MemDllInfo.NtQueryAttributesFile);
		MH_CreateHook(NtOpenFile, LoadMemoryDll_NtOpenFile, (PVOID*)&MemDllInfo.NtOpenFile);
		MH_CreateHook(NtCreateSection, LoadMemoryDll_NtCreateSection, (PVOID*)&MemDllInfo.NtCreateSection);
		MH_CreateHook(NtMapViewOfSection, LoadMemoryDll_NtMapViewOfSection, (PVOID*)&MemDllInfo.NtMapViewOfSection);
		MH_CreateHook(NtClose, LoadMemoryDll_NtClose, (PVOID*)&MemDllInfo.NtClose);
		MH_CreateHook(NtQuerySection, LoadMemoryDll_NtQuerySection, (PVOID*)&MemDllInfo.NtQuerySection);

		MH_EnableHook(NtQueryAttributesFile);
		MH_EnableHook(NtOpenFile);
		MH_EnableHook(NtCreateSection);
		MH_EnableHook(NtMapViewOfSection);
		MH_EnableHook(NtClose);
		MH_EnableHook(NtQuerySection);
#endif

		//Mp::PatchMemory(p, _countof(p));
		Status = LdrLoadDll(nullptr, 0, ModuleFileName, &ModuleBase);
		//Mp::RestoreMemory(p, _countof(p));

#if 1
		MH_DisableHook(NtQueryAttributesFile);
		MH_DisableHook(NtOpenFile);
		MH_DisableHook(NtCreateSection);
		MH_DisableHook(NtMapViewOfSection);
		MH_DisableHook(NtClose);
		MH_DisableHook(NtQuerySection);

		MH_RemoveHook(NtQueryAttributesFile);
		MH_RemoveHook(NtOpenFile);
		MH_RemoveHook(NtCreateSection);
		MH_RemoveHook(NtMapViewOfSection);
		MH_RemoveHook(NtClose);
		MH_RemoveHook(NtQuerySection);
#endif
		if (!NT_SUCCESS(Status) && FLAG_OFF(Flags, LMD_MAPPED_DLL))
		{
			break;
		}

		if (ModuleHandle != nullptr)
			*ModuleHandle = (HANDLE)ModuleBase;
	} while (0);
	RtlPopFrame(&MemDllInfo);
	RtlFreeUnicodeString(&MemDllInfo.MemDllFullPath);
	RtlFreeUnicodeString(&MemDllInfo.Lz32Path);

	//UnloadPeImage(ShadowNtdll);

	return Status;
}
