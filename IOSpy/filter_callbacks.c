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
	UNREFERENCED_PARAMETER(CompletionContext);

	NTSTATUS status;

// Check if its the target case examining process name and file path
	PEPROCESS pRequestorEP;
	ULONG requestorPID = FltGetRequestorProcessId(Data);
	if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)requestorPID, &pRequestorEP))) {
		PUNICODE_STRING pImageFileName;
		SeLocateProcessImageName(pRequestorEP, &pImageFileName);
		UNICODE_STRING requestorProcName = getFileNameFromFilePath(*pImageFileName);
		DbgPrint("requestorImageFileName: %wZ", requestorProcName);
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
	// Get PID

	// Get TID


	// Get FileObject

	// Get info whether file created or opened

// Save data in log file
	#define MAX_LOG_RECORD_LENGTH 1000
	WCHAR logRecord[MAX_LOG_RECORD_LENGTH];
	swprintf(logRecord, L"[CREATE_FILE] FilePath:%ws PID:%X TID:%X FileObject:\n\0", Name, requestorPID, 0x5678);
	filterLog(hFilterObj, FltObjects, &IOSpyCfg.symbolicLogFilePath, logRecord);

	return FLT_PREOP_SUCCESS_WITH_CALLBACK; //postop callback will be called
}

//====================================================================================================
FLT_POSTOP_CALLBACK_STATUS cbPostCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags) {
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(CompletionContext);

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