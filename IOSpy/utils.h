#pragma once

#include <fltKernel.h>
#include <dontuse.h>

typedef unsigned char BYTE, * PBYTE; //WDK doesnt contain it LUL

NTSTATUS getRegWStringValue(HANDLE handleRegKey, PCWSTR valueName, PUNICODE_STRING pResultWStr);