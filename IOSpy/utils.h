#pragma once

#include <fltKernel.h>
#include <dontuse.h>

typedef unsigned char BYTE, * PBYTE; //WDK doesnt contain it LUL

USHORT findLastOf(const UNICODE_STRING* pResultWStr, WCHAR targetChar);
UNICODE_STRING getFileNameFromFilePath(const UNICODE_STRING filePath);

NTSTATUS  getRegWStringValue(HANDLE handleRegKey, PCWSTR valueName, PUNICODE_STRING pResultWStr);

NTSTATUS filterLog(PFLT_FILTER hFilterObj, PCFLT_RELATED_OBJECTS FltObjects, PUNICODE_STRING logFilePath, const WCHAR* pLogBuffer);

//inline HANDLE GetCurPID(){ return PsGetProcessId(PsGetCurrentProcess()); }
#define GET_CUR_PID() PsGetProcessId(PsGetCurrentProcess()) //may be function but there is no guarantee of inlining => worse performance