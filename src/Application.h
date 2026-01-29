#pragma once

#include <d3d11.h>
#include <Windows.h>
#include <memory>

namespace DriverMonitor {

class MainWindow;
class EventManager;
class Config;
class DriverMonitor;

class Application {
public:
    Application();
    ~Application();
    
    // Initialize the application
    bool Initialize(HINSTANCE hInstance);
    
    // Run the main loop
    int Run();
    
    // Shutdown the application
    void Shutdown();
    
    // Get D3D11 device
    ID3D11Device* GetDevice() { return m_pd3dDevice; }
    ID3D11DeviceContext* GetDeviceContext() { return m_pd3dDeviceContext; }
    
    // Get window handle
    HWND GetWindowHandle() { return m_hwnd; }
    
private:
    HWND m_hwnd;
    ID3D11Device* m_pd3dDevice;
    ID3D11DeviceContext* m_pd3dDeviceContext;
    IDXGISwapChain* m_pSwapChain;
    ID3D11RenderTargetView* m_mainRenderTargetView;
    
    // Application components
    std::unique_ptr<EventManager> m_eventManager;
    std::unique_ptr<Config> m_config;
    std::unique_ptr<DriverMonitor> m_driverMonitor;
    std::unique_ptr<MainWindow> m_mainWindow;
    
    // Initialize DirectX 11
    bool CreateDeviceD3D(HWND hWnd);
    
    // Cleanup DirectX 11
    void CleanupDeviceD3D();
    
    // Create render target
    void CreateRenderTarget();
    
    // Cleanup render target
    void CleanupRenderTarget();
    
    // Setup ImGui
    bool SetupImGui();
    
    // Cleanup ImGui
    void CleanupImGui();
    
    // Apply dark theme
    void ApplyDarkTheme();
};

} // namespace DriverMonitor
