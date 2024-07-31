#include "pch.h"
#include "Game.h"

#include <chrono>

#pragma warning(disable : 4061)
#pragma warning(disable : 4996)

using namespace DirectX;

// Indicates to hybrid graphics systems to prefer the discrete GPU by default.
// See: https://gpuopen.com/learn/amdpowerxpressrequesthighperformance/
extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement                  = 0x00000001;
__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
}

LPCSTR g_szAppName = "Pong | FPS: 60.00 <DX11>";

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

namespace {
    std::unique_ptr<Game> g_Game;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance,
                    _In_opt_ HINSTANCE hPrevInstance,
                    _In_ LPWSTR lpCmdLine,
                    _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (!::XMVerifyCPUSupport())
        return 1;

    HRESULT hr = ::CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    if (FAILED(hr))
        return 1;

    g_Game = std::make_unique<Game>();

    // Register class and create window

    // Register class
    WNDCLASSEXA wcex   = {};
    wcex.cbSize        = sizeof(WNDCLASSEXA);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = ::LoadIcon(hInstance, "IDI_ICON1");
    wcex.hCursor       = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = "PongDX11Class";
    wcex.hIconSm       = wcex.hIcon;
    if (!::RegisterClassEx(&wcex))
        return 1;

    // Create window
    int w, h;
    g_Game->GetDefaultSize(w, h);

    RECT rc = {0, 0, static_cast<LONG>(w), static_cast<LONG>(h)};

    ::AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = ::CreateWindowEx(0,
                                 "PongDX11Class",
                                 g_szAppName,
                                 WS_OVERLAPPEDWINDOW,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 rc.right - rc.left,
                                 rc.bottom - rc.top,
                                 nullptr,
                                 nullptr,
                                 hInstance,
                                 g_Game.get());
    // TODO: Change to CreateWindowExW(WS_EX_TOPMOST, L"PongDX11WindowClass",
    // g_szAppName, WS_POPUP, to default to fullscreen.

    if (!hwnd)
        return 1;

    ::ShowWindow(hwnd, nCmdShow);
    // TODO: Change nCmdShow to SW_SHOWMAXIMIZED to default to fullscreen.

    ::GetClientRect(hwnd, &rc);

    g_Game->Initialize(hwnd, rc.right - rc.left, rc.bottom - rc.top);

    // Main message loop
    MSG msg = {};
    while (WM_QUIT != msg.message) {
        auto start = std::chrono::high_resolution_clock::now();
        if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        } else {
            g_Game->Tick();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> elapsed = end - start;

        auto fR  = 1000.f / elapsed.count();
        auto fmt = std::format("Pong | FPS: {:.2f} <DX11>", fR);
        ::SetWindowTextA(hwnd, fmt.c_str());
    }

    g_Game.reset();

    ::CoUninitialize();

    return static_cast<int>(msg.wParam);
}

LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        case WM_CLOSE:
            ::DestroyWindow(hwnd);
            return 0;
        default:
            break;
    }
    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}
