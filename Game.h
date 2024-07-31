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
    void Clear();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    std::unique_ptr<DX::DeviceResources> m_DeviceResources;
    DX::StepTimer m_Timer;
};
