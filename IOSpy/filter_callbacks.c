#include "filter_callbacks.h"

#include "utils.h"
#include "init.h"

#include <ntstrsafe.h>
#include <Ntddk.h>
#include <Ntifs.h>
#include <Wdm.h>

extern UCHAR* PsGetProcessImageFileName(IN PEPROCESS Process); //undocumented
//extern HANDLE PsGetThreadId(_In_ PETHREAD Thread);

extern PFLT_FILTER hFilterObj;
extern IOSPYConfig IOSpyCfg;

//====================================================================================================
FLT_POSTOP_CALLBACK_STATUS cbPostHandler(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags) {
	UNREFERENCED_PARAMETER(CompletionContext);
	UNREFERENCED_PARAMETER(Flags);

	NTSTATUS status;
	// Check if its the target case examining process name and file path
		//Check if operation succeeded
	if (!NT_SUCCESS(Data->IoStatus.Status)) {
		//DbgPrint("Operation failed: 0x%X", Data->IoStatus.Status);
		return FLT_POSTOP_FINISHED_PROCESSING;
	}
	//Check if right process
	PEPROCESS pRequestorEP;
	ULONG requestorPID = FltGetRequestorProcessId(Data);
	if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)requestorPID, &pRequestorEP))) {
		PUNICODE_STRING pImageFileName;
		SeLocateProcessImageName(pRequestorEP, &pImageFileName);
		UNICODE_STRING requestorProcName = getFileNameFromFilePath(*pImageFileName);
		//DbgPrint("requestorImageFileName: %wZ", requestorProcName);
		if (FALSE == RtlEqualUnicodeString(&requestorProcName, &IOSpyCfg.targetProcessName, TRUE)) {
			//DbgPrint("process name compare: %wZ != %wZ", requestorProcName, IOSpyCfg.targetProcessName);
			return FLT_POSTOP_FINISHED_PROCESSING;
		}
		else {
			//DbgPrint("process name compare: target process name found: %wZ!", &requestorProcName);
		}
	}

	//Acquire data
		// Get file name

	PFLT_FILE_NAME_INFORMATION FileNameInfo;
#define MAX_FILE_PATH_LENGTH 600
	WCHAR Name[MAX_FILE_PATH_LENGTH];
	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
	if (!NT_SUCCESS(status)) {
		DbgPrint("{IOSpy} [ERROR] cbPostCreate FltGetFileNameInformation failed. Status: %X", status);
		return FLT_POSTOP_FINISHED_PROCESSING;
	}
	status = FltParseFileNameInformation(FileNameInfo);
	if (!NT_SUCCESS(status)) {
		DbgPrint("{IOSpy} [ERROR] cbPostCreate FltParseFileNameInformation failed. Status: %X", status);
		FltReleaseFileNameInformation(FileNameInfo);
		return FLT_POSTOP_FINISHED_PROCESSING;
	}
	//TODO Allocate mem for name
	if (FileNameInfo->Name.MaximumLength < MAX_FILE_PATH_LENGTH) {
		RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
		Name[FileNameInfo->Name.MaximumLength] = L'\0';
		//KdPrint(("Create file: %ws \r\n", Name));
	}
	FltReleaseFileNameInformation(FileNameInfo);

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
	default:
		return FLT_POSTOP_FINISHED_PROCESSING;
	}

	
	// Save data in log file
	DbgPrint("%ws", logRecord);
	filterLog(hFilterObj, FltObjects, &IOSpyCfg.symbolicLogFilePath, logRecord);

	return FLT_POSTOP_FINISHED_PROCESSING;
}