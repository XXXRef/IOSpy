#pragma once

#include <fltKernel.h>
#include <dontuse.h>

typedef struct _IOSPYConfig {
	UNICODE_STRING targetProcessName;
	UNICODE_STRING targetFilePath;
	UNICODE_STRING logFilePath;
} IOSPYConfig, * PIOSPYConfig;

//cant use bool LOL
NTSTATUS getConfigurationData(/*in*/PIOSPYConfig cfg);
