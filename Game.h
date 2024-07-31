// Author: Jake Rieger
// Created: 7/30/2024.
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"

class Game final : public DX::IDeviceNotify {
public:
    Game() noexcept(false);
    ~Game() = default;

    Game(Game&&)            = default;
    Game& operator=(Game&&) = default;

    Game(Game const&)            = delete;
    Game& operator=(Game const&) = delete;

    void Initialize(HWND window, int width, int height);
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    void GetDefaultSize(int& width, int& height) const;

private:
    void Update(const DX::StepTimer& timer);
    void Render();

    /// Renders the UI drawn by Direct2D
    void RenderInterface() const;

    void Clear();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void CreateD2DResources();
    void CreateD2DSurface();

    std::unique_ptr<DX::DeviceResources> m_pDeviceResources;
    DX::StepTimer m_Timer;

    ComPtr<ID2D1Factory> m_pD2DFactory;
    ComPtr<IDWriteFactory> m_pDWriteFactory;
    ComPtr<IDWriteTextFormat> m_pTextFormat;
    ComPtr<ID2D1RenderTarget> m_pD2DRenderTarget;
};
