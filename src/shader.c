#include "psychosomatic.h"

VOID WINAPI GdiShader1(PRGBQUAD prgbsrc, PRGBQUAD prgbdst, HDC hdc, INT t, TIME_T tTimePassed, INT w, INT h, LPARAM lParam) {
	TIME_T vel = (tTimePassed / 30) / 60 + 1;
	INT prevChannel = ((INT)((((tTimePassed/30.0-1)*30.0) / 30) / 60) % 3);
	INT channel = (((tTimePassed / 30)/60) % 3);
	RGBQUAD rgb;
	INT sinX, sinY;
	TIME_T tV = t * vel;

	for (INT x = 0; x < w; x++) {
		for (INT y = 0; y < h; y++) {
			rgb = prgbsrc[y * w + x];
			sinX = abs((INT)(x * sin(y / 100.))) % w;
			sinY = abs((INT)(y * sin(x / 100.))) % h;
			if (prevChannel != channel && t)
				*(prevChannel == 0 ? &rgb.b : (prevChannel == 1 ? &rgb.g : &rgb.r)) = *(channel == 0 ? &rgb.b : (channel == 1 ? &rgb.g : &rgb.r));
			*(channel==0?&rgb.b:(channel==1?&rgb.g:&rgb.r)) = - (INT)((sinX ^ sinY) + tV % 256);
			prgbdst[y * w + x] = rgb;
		}
	}
}

VOID WINAPI AudioSequence1(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam) {
	for (INT t = 0; t < nCount; t++) {
		buffer[t] = (t>>t) * 256;
	}
}

VOID WINAPI layeredWindowPayload1(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam) {
	BITMAP bmp = { 0 };
	if (!GetObjectW(hbm, sizeof(bmp), &bmp)) {
		DebugLine(L"GetObjectW Failed!");
		return;
	}
	SIZE_T imgSz = (SIZE_T)bmp.bmHeight * bmp.bmWidthBytes * bmp.bmPlanes;
	PDWORD imgData = malloc(imgSz); // certified genius moment
	if (!imgData)
		return;
	SHADER_DATA shd = *((SHADER_DATA*)lParam);
	LONG i;
	INT rData = shd.mode1 * 8;
	INT pDt = shd.mode2 ? 1 : -1;

	GetBitmapBits(hbm, (LONG)imgSz, imgData);
	for (INT x = 0; x < w; x++) {
		for (INT y = 0; y < h; y++) {
			i = y * w + x;
			imgData[i] = ((BYTE)(x ^ (INT)(y + pDt * (timePassed / 100)))) << rData;
		}
	}

	SetBitmapBits(hbm, (DWORD)imgSz, (const void*)imgData);
	SelectObject(hdc, hbm);
	free(imgData);
}

VOID WINAPI layeredWindowPayload2(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam) {
	BITMAP bmp = { 0 };
	if (!GetObjectW(hbm, sizeof(bmp), &bmp)) {
		DebugLine(L"GetObjectW Failed!");
		return;
	}
	SIZE_T imgSz = (SIZE_T)bmp.bmHeight * bmp.bmWidthBytes * bmp.bmPlanes;
	PDWORD imgData = malloc(imgSz);
	if (!imgData)
		return;
	LONG i, s;
	DOUBLE pxPerW = 255. / w;
	DOUBLE pxPerH = 255. / h;

	GetBitmapBits(hbm, (LONG)imgSz, imgData);
	for (INT x = 0; x < w; x++) {
		for (INT y = 0; y < h; y++) {
			i = y * w + x;
			s = (LONG)((
				  sin(((DOUBLE)x) / (w/3.))
				* sin(((DOUBLE)y) / (h/3.))
				) * 255 + (timePassed / (10- timePassed/ 3001.)));
			imgData[i] = HUE(s);
		}
	}

	SetBitmapBits(hbm, (DWORD)imgSz, (const void*)imgData);
	free(imgData);
}

VOID WINAPI layeredWindowPayload3(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam) {
	BITMAP bmp = { 0 };
	if (!GetObjectW(hbm, sizeof(bmp), &bmp)) {
		DebugLine(L"GetObjectW Failed!");
		return;
	}
	SIZE_T imgSz = (SIZE_T)bmp.bmHeight * bmp.bmWidthBytes * bmp.bmPlanes;
	PDWORD imgData = malloc(imgSz);
	if (!imgData)
		return;
	LONG i;
	DOUBLE pxPerW = 255. / w;
	DOUBLE pxPerH = 255. / h;
	DOUBLE ff = sin(timePassed/1000.)+1;

	GetBitmapBits(hbm, (LONG)imgSz, imgData);
	for (INT x = 0; x < w; x++) {
		for (INT y = 0; y < h; y++) {
			i = y * w + x;
			imgData[i] = (INT)((sin((DOUBLE)x / (w*ff)) * cos((DOUBLE)y / (h*ff)) * x
						 * 255 + (timePassed * 100)) + (DOUBLE)HUE((INT)(ff*(x ^ y))));
		}
	}

	SetBitmapBits(hbm, (DWORD)imgSz, (const void*)imgData);
	HICON hcWarn = LoadIcon(NULL, MAKEINTRESOURCE(IDI_EXCLAMATION));
	DrawIcon(hdc, rand() % w, rand() % h, hcWarn);
	DestroyIcon(hcWarn);
	free(imgData);
}

VOID WINAPI AudioSequence2(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam) {
	for (INT t = 0; t < nCount; t++) {
		buffer[t] = (SHORT)((sin(t / 1000.) * cos(t/1000.) * t) * 256);
	}
}


VOID WINAPI AudioSequence3(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam) {
	for (INT t = 0; t < nCount; t++) {
		buffer[t] = ((t * (t & 16)) ^ (UINT)(t * (sin(log(t / 1500)) * 12)) * 5) * 256;
	}
}

VOID WINAPI layeredWindowPayload4(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam) {
	BITMAP bmp = { 0 };
	if (!GetObjectW(hbm, sizeof(bmp), &bmp)) {
		DebugLine(L"GetObjectW Failed!");
		return;
	}
	SIZE_T imgSz = (SIZE_T)bmp.bmHeight * bmp.bmWidthBytes * bmp.bmPlanes;
	PDWORD imgData = malloc(imgSz);
	if (!imgData)
		return;
	LONG i;
	DOUBLE pxPerW = 255. / w;
	DOUBLE pxPerH = 255. / h;
	TIME_T tp = (TIME_T)(timePassed * (timePassed / 10000.));
	DWORD hc = HUE((INT)tp);
	INT m = (INT)(timePassed / 10);
	DOUBLE tps = sin(timePassed/300.) * 500;
	INT cx = (INT)(sin( timePassed / 50. ) * tps),
		cy = (INT)(cos( timePassed / 50. ) * tps);

	ZeroMemory(imgData, imgSz);

	for (INT x = 0; x < w; x++) {
		for (INT y = 0; y < h; y++) {
			i = y * w + x;
			imgData[i] = hc;
			if (!(((y/3)+m) % 6)) {
				imgData[i] = HUE((INT)(tp + 180));
			}
		}
	}

	SetBitmapBits(hbm, (DWORD)imgSz, (const void*)imgData);
	free(imgData);
	HRGN rgn = CreateEllipticRgn(w / 2 - 100 + cx, h / 2 - 100 + cy,
								 w / 2 + 100 + cx, h / 2 + 100 + cy);
	SelectClipRgn(hdc, rgn);
	BitBlt(hdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
	PatBlt(hdc, 0, 0, w, h, PATINVERT);
	SelectClipRgn(hdc, NULL);
	DeleteObject(rgn);
}

VOID WINAPI AudioSequence4(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam) {
	for (INT t = 0; t < nCount; t++) {
		buffer[t] = (SHORT)((PS_DRUM(t) ^ (t >> t)) * 256);
	}
}

VOID WINAPI layeredWindowPayload5(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam) {
	PFRACTAL_COLORMAP fractalColormap = (PFRACTAL_COLORMAP)lParam;
	COLORREF* colormap = fractalColormap->colormap;
	DWORD maxColors = fractalColormap->dwColormapLength;
	BITMAP bmp = { 0 };
	if (!GetObjectW(hbm, sizeof(bmp), &bmp)) {
		DebugLine(L"GetObjectW Failed!");
		return;
	}
	SIZE_T imgSz = (SIZE_T)bmp.bmHeight * bmp.bmWidthBytes * bmp.bmPlanes;
	PDWORD imgData = malloc(imgSz);
	if (!imgData)
		return;
	LONG i;
	INT iteration = 0;
	DOUBLE x0 = 0., y0 = 0., x = 0., y = 0., xTemp=0.;

	// settings
	INT mandelbrotX = w / 8, mandelbrotY = 0;
	SIZE mandelSize = { -2, -2 };
	SIZE dMandelSize = { 2, 2 };
	INT maxIteration = 300;
	DOUBLE mClr = 255. / maxIteration;
	DOUBLE mx = 0., my = 0.;

	DOUBLE R = 2.0;
	DOUBLE f = 0.95, RZ = 2;
	DOUBLE zoomX = -0.740061055,
		   zoomY = 0.19184679995;
	DOUBLE zoomTimes = timePassed / 100.;
	COLORREF white = RGB(255, 255, 255);
	DOUBLE qClr = 0.;
	DOUBLE smoothIterN = 0.;
	DOUBLE stability = 0.;
	DWORD dwIndex;
	CHAR zoomtxt[256] = { 0 };
	DOUBLE l2 = log(2);
	RZ = pow(f, zoomTimes) * RZ;

	ZeroMemory(imgData, imgSz);
	for (INT Px = 0; Px < w; Px++) {
		x0 = (Px / (DOUBLE)w) * RZ - RZ + zoomX;

		for (INT Py = 0; Py < h; Py++) {
			i = Py * w + Px;
			y0 = (Py / (DOUBLE)h) * RZ - RZ + zoomY;

			x = 0;
			y = 0;
			iteration = 0;

			while ((x * x + y * y) <= (R * R) &&
				   (iteration < maxIteration)) {
				xTemp = x * x - y * y + x0;
				y = 2 * x * y + y0;
				x = xTemp;
				iteration++;
			}
			smoothIterN = iteration + 1 - 
				          log(log(COMPLEX_ABS(x, y))) / log(2);
			stability = 1 - max(0.0, min(1.0, smoothIterN / maxIteration));
			dwIndex = (DWORD)(min(maxColors, stability * maxColors));
			imgData[i] = colormap[dwIndex];
		}
	}

	SetBitmapBits(hbm, (DWORD)imgSz, (const void*)imgData);
	free(imgData);
#ifdef DEBUG
	HFONT hf = CreateFontA(48, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Impact");
	StringCchPrintfA(zoomtxt, 256, "Zoom: %e", RZ);
	RECT rc1 = { 2, 2, w, h };
	RECT rc2 = { 2, 4, w, h };
	SelectObject(hdc, hf);
	BitBlt(hdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, 0);
	DrawTextA(hdc, zoomtxt, -1, &rc1, DT_SINGLELINE | DT_NOCLIP);
	SetTextColor(hdc, white);
	DrawTextA(hdc, zoomtxt, -1, &rc2, DT_SINGLELINE | DT_NOCLIP);
	DeleteObject(hf);
#endif // DEBUG
	
}

VOID WINAPI layeredWindowPayload6(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam) {
	SHADER_DATA shd = *((SHADER_DATA*)lParam);
	BITMAP bmp = { 0 };
	if (!GetObjectW(hbm, sizeof(bmp), &bmp)) {
		DebugLine(L"GetObjectW Failed!");
		return;
	}
	SIZE_T imgSz = (SIZE_T)bmp.bmHeight * bmp.bmWidthBytes * bmp.bmPlanes;
	PDWORD imgData = malloc(imgSz);
	if (!imgData)
		return;
	LONG i = 0;
	INT iteration = 0;
	INT maxIteration = 200;
	DOUBLE zx = 0., zy = 0., xTemp = 0.;
	double long juliaIter = 0;// (timePassed / 40000000.);

	// DOUBLE cx = 0.7885 * cos(juliaIter * M_PI / 180.),
	//	      cy = 0.7885 * sin(juliaIter * M_PI / 180.);
	DOUBLE cx = 0.4585 + juliaIter, cy = 0.4585 + juliaIter;
	DOUBLE R = (1.0 + pow(1.0 + 4.0 * pow(cx * cx + cy * cy, 0.5), 0.5)) / 2.0 + 0.001;
	DOUBLE rot = DEG2RAD(timePassed/200.);
	DOUBLE rs = sin(rot), rc = cos(rot);
	INT ax = 0, ay = 0, aTmp = 0;
	INT rax = w / 2, ray = h / 2;
	DOUBLE f = 0.95, RZ = 1.0;
	DOUBLE zoomX = 0.09696799839823, zoomY = 1.1668940944049;
	DOUBLE zoomTimes = timePassed / 100.;
	COLORREF white = RGB(255, 255, 255);
	RZ = pow(f, zoomTimes) * RZ;

	ZeroMemory(imgData, imgSz);
	for (INT Px = 0; Px < w; Px++) {
		for (INT Py = 0; Py < h; Py++) {
			i = Py * w + Px;
			ax = Px - rax;
			ay = Py - ray;
			aTmp = (INT)(ax * rc - ay * rs);
			ay = (INT)(ax * rs + ay * rc + ray);
			ax = aTmp + rax;

			zx = (DOUBLE)(ax) / (w - 1.) * 2.0 * RZ - RZ + zoomX;
			zy = (DOUBLE)(ay) / (h - 1.) * 2.0 * RZ - RZ + zoomY;
			iteration = 0;
			while (((zx * zx + zy * zy) < (R * R))
				   && (iteration < maxIteration)) {
				xTemp = zx * zx - zy * zy;
				zy = 2 * zx * zy + cy;
				zx = xTemp + cx;
				iteration++;
			}

			if (!shd.mode1)
				imgData[i] = white - LIGHTNESS(iteration * 20);
			else if (shd.mode1 == 1)
				imgData[i] = HUE(iteration * 20);
			else if (shd.mode1 == 2)
				imgData[i] = iteration * 20;
			else if (shd.mode1 == 3)
				imgData[i] = (iteration * 20) << 8;
			else
				imgData[i] = (iteration * 20) << 16;
			
		}
	}

	SetBitmapBits(hbm, (DWORD)imgSz, (const void*)imgData);
	free(imgData);
}

VOID WINAPI layeredWindowPayload7(HDC hdc, HDC hdcDst, HBITMAP hbm, INT w, INT h, TIME_T timePassed, LPARAM lParam) {
	// move window to top
	SetWindowPos(WindowFromDC(hdcDst), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	
	BITMAP bmp = { 0 };
	if (!GetObjectW(hbm, sizeof(bmp), &bmp)) {
		DebugLine(L"GetObjectW Failed!");
		return;
	}
	SIZE_T imgSz = (SIZE_T)bmp.bmHeight * bmp.bmWidthBytes * bmp.bmPlanes;
	PDWORD imgData = malloc(imgSz);
	if (!imgData)
		return;
	LONG i;
	INT iteration = 0;
	DOUBLE x0 = 0., y0 = 0., x = 0., y = 0., xTemp = 0.;

	// settings
	INT mandelbrotX = w / 8, mandelbrotY = 0;
	SIZE mandelSize = { -2, -2 };
	SIZE dMandelSize = { 2, 2 };
	INT maxIteration = 128;
	DOUBLE mClr = 255. / maxIteration;
	DOUBLE mx = 0., my = 0.;

	DOUBLE R = 2.0;
	DOUBLE f = 0.95;
	COLORREF white = RGB(255, 255, 255);
	DOUBLE qClr = 0.;
	DOUBLE smoothIterN = 0.;
	DOUBLE stability = 0.;
	DWORD dwIndex;

	DOUBLE zoom = 1;
	DOUBLE zoomX = -1.77648,
		   zoomY = -0.0415;
	
	DOUBLE minX = -2.5, maxX = 1,
		   minY = -1,   maxY = 1;

	DOUBLE RZ = pow(f, timePassed / 100.);


	ZeroMemory(imgData, imgSz);
	for (INT Px = 0; Px < w; Px++) {
		for (INT Py = 0; Py < h; Py++) {
			i = Py * w + Px;
			x0 = (Px / (DOUBLE)w) * (maxX - minX) * RZ - RZ + zoomX;
			y0 = (Py / (DOUBLE)h) * (maxY - minY) * RZ - RZ + zoomY;

			x = x0;
			y = y0;
			iteration = 0;

			while (((x * x + y * y) < (4)) &&
				(iteration < maxIteration)) {
				xTemp = x * x - y * y + x0;
				y = fabs(2. * x * y) + y0;
				x = xTemp;
				iteration++;
			}
			smoothIterN = iteration + 1 -
				log(log(COMPLEX_ABS(x, y))) / log(2);
			stability = 1 - max(0.0, min(1.0, smoothIterN / maxIteration));
			dwIndex = (DWORD)(min(255, stability * 255));
			imgData[i] = iteration == maxIteration ? 0 : mandelbrotPalette[dwIndex];
		}
	}

	SetBitmapBits(hbm, (DWORD)imgSz, (const void*)imgData);
	free(imgData);
}

VOID WINAPI AudioSequence5(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam) {
	for (INT t = 0; t < nCount; t++) {
		buffer[t] = (SHORT)(((((t * 100) / 81) % 81) / 100.) * t * sin(t / 8000.));
	}
}

VOID WINAPI AudioSequence6(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam) {
	for (INT t = 0; t < nCount; t++) {
		buffer[t] = ((t % 16 * t) * ((t / 2000 % 4) + 1)) * 255;
	}
}

VOID WINAPI AudioSequence7(_Inout_ PSHORT buffer, _In_ LONG nCount, _In_ LPARAM lParam) {
	for (LONG t = 0; t < nCount; t++) {
		buffer[t] = (SHORT)(11000 * (sin(t) * (t / 22000.)) * 255);
	}
}
