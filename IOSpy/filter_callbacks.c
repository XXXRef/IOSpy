#include "filter_callbacks.h"

// Create
//====================================================================================================
FLT_PREOP_CALLBACK_STATUS cbPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
	UNREFERENCED_PARAMETER(FltObjects); //explicitly mark parameter as unreferenced
	UNREFERENCED_PARAMETER(CompletionContext);

	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	NTSTATUS status;
	WCHAR Name[200] = { 0 };
	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
	
	if (NT_SUCCESS(status)) {
		status = FltParseFileNameInformation(FileNameInfo);
		if (NT_SUCCESS(status)) {
			if (FileNameInfo->Name.MaximumLength < 260) {
				RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
				//KdPrint(("Create file: %ws \r\n", Name));
			}
		}
		FltReleaseFileNameInformation(FileNameInfo);
	}

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
	UNREFERENCED_PARAMETER(FltObjects); //explicitly mark parameter as unreferenced
	UNREFERENCED_PARAMETER(CompletionContext);

	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	NTSTATUS status;
	WCHAR Name[200] = { 0 };
	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
	
	if (NT_SUCCESS(status)) {
		status = FltParseFileNameInformation(FileNameInfo);
		if (NT_SUCCESS(status)) {
			if (FileNameInfo->Name.MaximumLength < 260) {
				RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
				//KdPrint(("Create file: %ws \r\n", Name));
			}
		}
		FltReleaseFileNameInformation(FileNameInfo);
	}

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
	UNREFERENCED_PARAMETER(FltObjects); //explicitly mark parameter as unreferenced
	UNREFERENCED_PARAMETER(CompletionContext);

	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	NTSTATUS status;
	WCHAR Name[200] = { 0 };
	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

	if (NT_SUCCESS(status)) {
		status = FltParseFileNameInformation(FileNameInfo);
		if (NT_SUCCESS(status)) {
			if (FileNameInfo->Name.MaximumLength < 260) {
				RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
				//KdPrint(("Create file: %ws \r\n", Name));
			}
		}
		FltReleaseFileNameInformation(FileNameInfo);
	}

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
	UNREFERENCED_PARAMETER(FltObjects); //explicitly mark parameter as unreferenced
	UNREFERENCED_PARAMETER(CompletionContext);

	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	NTSTATUS status;
	WCHAR Name[200] = { 0 };
	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

	if (NT_SUCCESS(status)) {
		status = FltParseFileNameInformation(FileNameInfo);
		if (NT_SUCCESS(status)) {
			if (FileNameInfo->Name.MaximumLength < 260) {
				RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
				//KdPrint(("Create file: %ws \r\n", Name));
			}
		}
		FltReleaseFileNameInformation(FileNameInfo);
	}

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