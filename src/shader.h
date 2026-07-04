#pragma once
#include "psychosomatic.h"

VOID WINAPI GdiShader1(PRGBQUAD prgbsrc, PRGBQUAD prgbdst, HDC hdc, INT t, TIME_T tTimePassed, INT w, INT sh, LPARAM lParam);

VOID WINAPI AudioSequence1(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam);
VOID WINAPI AudioSequence2(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam);
VOID WINAPI AudioSequence3(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam);
VOID WINAPI AudioSequence4(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam);
VOID WINAPI AudioSequence5(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam);
VOID WINAPI AudioSequence6(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam);
VOID WINAPI AudioSequence7(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam);

VOID WINAPI layeredWindowPayload1(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam);
VOID WINAPI layeredWindowPayload2(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam);
VOID WINAPI layeredWindowPayload3(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam);
VOID WINAPI layeredWindowPayload4(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam);
VOID WINAPI layeredWindowPayload5(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam);
VOID WINAPI layeredWindowPayload6(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam);
VOID WINAPI layeredWindowPayload7(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam);

typedef struct _SHADER_DATA {
	INT mode1;
	INT mode2;
} SHADER_DATA;

typedef struct _MANDELBROT_RENDER_DATA {
	TIME_T lastRender;
	HANDLE renderThread;
	DOUBLE zoom;
} MANDELBROT_RENDER_DATA;

typedef struct _SHADER_CALL_PARAMS {
	HDC hdc;
	HBITMAP hbm;
	INT w;
	INT h;
	TIME_T timePassed;
	LPARAM lParam;
} SHADER_CALL_PARAMS, *PSHADER_CALL_PARAMS;

typedef struct __FRACTAL_COLORMAP {
	COLORREF* colormap;
	DWORD dwColormapLength;
} FRACTAL_COLORMAP, *PFRACTAL_COLORMAP;