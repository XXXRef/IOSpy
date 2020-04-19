#include "init.h"

#include "config.h"
#include "utils.h"

#include <ntstrsafe.h>

/*
typedef struct _IOSPYConfig {
	UNICODE_STRING targetProcessName;
	UNICODE_STRING targetFilePath;
	UNICODE_STRING logFilePath;
} IOSPYConfig, * PIOSPYConfig;
*/

//====================================================================================================
NTSTATUS getConfigurationData(/*in*/PIOSPYConfig cfg) {
	NTSTATUS status;
	HANDLE handleRegKey = NULL;
	UNICODE_STRING RegistryKeyName;
	OBJECT_ATTRIBUTES ObjectAttributes;

	// Open reg key
	RtlInitUnicodeString(&RegistryKeyName, CFG_REG_CFG_SERVICE_PATH);
	InitializeObjectAttributes(&ObjectAttributes, &RegistryKeyName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwOpenKey(&handleRegKey, KEY_READ, &ObjectAttributes);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	// Get target process name
	status = getRegWStringValue(handleRegKey, CFG_REG_CFG_FIELDNAME_TARGETPROCESSNAME, &cfg->targetProcessName);
	if (!NT_SUCCESS(status)) {
		ZwClose(handleRegKey);
		return status;
	}
#ifdef _DEBUG
	KdPrint(("{IOSpy} [INFO] (getConfigurationData) TARGET PROCESS NAME: %wZ\n", &cfg->targetProcessName));
#endif

	// Get target file path
	status = getRegWStringValue(handleRegKey, CFG_REG_CFG_FIELDNAME_TARGETFILEPATH, &cfg->targetFilePath);
	if (!NT_SUCCESS(status)) {
		ZwClose(handleRegKey);
		return status;
	}
#ifdef _DEBUG
	KdPrint(("{IOSpy} [INFO] (getConfigurationData) TARGET FILE PATH: %wZ\n", &cfg->targetFilePath));
#endif

	// Get log file path
	UNICODE_STRING logFilePath;
	status = getRegWStringValue(handleRegKey, CFG_REG_CFG_FIELDNAME_LOGFILEPATH, &logFilePath);
	if (!NT_SUCCESS(status)) {
		ZwClose(handleRegKey);
		return status;
	}
	ZwClose(handleRegKey);
	//Convert to symbolic path by appending \??\ (\DosDevices\);
	#define SYMBOLIC_PATH_PREFIX L"\\??\\"
	cfg->symbolicLogFilePath.MaximumLength = (USHORT)(wcslen(SYMBOLIC_PATH_PREFIX) + logFilePath.Length+4); //TODO remote disgusting magic 4
	cfg->symbolicLogFilePath.Buffer = ExAllocatePoolWithTag(NonPagedPool, cfg->symbolicLogFilePath.MaximumLength, CFG_TAG);
	if (NULL == cfg->symbolicLogFilePath.Buffer) {
		return STATUS_MEMORY_NOT_ALLOCATED;
	}
	RtlUnicodeStringCopyStringEx(&cfg->symbolicLogFilePath, SYMBOLIC_PATH_PREFIX,NULL, STRSAFE_FILL_BEHIND);
	//RtlAppendUnicodeStringToString(&cfg->symbolicLogFilePath, &logFilePath);
	RtlUnicodeStringCbCatStringN(&cfg->symbolicLogFilePath, logFilePath.Buffer, logFilePath.Length);
	RtlFreeUnicodeString(&logFilePath);
#ifdef _DEBUG
	KdPrint(("{IOSpy} [INFO] getConfigurationData LOG FILE PATH: %wZ\n", &cfg->symbolicLogFilePath));
#endif
	

	return STATUS_SUCCESS;
}