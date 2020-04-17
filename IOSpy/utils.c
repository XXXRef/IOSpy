#include "utils.h"

#include "config.h"

/*
typedef struct _IOSPYConfig {
	UNICODE_STRING targetProcessName;
	UNICODE_STRING targetFilePath;
	UNICODE_STRING logFilePath;
} IOSPYConfig, * PIOSPYConfig;
*/

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

	PKEY_VALUE_FULL_INFORMATION  pKeyInfo = NULL;
	UNICODE_STRING               ValueName;
	ULONG                        ulKeyInfoSize = 0;
	ULONG                        ulKeyInfoSizeNeeded = 0;

// Get target process name
	RtlInitUnicodeString(&ValueName, CFG_REG_CFG_FIELDNAME_TARGETPROCESSNAME);
	status = ZwQueryValueKey(handleRegKey, &ValueName, KeyValueFullInformation, pKeyInfo, ulKeyInfoSize, &ulKeyInfoSizeNeeded);
	if ((STATUS_BUFFER_TOO_SMALL != status) && (STATUS_BUFFER_OVERFLOW != status)) {
		ZwClose(handleRegKey);
		return status;
	}
	// Allocate the memory required for the key
	ulKeyInfoSize = ulKeyInfoSizeNeeded;
	pKeyInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, ulKeyInfoSizeNeeded, CFG_TAG);
	if (NULL == pKeyInfo) {
		ZwClose(handleRegKey);
		return STATUS_MEMORY_NOT_ALLOCATED;
	}
	RtlZeroMemory(pKeyInfo, ulKeyInfoSize);
	// Get the key data
	status = ZwQueryValueKey(handleRegKey, &ValueName, KeyValueFullInformation, pKeyInfo, ulKeyInfoSize, &ulKeyInfoSizeNeeded);
	if ((status != STATUS_SUCCESS) || (ulKeyInfoSizeNeeded != ulKeyInfoSize) || (NULL == pKeyInfo)){
		ZwClose(handleRegKey);
		return STATUS_APP_INIT_FAILURE;
	}
	RtlInitUnicodeString(&cfg->targetProcessName, (PWSTR)((PBYTE)pKeyInfo + pKeyInfo->DataOffset) );
	KdPrint(("{IOSPY} [INFO] __FUNCTION__ TARGET PROCESS NAME: %wZ\n", &cfg->targetProcessName));
	// The driver is done with the pKeyInfo
	ExFreePoolWithTag(pKeyInfo, CFG_TAG);

// Get target file path
	pKeyInfo = NULL; //WARNING mb its unnecessary
	ulKeyInfoSize = 0;
	ulKeyInfoSizeNeeded = 0;
	RtlInitUnicodeString(&ValueName, CFG_REG_CFG_FIELDNAME_TARGETFILEPATH);
	status = ZwQueryValueKey(handleRegKey, &ValueName, KeyValueFullInformation, pKeyInfo, ulKeyInfoSize, &ulKeyInfoSizeNeeded);
	if ((STATUS_BUFFER_TOO_SMALL != status) && (STATUS_BUFFER_OVERFLOW != status)) {
		ZwClose(handleRegKey);
		return status;
	}
	// Allocate the memory required for the key
	ulKeyInfoSize = ulKeyInfoSizeNeeded;
	pKeyInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, ulKeyInfoSizeNeeded, CFG_TAG);
	if (NULL == pKeyInfo) {
		ZwClose(handleRegKey);
		return STATUS_MEMORY_NOT_ALLOCATED;
	}
	RtlZeroMemory(pKeyInfo, ulKeyInfoSize);
	// Get the key data
	status = ZwQueryValueKey(handleRegKey, &ValueName, KeyValueFullInformation, pKeyInfo, ulKeyInfoSize, &ulKeyInfoSizeNeeded);
	if ((status != STATUS_SUCCESS) || (ulKeyInfoSizeNeeded != ulKeyInfoSize) || (NULL == pKeyInfo)) {
		ZwClose(handleRegKey);
		return STATUS_APP_INIT_FAILURE;
	}
	RtlInitUnicodeString(&cfg->targetFilePath, (PWSTR)((PBYTE)pKeyInfo + pKeyInfo->DataOffset));
	KdPrint(("{IOSPY} [INFO] __FUNCTION__ TARGET FILE PATH: %wZ\n", &cfg->targetFilePath));
	// The driver is done with the pKeyInfo
	ExFreePoolWithTag(pKeyInfo, CFG_TAG);

// Get log file path
	pKeyInfo = NULL; //WARNING mb its unnecessary
	ulKeyInfoSize = 0;
	ulKeyInfoSizeNeeded = 0;
	RtlInitUnicodeString(&ValueName, CFG_REG_CFG_FIELDNAME_LOGFILEPATH);
	status = ZwQueryValueKey(handleRegKey, &ValueName, KeyValueFullInformation, pKeyInfo, ulKeyInfoSize, &ulKeyInfoSizeNeeded);
	if ((STATUS_BUFFER_TOO_SMALL != status) && (STATUS_BUFFER_OVERFLOW != status)) {
		ZwClose(handleRegKey);
		return status;
	}
	// Allocate the memory required for the key
	ulKeyInfoSize = ulKeyInfoSizeNeeded;
	pKeyInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, ulKeyInfoSizeNeeded, CFG_TAG);
	if (NULL == pKeyInfo) {
		ZwClose(handleRegKey);
		return STATUS_MEMORY_NOT_ALLOCATED;
	}
	RtlZeroMemory(pKeyInfo, ulKeyInfoSize);
	// Get the key data
	status = ZwQueryValueKey(handleRegKey, &ValueName, KeyValueFullInformation, pKeyInfo, ulKeyInfoSize, &ulKeyInfoSizeNeeded);
	if ((status != STATUS_SUCCESS) || (ulKeyInfoSizeNeeded != ulKeyInfoSize) || (NULL == pKeyInfo)) {
		ZwClose(handleRegKey);
		return STATUS_APP_INIT_FAILURE;
	}
	RtlInitUnicodeString(&cfg->logFilePath, (PWSTR)((PBYTE)pKeyInfo + pKeyInfo->DataOffset));
	KdPrint(("{IOSPY} [INFO] (getConfigurationData) LOG FILE PATH: %wZ\n", &cfg->logFilePath));
	// The driver is done with the pKeyInfo
	ExFreePoolWithTag(pKeyInfo, CFG_TAG);

	ZwClose(handleRegKey);

	return STATUS_SUCCESS;
}