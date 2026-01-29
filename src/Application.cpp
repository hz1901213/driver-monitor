#include "Application.h"
#include "gui/MainWindow.h"
#include "core/EventManager.h"
#include "core/Config.h"
#include "core/DriverMonitor.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <tchar.h>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace DriverMonitor {

// Window procedure
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    
    switch (msg) {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED) {
                Application* app = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
                if (app && app->GetDevice()) {
                    // Resize swap chain would go here
                }
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

Application::Application() 
    : m_hwnd(nullptr)
    , m_pd3dDevice(nullptr)
    , m_pd3dDeviceContext(nullptr)
    , m_pSwapChain(nullptr)
    , m_mainRenderTargetView(nullptr) {
}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize(HINSTANCE hInstance) {
    // Create window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, hInstance, nullptr, nullptr, nullptr, nullptr, _T("DriverMonitor"), nullptr };
    RegisterClassEx(&wc);
    
    // Create window
    m_hwnd = CreateWindow(wc.lpszClassName, _T("Windows Driver Monitor v2.0"), 
                         WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720, 
                         nullptr, nullptr, wc.hInstance, nullptr);
    
    if (!m_hwnd) {
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return false;
    }
    
    // Store this pointer for WndProc
    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    
    // Initialize Direct3D
    if (!CreateDeviceD3D(m_hwnd)) {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return false;
    }
    
    // Show the window
    ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hwnd);
    
    // Setup ImGui
    if (!SetupImGui()) {
        return false;
    }
    
    // Apply dark theme
    ApplyDarkTheme();
    
    // Create application components
    m_eventManager = std::make_unique<EventManager>();
    m_config = std::make_unique<Config>();
    m_config->Load();
    
    m_driverMonitor = std::make_unique<DriverMonitor>(m_eventManager.get(), m_config.get());
    m_mainWindow = std::make_unique<MainWindow>(m_eventManager.get(), m_config.get(), m_driverMonitor.get());
    
    return true;
}

int Application::Run() {
    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        
        // Start ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        // Render main window
        m_mainWindow->Render();
        
        // Rendering
        ImGui::Render();
        const float clear_color[4] = { 0.071f, 0.071f, 0.071f, 1.00f }; // RGB(18, 18, 18)
        m_pd3dDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
        m_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        m_pSwapChain->Present(1, 0); // Present with vsync
    }
    
    return (int)msg.wParam;
}

void Application::Shutdown() {
    // Stop monitoring
    if (m_driverMonitor) {
        m_driverMonitor->Stop();
    }
    
    // Save configuration
    if (m_config) {
        m_config->Save();
    }
    
    // Cleanup
    m_mainWindow.reset();
    m_driverMonitor.reset();
    m_config.reset();
    m_eventManager.reset();
    
    CleanupImGui();
    CleanupDeviceD3D();
    
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

bool Application::CreateDeviceD3D(HWND hWnd) {
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    
    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, 
                                                featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain, 
                                                &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext);
    
    if (res == DXGI_ERROR_UNSUPPORTED) {
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, 
                                           featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain, 
                                           &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext);
    }
    
    if (res != S_OK)
        return false;
    
    CreateRenderTarget();
    return true;
}

void Application::CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = nullptr; }
    if (m_pd3dDeviceContext) { m_pd3dDeviceContext->Release(); m_pd3dDeviceContext = nullptr; }
    if (m_pd3dDevice) { m_pd3dDevice->Release(); m_pd3dDevice = nullptr; }
}

void Application::CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
    pBackBuffer->Release();
}

void Application::CleanupRenderTarget() {
    if (m_mainRenderTargetView) { 
        m_mainRenderTargetView->Release(); 
        m_mainRenderTargetView = nullptr; 
    }
}

bool Application::SetupImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(m_pd3dDevice, m_pd3dDeviceContext);
    
    return true;
}

void Application::CleanupImGui() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Application::ApplyDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    
    // Window
    colors[ImGuiCol_WindowBg] = ImVec4(0.071f, 0.071f, 0.071f, 1.00f); // RGB(18, 18, 18)
    colors[ImGuiCol_ChildBg] = ImVec4(0.082f, 0.082f, 0.082f, 1.00f);  // RGB(21, 21, 21)
    colors[ImGuiCol_PopupBg] = ImVec4(0.082f, 0.082f, 0.082f, 0.94f);
    
    // Frame
    colors[ImGuiCol_FrameBg] = ImVec4(0.125f, 0.125f, 0.125f, 1.00f);  // RGB(32, 32, 32)
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.188f, 0.282f, 0.502f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.188f, 0.282f, 0.502f, 0.67f);
    
    // Header
    colors[ImGuiCol_Header] = ImVec4(0.000f, 0.282f, 0.502f, 1.00f);  // RGB(0, 72, 128)
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.000f, 0.353f, 0.627f, 1.00f); // RGB(0, 90, 160)
    colors[ImGuiCol_HeaderActive] = ImVec4(0.000f, 0.376f, 0.753f, 1.00f);  // RGB(0, 96, 192)
    
    // Button
    colors[ImGuiCol_Button] = ImVec4(0.125f, 0.282f, 0.502f, 1.00f);  // RGB(32, 72, 128)
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.125f, 0.345f, 0.565f, 1.00f); // RGB(32, 88, 144)
    colors[ImGuiCol_ButtonActive] = ImVec4(0.063f, 0.251f, 0.439f, 1.00f);  // RGB(16, 64, 112)
    
    // Text
    colors[ImGuiCol_Text] = ImVec4(0.831f, 0.831f, 0.831f, 1.00f);  // RGB(212, 212, 212)
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    
    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    
    // Title
    colors[ImGuiCol_TitleBg] = ImVec4(0.071f, 0.071f, 0.071f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.000f, 0.282f, 0.502f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.071f, 0.071f, 0.071f, 0.51f);
    
    // Menu bar
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.125f, 0.125f, 0.125f, 1.00f);
    
    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.071f, 0.071f, 0.071f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    
    // Check mark
    colors[ImGuiCol_CheckMark] = ImVec4(0.188f, 0.788f, 0.690f, 1.00f); // Teal
    
    // Slider
    colors[ImGuiCol_SliderGrab] = ImVec4(0.188f, 0.282f, 0.502f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.000f, 0.376f, 0.753f, 1.00f);
    
    // Tab
    colors[ImGuiCol_Tab] = ImVec4(0.125f, 0.125f, 0.125f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.000f, 0.353f, 0.627f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.000f, 0.282f, 0.502f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.071f, 0.071f, 0.071f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.125f, 0.125f, 0.125f, 1.00f);
    
    // Separator
    colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    
    // Resize grip
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    
    // Style
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(5, 3);
    style.ItemSpacing = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.IndentSpacing = 21.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;
}

} // namespace DriverMonitor
