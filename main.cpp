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

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

namespace {
    auto g_AppName = "Pong | FPS: 60.00 <DX11>";
    std::unique_ptr<Game> g_Game;
}  // namespace

int WINAPI wWinMain(_In_ HINSTANCE hInstance,
                    _In_opt_ HINSTANCE hPrevInstance,
                    _In_ LPWSTR lpCmdLine,
                    _In_ int nShowCmd) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    if (!::XMVerifyCPUSupport())
        return 1;

    const HRESULT hr = ::CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
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

    HWND hwnd = ::CreateWindowEx(WS_EX_TOPMOST,
                                 "PongDX11Class",
                                 g_AppName,
                                 WS_POPUP,
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

    ::ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    // TODO: Change nCmdShow to SW_SHOWMAXIMIZED to default to fullscreen.

    ::GetClientRect(hwnd, &rc);

    g_Game->Initialize(hwnd, rc.right - rc.left, rc.bottom - rc.top);

    // Main msg loop
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
    static bool s_InSizeMove = false;
    static bool s_InSuspend  = false;
    static bool s_Minimized  = false;
    static bool s_Fullscreen = true;

    const auto game = reinterpret_cast<Game*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_CREATE:
            if (lParam) {
                const auto params = reinterpret_cast<LPCREATESTRUCTW>(lParam);
                SetWindowLongPtr(hwnd,
                                 GWLP_USERDATA,
                                 reinterpret_cast<LONG_PTR>(params->lpCreateParams));
            }
            break;

        case WM_PAINT:
            if (s_InSizeMove && game) {
                game->Tick();
            } else {
                PAINTSTRUCT ps;
                std::ignore = ::BeginPaint(hwnd, &ps);
                ::EndPaint(hwnd, &ps);
            }
            break;

        case WM_DISPLAYCHANGE:
            if (game) {
                game->OnDisplayChange();
            }
            break;

        case WM_MOVE:
            if (game) {
                game->OnWindowMoved();
            }
            break;

        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED) {
                if (!s_Minimized) {
                    s_Minimized = true;
                    if (!s_InSuspend && game)
                        game->OnSuspending();
                    s_InSuspend = true;
                }
            } else if (s_Minimized) {
                s_Minimized = false;
                if (s_InSuspend && game)
                    game->OnResuming();
                s_InSuspend = false;
            } else if (!s_InSizeMove && game) {
                game->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
            }
            break;

        case WM_ENTERSIZEMOVE:
            s_InSizeMove = true;
            break;

        case WM_EXITSIZEMOVE:
            s_InSizeMove = false;
            if (game) {
                RECT rc;
                GetClientRect(hwnd, &rc);

                game->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
            }
            break;

        case WM_GETMINMAXINFO:
            if (lParam) {
                const auto info        = reinterpret_cast<MINMAXINFO*>(lParam);
                info->ptMinTrackSize.x = 320;
                info->ptMinTrackSize.y = 200;
            }
            break;

        case WM_ACTIVATEAPP:
            if (game) {
                if (wParam) {
                    game->OnActivated();
                } else {
                    game->OnDeactivated();
                }
            }
            break;

        case WM_POWERBROADCAST:
            switch (wParam) {
                case PBT_APMQUERYSUSPEND:
                    if (!s_InSuspend && game)
                        game->OnSuspending();
                    s_InSuspend = true;
                    return TRUE;

                case PBT_APMRESUMESUSPEND:
                    if (!s_Minimized) {
                        if (s_InSuspend && game)
                            game->OnResuming();
                        s_InSuspend = false;
                    }
                    return TRUE;
                default:
                    break;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                ::PostQuitMessage(0);
            }
            return 0;

        case WM_SYSKEYDOWN:
            if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000) {
                // Implements the classic ALT+ENTER fullscreen toggle
                if (s_Fullscreen) {
                    SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                    SetWindowLongPtr(hwnd, GWL_EXSTYLE, 0);

                    int width  = 800;
                    int height = 600;
                    if (game)
                        game->GetDefaultSize(width, height);

                    ShowWindow(hwnd, SW_SHOWNORMAL);

                    SetWindowPos(hwnd,
                                 HWND_TOP,
                                 0,
                                 0,
                                 width,
                                 height,
                                 SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
                } else {
                    SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP);
                    SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);

                    SetWindowPos(hwnd,
                                 HWND_TOP,
                                 0,
                                 0,
                                 0,
                                 0,
                                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

                    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
                }

                s_Fullscreen = !s_Fullscreen;
            }
            break;

        case WM_MENUCHAR:
            // A menu is active and the user presses a key that does not correspond
            // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
            return MAKELRESULT(0, MNC_CLOSE);
        default:
            break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
