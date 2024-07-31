// Author: Jake Rieger
// Created: 7/30/2024.
//

#include "pch.h"
#include "Game.h"

#include <chrono>

using namespace DirectX;

extern void ExitGame() noexcept;

static float g_FrameRate            = 0.f;
static int g_FrameCount             = 0;
static constexpr int kMaxFrameCount = 30;

Game::Game() noexcept(false) {
    m_pDeviceResources = std::make_unique<DX::DeviceResources>();
    m_pDeviceResources->RegisterDeviceNotify(this);
}

void Game::Initialize(HWND window, const int width, const int height) {
    m_pDeviceResources->SetWindow(window, width, height);

    m_pDeviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_pDeviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    CreateDirect2DResources();
}

void Game::Tick() {
    m_Timer.Tick([&]() { Update(m_Timer); });

    const auto start = std::chrono::high_resolution_clock::now();

    Render();

    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<float, std::milli> elapsed = end - start;

    if (g_FrameCount >= kMaxFrameCount) {
        g_FrameCount = 0;
        g_FrameRate  = 1000.f / elapsed.count();
    }

    g_FrameCount++;
}

void Game::OnDeviceLost() {
    // Cleaup code here
}

void Game::OnDeviceRestored() {
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

void Game::OnActivated() {
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated() {
    // TODO: Game is becoming background window.
}

void Game::OnSuspending() {
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming() {
    m_Timer.ResetElapsedTime();
    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved() {
    const auto [left, top, right, bottom] = m_pDeviceResources->GetOutputSize();
    m_pDeviceResources->WindowSizeChanged(right, bottom);
}

void Game::OnDisplayChange() {
    m_pDeviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height) {
    if (!m_pDeviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
    // TODO: Game window is being resized.
}

void Game::GetDefaultSize(int& width, int& height) const {
    width  = 1280;
    height = 720;
}

void Game::Update(const DX::StepTimer& timer) {
    const auto dT = static_cast<float>(timer.GetElapsedSeconds());
}

void Game::Render() {
    if (m_Timer.GetFrameCount() == 0) {
        return;
    }

    Clear();

    auto context = m_pDeviceResources->GetD3DDeviceContext();

    // TODO: Render code goes here

    // Ensure all D3D command are executed before switching to D2D
    m_pDeviceResources->GetD3DDeviceContext()->Flush();

    RenderInterface();

    m_pDeviceResources->Present();
}

void Game::RenderInterface() {
    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->Clear(D2D1::ColorF(0, 0.f));

    // UI rendering code here
    ID2D1SolidColorBrush* brush = nullptr;
    DX::ThrowIfFailed(
      m_pRenderTarget->CreateSolidColorBrush(D2D1_COLOR_F(1.f, 0.f, 0.f, 1.f), &brush));
    m_pRenderTarget->DrawRectangle({0, 0, 100, 100}, brush);
    brush->Release();

    DX::ThrowIfFailed(m_pRenderTarget->EndDraw());
}

void Game::Clear() {
    // Clear the views.
    auto context      = m_pDeviceResources->GetD3DDeviceContext();
    auto renderTarget = m_pDeviceResources->GetRenderTargetView();
    auto depthStencil = m_pDeviceResources->GetDepthStencilView();

    float clearColor[4] = {17.f / 255.f, 18.f / 255.f, 28.f / 255.f, 1.f};
    context->ClearRenderTargetView(renderTarget, clearColor);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto const viewport = m_pDeviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);
}

void Game::CreateDeviceDependentResources() {
    auto device  = m_pDeviceResources->GetD3DDevice();
    auto context = m_pDeviceResources->GetD3DDeviceContext();
}

void Game::CreateWindowSizeDependentResources() {}

void Game::CreateDirect2DResources() {
    DX::ThrowIfFailed(
      D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&m_pD2DFactory)));
    DX::ThrowIfFailed(
      DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_pDWriteFactory));

    IDXGISurface* surface;
    DX::ThrowIfFailed(m_pDeviceResources->GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&surface)));

    const D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
      D2D1_RENDER_TARGET_TYPE_DEFAULT,
      D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
      0,
      0);

    DX::ThrowIfFailed(
      m_pD2DFactory->CreateDxgiSurfaceRenderTarget(surface, &props, &m_pRenderTarget));
    surface->Release();
}
