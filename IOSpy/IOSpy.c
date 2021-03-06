/*++

Module Name:

	FsFilter2.c

Abstract:

	This is the main module of the FsFilter2 miniFilter driver.

Environment:

	Kernel mode

--*/

#include <fltKernel.h>
#include <dontuse.h>

#include "init.h"
#include "filter_callbacks.h"

PFLT_FILTER hFilterObj = NULL;
IOSPYConfig IOSpyCfg;

NTSTATUS cbUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {
	UNREFERENCED_PARAMETER(Flags);

	RtlFreeUnicodeString(&IOSpyCfg.symbolicLogFilePath);
	RtlFreeUnicodeString(&IOSpyCfg.targetFilePath);
	RtlFreeUnicodeString(&IOSpyCfg.targetProcessName);
#ifdef _DEBUG
	KdPrint(("{IOSpy} cbUnload Driver unload \r\n"));
#endif
	FltUnregisterFilter(hFilterObj);
	return STATUS_SUCCESS;
}

/*
//The FLT_OPERATION_REGISTRATION structure is used to register operation callback routines.

typedef struct _FLT_OPERATION_REGISTRATION {
	UCHAR                            MajorFunction;
	FLT_OPERATION_REGISTRATION_FLAGS Flags;
	PFLT_PRE_OPERATION_CALLBACK      PreOperation;
	PFLT_POST_OPERATION_CALLBACK     PostOperation;
	PVOID                            Reserved1;
} FLT_OPERATION_REGISTRATION, * PFLT_OPERATION_REGISTRATION;
*/
const FLT_OPERATION_REGISTRATION Callbacks[] = {
	{IRP_MJ_CREATE,0,cbPreHandler, cbPostHandler}, // File create/open operation handlers
	{IRP_MJ_WRITE,0,cbPreHandler, cbPostHandler},				 // File write operation handlers
	{IRP_MJ_SET_INFORMATION,0,cbPreHandler, cbPostHandler},	 // File set_info operation handlers
	{IRP_MJ_CLOSE,0,cbPreHandler, cbPostHandler},
	{IRP_MJ_CLEANUP,0,cbPreHandler, cbPostHandler},
	{IRP_MJ_OPERATION_END}							 // The last element of this array must be {IRP_MJ_OPERATION_END}
};

/*
//Minifilter driver registration structure

typedef struct _FLT_REGISTRATION {
	USHORT                                      Size;
	USHORT                                      Version;
	FLT_REGISTRATION_FLAGS                      Flags;
	const FLT_CONTEXT_REGISTRATION* ContextRegistration;
	const FLT_OPERATION_REGISTRATION* OperationRegistration;
	PFLT_FILTER_UNLOAD_CALLBACK                 FilterUnloadCallback;
	PFLT_INSTANCE_SETUP_CALLBACK                InstanceSetupCallback;
	PFLT_INSTANCE_QUERY_TEARDOWN_CALLBACK       InstanceQueryTeardownCallback;
	PFLT_INSTANCE_TEARDOWN_CALLBACK             InstanceTeardownStartCallback;
	PFLT_INSTANCE_TEARDOWN_CALLBACK             InstanceTeardownCompleteCallback;
	PFLT_GENERATE_FILE_NAME                     GenerateFileNameCallback;
	PFLT_NORMALIZE_NAME_COMPONENT               NormalizeNameComponentCallback;
	PFLT_NORMALIZE_CONTEXT_CLEANUP              NormalizeContextCleanupCallback;
	PFLT_TRANSACTION_NOTIFICATION_CALLBACK      TransactionNotificationCallback;
	PFLT_NORMALIZE_NAME_COMPONENT_EX            NormalizeNameComponentExCallback;
	PFLT_SECTION_CONFLICT_NOTIFICATION_CALLBACK SectionNotificationCallback;
} FLT_REGISTRATION, * PFLT_REGISTRATION;
*/
const FLT_REGISTRATION FilterRegistrationObj = {
	sizeof(FLT_REGISTRATION),
	FLT_REGISTRATION_VERSION, //The revision level of the FLT_REGISTRATION structure. Minifilter drivers must set this member to FLT_REGISTRATION_VERSION
	0,
	NULL,
	Callbacks,
	cbUnload,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

//====================================================================================================
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

#ifdef _DEBUG
	DbgPrint("{IOSpy} [INFO] BEGIN DriverEntry");
#endif

	NTSTATUS status;
	//Acquire configuration data
	status = getConfigurationData(&IOSpyCfg);
	if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
		DbgPrint("{IOSpy} [ERROR] DriverEntry Cant get configuration info");
#endif
		return status;
	}
	
	//Registering in global list of registered minifilter drivers
	//Provide Filter Manager with callbacks/additional minifilter driver info
	status = FltRegisterFilter(DriverObject, &FilterRegistrationObj, &hFilterObj); //IoRegisterFsRegistrationChange as alternative?
	if (!NT_SUCCESS(status)) {
		return status;
	}

	//Begin filtering IO operations
	status = FltStartFiltering(hFilterObj);
	if (!NT_SUCCESS(status)) {
		FltUnregisterFilter(hFilterObj);
		return status;
	}

	return STATUS_SUCCESS;
}
