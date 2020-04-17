#include "init.h"

#include "config.h"
#include "utils.h"

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
	KdPrint(("{IOSpy} [INFO] (getConfigurationData) TARGET PROCESS NAME: %wZ\n", &cfg->targetProcessName));

	// Get target file path
	status = getRegWStringValue(handleRegKey, CFG_REG_CFG_FIELDNAME_TARGETFILEPATH, &cfg->targetFilePath);
	if (!NT_SUCCESS(status)) {
		ZwClose(handleRegKey);
		return status;
	}
	KdPrint(("{IOSpy} [INFO] (getConfigurationData) TARGET FILE PATH: %wZ\n", &cfg->targetFilePath));

	// Get log file path
	status = getRegWStringValue(handleRegKey, CFG_REG_CFG_FIELDNAME_LOGFILEPATH, &cfg->logFilePath);
	if (!NT_SUCCESS(status)) {
		ZwClose(handleRegKey);
		return status;
	}
	KdPrint(("{IOSpy} [INFO] (getConfigurationData) LOG FILE PATH: %wZ\n", &cfg->logFilePath));

	ZwClose(handleRegKey);

	return STATUS_SUCCESS;
}