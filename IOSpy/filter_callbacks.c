#include "filter_callbacks.h"

#include "utils.h"
#include "init.h"

#include <ntstrsafe.h>
#include <ntddk.h>

extern UCHAR* PsGetProcessImageFileName(IN PEPROCESS Process); //undocumented

extern PFLT_FILTER hFilterObj;
extern IOSPYConfig IOSpyCfg;

// Create
//====================================================================================================
FLT_PREOP_CALLBACK_STATUS cbPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);
	/*
	NTSTATUS status;
// Check if its the target case examining process name and file path
	//Check if operation succeeded
	if (!NT_SUCCESS(Data->IoStatus.Status)) {
		DbgPrint("Operation failed: 0x%X", Data->IoStatus.Status);
		return FLT_PREOP_SUCCESS_WITH_CALLBACK;
	}
	//Check if right process
	PEPROCESS pRequestorEP;
	ULONG requestorPID = FltGetRequestorProcessId(Data);
	if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)requestorPID, &pRequestorEP))) {
		PUNICODE_STRING pImageFileName;
		SeLocateProcessImageName(pRequestorEP, &pImageFileName);
		UNICODE_STRING requestorProcName = getFileNameFromFilePath(*pImageFileName);
		DbgPrint("requestorImageFileName: %wZ", requestorProcName);
		if (FALSE==RtlEqualUnicodeString(&requestorProcName,&IOSpyCfg.targetProcessName,TRUE)) {
			//DbgPrint("process name compare: %wZ != %wZ", requestorProcName, IOSpyCfg.targetProcessName);
			return FLT_PREOP_SUCCESS_WITH_CALLBACK;
		}else {
			DbgPrint("process name compare: target process name found: %wZ!", &requestorProcName);
		}
	}

//Acquire data
	// Get file name

	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	#define MAX_FILE_PATH_LENGTH 600
	WCHAR Name[MAX_FILE_PATH_LENGTH];
	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
	if (!NT_SUCCESS(status)) {
		DbgPrint("{IOSpy} [ERROR] cbPreCreate FltGetFileNameInformation failed. Status: %X", status);
		return FLT_PREOP_SUCCESS_WITH_CALLBACK;
	}
	status = FltParseFileNameInformation(FileNameInfo);
	if (!NT_SUCCESS(status)) {
		DbgPrint("{IOSpy} [ERROR] cbPreCreate FltParseFileNameInformation failed. Status: %X", status);
		FltReleaseFileNameInformation(FileNameInfo);
		return FLT_PREOP_SUCCESS_WITH_CALLBACK;
	}
	//TODO Allocate mem for name
	if (FileNameInfo->Name.MaximumLength < MAX_FILE_PATH_LENGTH) {
		RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
		Name[FileNameInfo->Name.MaximumLength] = L'\0';
		//KdPrint(("Create file: %ws \r\n", Name));
	}
	FltReleaseFileNameInformation(FileNameInfo);
	DbgPrint("[CREATE_FILE] FilePath: %ws PID: %X TID :%X FileObject:<> \n\0", Name, requestorPID, 0x5678);
	// Get PID

	// Get TID


	// Get FileObject

	// Get info whether file created or opened
	BOOLEAN flagFileCreated = (Data->IoStatus.Information & FILE_CREATED == 0) ? () : ();

// Save data in log file
	#define MAX_LOG_RECORD_LENGTH 1000
	WCHAR logRecord[MAX_LOG_RECORD_LENGTH];
	swprintf(logRecord, L"[CREATE_FILE] FilePath:%ws PID:%X TID:%X FileObject:\n\0", Name, requestorPID, 0x5678);
	filterLog(hFilterObj, FltObjects, &IOSpyCfg.symbolicLogFilePath, logRecord);
	*/
	return FLT_PREOP_SUCCESS_WITH_CALLBACK; //postop callback will be called
}

//====================================================================================================
FLT_POSTOP_CALLBACK_STATUS cbPostCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags) {
	UNREFERENCED_PARAMETER(CompletionContext);
	UNREFERENCED_PARAMETER(Flags);

	NTSTATUS status;
	// Check if its the target case examining process name and file path
		//Check if operation succeeded
	if (!NT_SUCCESS(Data->IoStatus.Status)) {
		DbgPrint("Operation failed: 0x%X", Data->IoStatus.Status);
		return FLT_POSTOP_FINISHED_PROCESSING;
	}
	//Check if right process
	PEPROCESS pRequestorEP;
	ULONG requestorPID = FltGetRequestorProcessId(Data);
	if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)requestorPID, &pRequestorEP))) {
		PUNICODE_STRING pImageFileName;
		SeLocateProcessImageName(pRequestorEP, &pImageFileName);
		UNICODE_STRING requestorProcName = getFileNameFromFilePath(*pImageFileName);
		DbgPrint("requestorImageFileName: %wZ", requestorProcName);
		if (FALSE == RtlEqualUnicodeString(&requestorProcName, &IOSpyCfg.targetProcessName, TRUE)) {
			//DbgPrint("process name compare: %wZ != %wZ", requestorProcName, IOSpyCfg.targetProcessName);
			return FLT_POSTOP_FINISHED_PROCESSING;
		}
		else {
			DbgPrint("process name compare: target process name found: %wZ!", &requestorProcName);
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
	
	// Get PID

	// Get TID


	// Get FileObject

	// Get info whether file created or opened
	BOOLEAN flagFileCreated = ((Data->IoStatus.Information & FILE_CREATED) != 0) ? (TRUE) : (FALSE);

	DbgPrint("[CREATE_FILE] FilePath: %ws PID: %X TID :%X flagFileCreated:%d FileObject:\n\0", Name, requestorPID, 0x5678, flagFileCreated);
	// Save data in log file
#define MAX_LOG_RECORD_LENGTH 1000
	WCHAR logRecord[MAX_LOG_RECORD_LENGTH];
	swprintf(logRecord, L"[CREATE_FILE] FilePath:%ws PID:%X TID:%X flagFileCreated:%d FileObject:\n\0", Name, requestorPID, 0x5678, flagFileCreated);
	filterLog(hFilterObj, FltObjects, &IOSpyCfg.symbolicLogFilePath, logRecord);

	return FLT_POSTOP_FINISHED_PROCESSING;
}

//Write
//====================================================================================================
FLT_PREOP_CALLBACK_STATUS cbPreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(FltObjects); //explicitly mark parameter as unreferenced
	UNREFERENCED_PARAMETER(CompletionContext);

	return FLT_PREOP_SUCCESS_WITH_CALLBACK; //postop callback will be called
}

//====================================================================================================
FLT_POSTOP_CALLBACK_STATUS cbPostWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags) {
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(CompletionContext);

	return FLT_POSTOP_FINISHED_PROCESSING;
}

//Set info
//====================================================================================================
FLT_PREOP_CALLBACK_STATUS cbPreSetInfo(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(FltObjects); //explicitly mark parameter as unreferenced
	UNREFERENCED_PARAMETER(CompletionContext);

	return FLT_PREOP_SUCCESS_WITH_CALLBACK; //postop callback will be called
}

//====================================================================================================
FLT_POSTOP_CALLBACK_STATUS cbPostSetInfo(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags) {
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(CompletionContext);

	return FLT_POSTOP_FINISHED_PROCESSING;
}

//Close
//====================================================================================================
FLT_PREOP_CALLBACK_STATUS cbPreClose(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(FltObjects); //explicitly mark parameter as unreferenced
	UNREFERENCED_PARAMETER(CompletionContext);

	return FLT_PREOP_SUCCESS_WITH_CALLBACK; //postop callback will be called
}

//====================================================================================================
FLT_POSTOP_CALLBACK_STATUS cbPostClose(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags) {
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(CompletionContext);

	return FLT_POSTOP_FINISHED_PROCESSING;
}