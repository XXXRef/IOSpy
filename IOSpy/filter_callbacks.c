#include "filter_callbacks.h"

#include "utils.h"
#include "init.h"
#include "config.h"

#include <ntstrsafe.h>
#include <Ntddk.h>
#include <Ntifs.h>
#include <Wdm.h>

extern UCHAR* PsGetProcessImageFileName(IN PEPROCESS Process); //undocumented
//extern HANDLE PsGetThreadId(_In_ PETHREAD Thread);

extern PFLT_FILTER hFilterObj;
extern IOSPYConfig IOSpyCfg;

//====================================================================================================
FLT_PREOP_CALLBACK_STATUS cbPreHandler(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
	UNREFERENCED_PARAMETER(FltObjects);
	
	NTSTATUS status;
	
	// We retrieve name information in "pre" for every operation except IRP_MJ_CREATE
	// For IRP_MJ_CREATE, we do it in post
	if (IRP_MJ_CREATE == Data->Iopb->MajorFunction) { 
		return FLT_PREOP_SUCCESS_WITH_CALLBACK; 
	}
	//Get file name
	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
	if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
		DbgPrint("{IOSpy} [ERROR] cbPreHandler FltGetFileNameInformation failed. Status: %X ", status);
#endif
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}
	status = FltParseFileNameInformation(FileNameInfo);
	if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
		DbgPrint("{IOSpy} [ERROR] cbPreHandler FltParseFileNameInformation failed. Status: %X", status);
#endif
		FltReleaseFileNameInformation(FileNameInfo);
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}
	//Pass name to postop callback
	WCHAR* Name=ExAllocatePoolWithTag(NonPagedPool, (SIZE_T)FileNameInfo->Name.Length + 2, CFG_TAG);
	if (Name == NULL) {
		FltReleaseFileNameInformation(FileNameInfo);
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}
	RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.Length);
	Name[FileNameInfo->Name.Length/sizeof(WCHAR)] = L'\0';
	FltReleaseFileNameInformation(FileNameInfo);
#ifdef _DEBUG
	DbgPrint("{IOSpy} [INFO] cbPreHandler MJ:%d Name:%ws", Data->Iopb->MajorFunction, Name);
#endif
	*CompletionContext = (PVOID)Name;
	
	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

//====================================================================================================
FLT_POSTOP_CALLBACK_STATUS cbPostHandler(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags) {
	UNREFERENCED_PARAMETER(Flags);
	
	NTSTATUS status;
	WCHAR* Name;

	//if IRP_MJ_CREATE => get file name in postop callback
	if (IRP_MJ_CREATE == Data->Iopb->MajorFunction) {
		//CompletionContext==0 => no need to free
		PFLT_FILE_NAME_INFORMATION FileNameInfo;
		status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
		if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
			DbgPrint("{IOSpy} [ERROR] cbPostHandler FltGetFileNameInformation failed. MJ=%d Status: %X ", Data->Iopb->MajorFunction, status);
#endif
			return FLT_POSTOP_FINISHED_PROCESSING;
		}
		status = FltParseFileNameInformation(FileNameInfo);
		if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
			DbgPrint("{IOSpy} [ERROR] cbPostHandler FltParseFileNameInformation failed. MJ=%d Status: %X", Data->Iopb->MajorFunction, status);
#endif
			FltReleaseFileNameInformation(FileNameInfo);
			return FLT_POSTOP_FINISHED_PROCESSING;
		}
		Name = ExAllocatePoolWithTag(NonPagedPool, (SIZE_T)FileNameInfo->Name.Length + 2, CFG_TAG);
		if (Name == NULL) {
			FltReleaseFileNameInformation(FileNameInfo);
			return FLT_POSTOP_FINISHED_PROCESSING;
		}
		RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.Length);
		Name[FileNameInfo->Name.Length / sizeof(WCHAR)] = L'\0';
		FltReleaseFileNameInformation(FileNameInfo);
	}else {
		//Get name from CompletionContext
		if (CompletionContext == NULL) {
			return FLT_POSTOP_FINISHED_PROCESSING;
		}

		//UNREFERENCED_PARAMETER(CompletionContext);
		//Name = NULL;
		Name = (WCHAR*)CompletionContext;
	}
	DbgPrint("{IOSpy} [INFO] cbPostHandler MJ=%d Name:%ws", Data->Iopb->MajorFunction, Name);
// Check if its the target case examining process name and file path
	//Check if operation succeeded
	if (TRUE!=NT_SUCCESS(Data->IoStatus.Status)) {
		//DbgPrint("Operation failed: 0x%X", Data->IoStatus.Status);
		ExFreePool(Name);
		return FLT_POSTOP_FINISHED_PROCESSING;
	}
	//Check if right process
	PEPROCESS pRequestorEP;
	ULONG requestorPID = FltGetRequestorProcessId(Data);
	if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)requestorPID, &pRequestorEP))) { //EPROCESS referenced, no mem allocation for new struct
		PUNICODE_STRING pImageFileName;
		SeLocateProcessImageName(pRequestorEP, &pImageFileName);
		UNICODE_STRING requestorProcName = getFileNameFromFilePath(*pImageFileName);
		//DbgPrint("requestorImageFileName: %wZ", requestorProcName);
		BOOLEAN procNameCmpResult = RtlEqualUnicodeString(&requestorProcName, &IOSpyCfg.targetProcessName, TRUE);
		ExFreePool(pImageFileName);
		if (FALSE == procNameCmpResult) {
			//DbgPrint("process name compare: %wZ != %wZ", requestorProcName, IOSpyCfg.targetProcessName);
			ExFreePool(Name);
			return FLT_POSTOP_FINISHED_PROCESSING;
		}
		else {
			//DbgPrint("process name compare: target process name found: %wZ!", &requestorProcName);
		}
	}
	
	// Check if right file name
	POBJECT_NAME_INFORMATION oni;
	status=IoQueryFileDosDeviceName(Data->Iopb->TargetFileObject, &oni);
	if (!NT_SUCCESS(status)) {
		DbgPrint("{IOSpy} [INFO] cbPostHandler IoQueryFileDosDeviceName failed");
		ExFreePool(Name);
		return FLT_POSTOP_FINISHED_PROCESSING;
	}
	if (TRUE!=RtlEqualUnicodeString(&oni->Name,&IOSpyCfg.targetFilePath,TRUE)) {
		// Its not target file
		ExFreePool(oni);
		ExFreePool(Name);
		return FLT_POSTOP_FINISHED_PROCESSING;
	}
	ExFreePool(oni);
	// Get TID
	HANDLE requestorThreadID = PsGetThreadId(Data->Thread);
	// Get FileObject
	PFILE_OBJECT pTargetFileObject = Data->Iopb->TargetFileObject;
	#define MAX_LOG_RECORD_LENGTH 1000
	WCHAR logRecord[MAX_LOG_RECORD_LENGTH];
//Process IO op according to its type
	switch (Data->Iopb->MajorFunction) {
	case IRP_MJ_CREATE: {
		// Get info whether file created or opened
		BOOLEAN flagFileCreated = ((Data->IoStatus.Information & FILE_CREATED) != 0) ? (TRUE) : (FALSE);
		swprintf(logRecord, L"[CREATE_FILE] FilePath:%ws PID:%X TID:%p FileObject:%p flagFileCreated:%d\n\0", Name, requestorPID, requestorThreadID, pTargetFileObject, flagFileCreated);
	} break;
	case IRP_MJ_WRITE: {
		KIRQL curIRQL=KeGetCurrentIrql();
		ULONG amountOfBytesWritten=Data->Iopb->Parameters.Write.Length;
		swprintf(logRecord, L"[WRITE_FILE] FilePath:%ws PID:%X TID:%p FileObject:%p IRQL:%d amountOfBytesWritten:%d\n\0", Name, requestorPID, requestorThreadID,  pTargetFileObject, curIRQL, amountOfBytesWritten);
	} break;
	case IRP_MJ_SET_INFORMATION: {
		FILE_INFORMATION_CLASS fileInfoClass=Data->Iopb->Parameters.SetFileInformation.FileInformationClass;
		swprintf(logRecord, L"[SET_INFO_FILE] FilePath:%ws PID:%X TID:%p FileObject:%p fileInfoClass:%d\n\0", Name, requestorPID, requestorThreadID, pTargetFileObject, fileInfoClass);
	} break;
	case IRP_MJ_CLOSE: {
		swprintf(logRecord, L"[CLOSE_FILE] FilePath:%ws PID:%X TID:%p FileObject:%p\n\0", Name, requestorPID, requestorThreadID, pTargetFileObject);
	} break;
	case IRP_MJ_CLEANUP: {
		swprintf(logRecord, L"[CLEANUP_FILE] FilePath:%ws PID:%X TID:%p FileObject:%p\n\0", Name, requestorPID, requestorThreadID, pTargetFileObject);
	} break;
	default:
		ExFreePool(Name);
		return FLT_POSTOP_FINISHED_PROCESSING;
	}
	ExFreePool(Name);
	// Save data in log file
#ifdef _DEBUG
	DbgPrint("Log msg:%ws", logRecord);
#endif
	filterLog(hFilterObj, FltObjects, &IOSpyCfg.symbolicLogFilePath, logRecord);

	return FLT_POSTOP_FINISHED_PROCESSING;
}