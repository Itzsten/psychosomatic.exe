#pragma once
#include "psychosomatic.h"

#define PSYCHOSOMATIC_COMMUNICATE WM_USER + 69
#define PSCOM_INJECT_DLL 0x1

typedef struct _COMMUNICATION_DATA {
	CHAR dllPath[MAX_PATH];
	DWORD pid;
	UINT msg;
} COMMUNICATION_DATA, * PCOMMUNICATION_DATA;

VOID WINAPI PsHideFromProcessesPayload(void);