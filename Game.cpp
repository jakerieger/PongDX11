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
static constexpr float g_UpdateFreq = 80;  // 80% current frame rate

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

    CreateD2DResources();
}

void Game::Tick() {
    m_Timer.Tick([&]() { Update(m_Timer); });

    const auto start = std::chrono::high_resolution_clock::now();

    Render();

    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<float, std::milli> elapsed = end - start;

    static auto maxFrameCount = (100.f / g_UpdateFreq) * (1000.f / elapsed.count());

    if (g_FrameCount >= maxFrameCount) {
        g_FrameCount = 0;
        g_FrameRate  = 1000.f / elapsed.count();
    }

    g_FrameCount++;
}

void Game::OnDeviceLost() {
    // Cleaup code here
    m_pD2DRenderTarget.Reset();
    m_pD2DFactory.Reset();
    m_pDWriteFactory.Reset();
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
    // const auto [left, top, right, bottom] = m_pDeviceResources->GetOutputSize();
    // m_pDeviceResources->WindowSizeChanged(right, bottom);
    //

    const auto [left, top, right, bottom] = m_pDeviceResources->GetOutputSize();
    OnWindowSizeChanged(right, bottom);
}

void Game::OnDisplayChange() {
    m_pDeviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height) {
    if (m_pD2DRenderTarget) {
        m_pD2DRenderTarget.Reset();
    }

    if (!m_pDeviceResources->WindowSizeChanged(width, height))
        return;
    CreateWindowSizeDependentResources();

    CreateD2DSurface();
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

void Game::RenderInterface() const {
    if (m_pD2DRenderTarget) {
        m_pD2DRenderTarget->BeginDraw();

        // UI rendering code here
        ID2D1SolidColorBrush* brush = nullptr;
        DX::ThrowIfFailed(
          m_pD2DRenderTarget->CreateSolidColorBrush(D2D1_COLOR_F(1.f, 1.f, 1.f, 1.f), &brush));

        {  // FPS counter
            const auto fmt = std::format("fRate: {:.2f}", g_FrameRate);
            std::wstring fpsCounter;
            ANSIToWide(fmt, fpsCounter);
            m_pD2DRenderTarget->DrawText(fpsCounter.c_str(),
                                         wcslen(fpsCounter.c_str()),
                                         m_pTextFormat.Get(),
                                         D2D1::RectF(20, 20, 200, 50),
                                         brush);
        }

        {  // Frame time counter
            const auto fmt = std::format("fTime: {:.2f} ms", 1000.f / g_FrameRate);
            std::wstring frameTime;
            ANSIToWide(fmt, frameTime);
            m_pD2DRenderTarget->DrawText(frameTime.c_str(),
                                         wcslen(frameTime.c_str()),
                                         m_pTextFormat.Get(),
                                         D2D1::RectF(20, 40, 400, 50),
                                         brush);
        }

        brush->Release();
        DX::ThrowIfFailed(m_pD2DRenderTarget->EndDraw());
    }
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
    const auto device  = m_pDeviceResources->GetD3DDevice();
    const auto context = m_pDeviceResources->GetD3DDeviceContext();

    D3D11_BLEND_DESC blendDesc                      = {};
    blendDesc.RenderTarget[0].BlendEnable           = TRUE;
    blendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* blendState = nullptr;
    DX::ThrowIfFailed(device->CreateBlendState(&blendDesc, &blendState));
    context->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
    blendState->Release();
}

void Game::CreateWindowSizeDependentResources() {}

void Game::CreateD2DResources() {
    DX::ThrowIfFailed(
      D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&m_pD2DFactory)));
    DX::ThrowIfFailed(
      DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_pDWriteFactory));

    CreateD2DSurface();

    DX::ThrowIfFailed(m_pDWriteFactory->CreateTextFormat(L"Chakra Petch",
                                                         nullptr,
                                                         DWRITE_FONT_WEIGHT_NORMAL,
                                                         DWRITE_FONT_STYLE_NORMAL,
                                                         DWRITE_FONT_STRETCH_NORMAL,
                                                         16.f,
                                                         L"en-us",
                                                         &m_pTextFormat));

    DX::ThrowIfFailed(m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
    DX::ThrowIfFailed(m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
}

void Game::CreateD2DSurface() {
    IDXGISurface* surface;
    DX::ThrowIfFailed(m_pDeviceResources->GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&surface)));

    const D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
      D2D1_RENDER_TARGET_TYPE_DEFAULT,
      D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
      0,
      0);

    DX::ThrowIfFailed(
      m_pD2DFactory->CreateDxgiSurfaceRenderTarget(surface, &props, &m_pD2DRenderTarget));
    surface->Release();
}
