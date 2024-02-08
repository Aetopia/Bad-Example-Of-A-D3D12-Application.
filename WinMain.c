#include <initguid.h>
#include <windows.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcommon.h>

static BOOL fClose = FALSE;

LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        fClose = TRUE;
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

DWORD ThreadProc(LPVOID lpParameter)
{
    HRESULT hr = S_OK;
    while (TRUE)
    {
        hr = ((IDXGISwapChain1 *)lpParameter)->lpVtbl->Present(lpParameter, 0, 0);
        if (fClose)
            break;
    }
    ((IDXGISwapChain1 *)lpParameter)->lpVtbl->Release(lpParameter);

    if (hr != S_OK)
        ExitProcess(0);

    return 0;
}

int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    MSG msg = {};
    HWND hWnd = NULL;

    if (!RegisterClassExW(&((WNDCLASSEXW){.cbSize = sizeof(WNDCLASSEXW),
                                          .lpszClassName = L"🔥",
                                          .hInstance = hInstance,
                                          .lpfnWndProc = WndProc,
                                          .hCursor = LoadCursorW(NULL, IDC_ARROW),
                                          .hbrBackground = GetStockObject(BLACK_BRUSH)})) ||
        !(hWnd = CreateWindowExW(WS_EX_LEFT, L"🔥", NULL, WS_VISIBLE | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                                 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL)))
        return 1;

    IDXGIFactory6 *pFactory = NULL;
    CreateDXGIFactory(&IID_IDXGIFactory7, (void **)&pFactory);

    IDXGIAdapter4 *pAdapter = NULL;
    pFactory->lpVtbl->EnumAdapterByGpuPreference(pFactory, 0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, &IID_IDXGIAdapter4,
                                                 (void **)&pAdapter);

    pFactory->lpVtbl->Release(pFactory);

    ID3D12Device *pDevice = NULL;
    D3D12CreateDevice((IUnknown *)pAdapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, (void **)&pDevice);

    ID3D12CommandQueue *pCommandQueue = NULL;
    pDevice->lpVtbl->CreateCommandQueue(
        pDevice,
        &((D3D12_COMMAND_QUEUE_DESC){.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     .Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT}),
        &IID_ID3D12CommandQueue, (void **)&pCommandQueue);
    pDevice->lpVtbl->Release(pDevice);

    IDXGISwapChain1 *pSwapChain = NULL;
    pFactory->lpVtbl->CreateSwapChainForHwnd(pFactory, (IUnknown *)pCommandQueue, hWnd,
                                             &((DXGI_SWAP_CHAIN_DESC1){.BufferCount = 2,
                                                                       .Width = 0,
                                                                       .Height = 0,
                                                                       .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                                                                       .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                                                                       .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
                                                                       .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
                                                                       .SampleDesc.Count = 1}),
                                             NULL, NULL, &pSwapChain);

    CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, pSwapChain, 0, NULL));

    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}
