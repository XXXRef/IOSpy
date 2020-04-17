#include "utils.h"

#include "config.h"

//====================================================================================================
NTSTATUS getRegWStringValue(HANDLE handleRegKey, PCWSTR valueName, PUNICODE_STRING pResultWStr) {
#define GETREGWSTRINGVALUE_TAG 'ZLUL' // WARNING can be const var but it takes space. Where are my constexpr? :<

	NTSTATUS status;
	PKEY_VALUE_FULL_INFORMATION  pKeyInfo = NULL;
	UNICODE_STRING               ValueName;
	ULONG                        ulKeyInfoSize = 0;
	ULONG                        ulKeyInfoSizeNeeded = 0;

	RtlInitUnicodeString(&ValueName, valueName); //Doesnt allocate any mem => no need to free UNICODE_STRING
	status = ZwQueryValueKey(handleRegKey, &ValueName, KeyValueFullInformation, pKeyInfo, ulKeyInfoSize, &ulKeyInfoSizeNeeded);
	if ((STATUS_BUFFER_TOO_SMALL != status) && (STATUS_BUFFER_OVERFLOW != status)) {
		return status;
	}
	// Allocate the memory required for the key
	ulKeyInfoSize = ulKeyInfoSizeNeeded;
	pKeyInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, ulKeyInfoSizeNeeded, GETREGWSTRINGVALUE_TAG);
	if (NULL == pKeyInfo) {
		return STATUS_MEMORY_NOT_ALLOCATED;
	}
	RtlZeroMemory(pKeyInfo, ulKeyInfoSize);
	// Get the key data
	status = ZwQueryValueKey(handleRegKey, &ValueName, KeyValueFullInformation, pKeyInfo, ulKeyInfoSize, &ulKeyInfoSizeNeeded);
	if ((status != STATUS_SUCCESS) || (ulKeyInfoSizeNeeded != ulKeyInfoSize) || (NULL == pKeyInfo)) {
		return STATUS_APP_INIT_FAILURE;
	}
	RtlInitUnicodeString(pResultWStr, (PWSTR)((PBYTE)pKeyInfo + pKeyInfo->DataOffset));
	// The driver is done with the pKeyInfo
	ExFreePoolWithTag(pKeyInfo, GETREGWSTRINGVALUE_TAG);

	return STATUS_SUCCESS;
}

