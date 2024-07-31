// Author: Jake Rieger
// Created: 7/30/2024.
//

#include "pch.h"
#include "Game.h"

using namespace DirectX;

extern void ExitGame() noexcept;

Game::Game() noexcept(false) {
    m_DeviceResources = std::make_unique<DX::DeviceResources>();
    m_DeviceResources->RegisterDeviceNotify(this);
}

void Game::Initialize(HWND window, const int width, const int height) {
    m_DeviceResources->SetWindow(window, width, height);

    m_DeviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_DeviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
}

void Game::Tick() {
    m_Timer.Tick([&]() { Update(m_Timer); });

    Render();
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
    const auto [left, top, right, bottom] = m_DeviceResources->GetOutputSize();
    m_DeviceResources->WindowSizeChanged(right, bottom);
}

void Game::OnDisplayChange() {
    m_DeviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height) {
    if (!m_DeviceResources->WindowSizeChanged(width, height))
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

    auto context = m_DeviceResources->GetD3DDeviceContext();

    // Render code goes here

    m_DeviceResources->Present();
}

void Game::Clear() {
    // Clear the views.
    auto context      = m_DeviceResources->GetD3DDeviceContext();
    auto renderTarget = m_DeviceResources->GetRenderTargetView();
    auto depthStencil = m_DeviceResources->GetDepthStencilView();

    float clearColor[4] = {17.f / 255.f, 18.f / 255.f, 28.f / 255.f, 1.f};
    context->ClearRenderTargetView(renderTarget, clearColor);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto const viewport = m_DeviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);
}

void Game::CreateDeviceDependentResources() {
    auto device  = m_DeviceResources->GetD3DDevice();
    auto context = m_DeviceResources->GetD3DDeviceContext();
}

void Game::CreateWindowSizeDependentResources() {}
