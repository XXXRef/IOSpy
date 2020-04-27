#include "utils.h"

#include "config.h"

USHORT findLastOf(const UNICODE_STRING *pResultWStr, WCHAR targetChar) {
	USHORT ctr = pResultWStr->Length/sizeof(WCHAR); //TODO this is bullshit cos escape chars exist
	while (ctr!=0) {
		if (pResultWStr->Buffer[ctr-1] == targetChar) { break; }
		--ctr;
	}
	return (ctr == 0) ? (pResultWStr->Length) : (ctr-1);
}

UNICODE_STRING getFileNameFromFilePath(const UNICODE_STRING filePath) {
	//DbgPrint("BEGIN getFileNameFromFilePath. FilePath: %wZ Length:%d MaximumLength:%d",filePath, filePath.Length, filePath.MaximumLength);
	USHORT lastDelimeterPos = findLastOf(&filePath,L'\\');
	//DbgPrint("lastDelimeterPos: %d", lastDelimeterPos);
	if (filePath.Length == lastDelimeterPos) { 
		//DbgPrint("filePath.Length == lastDelimeterPos");
		//DbgPrint("END getFileNameFromFilePath");
		return filePath; 
	}
	UNICODE_STRING fileName;
	fileName.Buffer = filePath.Buffer + lastDelimeterPos + 1;
	fileName.Length = filePath.Length - (lastDelimeterPos + 1)* sizeof(WCHAR);
	fileName.MaximumLength= filePath.MaximumLength - (lastDelimeterPos + 1) * sizeof(WCHAR);
	//DbgPrint("fileName: %wZ", fileName);
	//DbgPrint("END getFileNameFromFilePath");
	return fileName;
}

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
		ExFreePoolWithTag(pKeyInfo, GETREGWSTRINGVALUE_TAG);
		return STATUS_APP_INIT_FAILURE;
	}
	RtlCreateUnicodeString(pResultWStr, (PWSTR)((PBYTE)pKeyInfo + pKeyInfo->DataOffset)); // RtlCreateUnicodeString copies unlike RtlInitUnicodeString
	// The driver is done with the pKeyInfo
	ExFreePoolWithTag(pKeyInfo, GETREGWSTRINGVALUE_TAG);

	return STATUS_SUCCESS;
}

//====================================================================================================
NTSTATUS filterLog(PFLT_FILTER hFilterObj, PCFLT_RELATED_OBJECTS FltObjects, PUNICODE_STRING logFilePath, const WCHAR* pLogBuffer) {
	NTSTATUS status;
	HANDLE hFile;
	PFILE_OBJECT fileObj;
	OBJECT_ATTRIBUTES attr;
	IO_STATUS_BLOCK io_status_block;
	InitializeObjectAttributes(&attr,
		logFilePath,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL);

	status = FltCreateFileEx(hFilterObj, FltObjects->Instance, &hFile, &fileObj, FILE_GENERIC_WRITE | FILE_WRITE_DATA, &attr, &io_status_block, NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_ALERT,
		NULL,
		0,
		IO_IGNORE_SHARE_ACCESS_CHECK);
	
	//status = FltCreateFile(hFilterObj, FltObjects->Instance, &hFile, GENERIC_WRITE, &attr,                        /* ObjectAttributes  */
	//	&io_status_block,             /* IoStatusBlock     */
	//	NULL,                         /* AllocationSize    */
	//	FILE_ATTRIBUTE_NORMAL,        /* FileAttributes    */
	//	FILE_SHARE_READ |             /* ShareAccess       */
	//	FILE_SHARE_WRITE |
	//	FILE_SHARE_DELETE,
	//	FILE_OPEN,                    /* CreateDisposition */
	//	FILE_SYNCHRONOUS_IO_NONALERT, /* CreateOptions     */
	//	NULL,                         /* EaBuffer          */
	//	0,                            /* EaLength          */
	//	0);       /* Flags             */
	//	
	if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
		DbgPrint("{IOSpy} [ERROR] filterLog FltCreateFile failed. Status: %X", status);
#endif
		return status;
	}
	// Write file
	//#define  BUFFER_SIZE 8
	//CHAR     buffer[]="DEADBEEF";
	LARGE_INTEGER ByteOffset;
	ByteOffset.HighPart = -1;
	ByteOffset.LowPart = FILE_WRITE_TO_END_OF_FILE;
	status = FltWriteFile(FltObjects->Instance, fileObj, &ByteOffset, (ULONG)(wcslen(pLogBuffer) * sizeof(WCHAR)), (WCHAR*)pLogBuffer, 0, NULL, NULL, NULL);
	if (!NT_SUCCESS(status)) {
#ifdef _DEBUG
		DbgPrint("{IOSpy} [ERROR] filterLog FltWriteFile failed. Status: %X", status);
#endif
		ObDereferenceObject(fileObj);
		FltClose(hFile);
		return status;
	}

	ObDereferenceObject(fileObj);
	FltClose(hFile);

	return STATUS_SUCCESS;
}