#pragma once
#include "psychosomatic.h"


VOID DebugLine(_In_ LPWSTR dbgLine);
VOID WINAPI ExecuteAudioPayload(_In_ AUDIO_PARAMS AudioPayload, _In_ INT nSeconds);
VOID WINAPI ExecuteShaderPayload(_In_ GDI_PAYLOAD_PARAMS GdiPayload);
VOID WINAPI ExecutePayload(_In_ PAYLOAD_PARAMS Payload);
RGBQUAD COLORREFToRGBQUAD(_In_ COLORREF color);
COLORREF RGBQUADToCOLORREF(_In_ RGBQUAD color);
LONG WINAPI randi(_In_ INT start, _In_ INT end);
COLORREF WINAPI HUE(_In_ INT i);
ULONGLONG PsGenerateSeed();
PS_LAYERED_WINDOW WINAPI PsLayeredWindowNew(_In_ LAYERED_WINDOW_PARAMS lwp);
BOOL WINAPI PsCloseLayeredWindow(_In_ PS_LAYERED_WINDOW lw);
LPSTR* WINAPI CommandLineToArgvA(_In_ LPSTR lpCmdline, _Out_ PINT numargs);
VOID WINAPI MsgError(DWORD err);
DWORD CALLBACK PsAudioThread(LPVOID lpParam);
LPSTR sprintf_auto(char* fmt, ...);

typedef union __SHADER_LPARAM {
	GDI_PAYLOAD_FUNC GdiPayloadList[10];
	LONG ListSize;
	INT seconds;
} SHADER_LPARAM, * PSHADER_LPARAM, * LPSHADER_LPARAM;