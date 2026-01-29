#include "Application.h"
#include <Windows.h>
#include <tchar.h>

// Entry point for Windows application
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    
    // Check for administrator privileges
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    if (!isAdmin) {
        MessageBoxA(nullptr, 
                   "This application requires administrator privileges to monitor drivers.\n\n"
                   "Please run as Administrator.",
                   "Administrator Required", 
                   MB_OK | MB_ICONWARNING);
        return 1;
    }
    
    // Create and initialize application
    DriverMonitor::Application app;
    
    if (!app.Initialize(hInstance)) {
        MessageBoxA(nullptr, 
                   "Failed to initialize application.\n\n"
                   "Please ensure DirectX 11 is available and ImGui library is properly installed.",
                   "Initialization Error", 
                   MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Run application
    int result = app.Run();
    
    // Cleanup
    app.Shutdown();
    
    return result;
}

// Console entry point for testing/debugging
int main() {
    return WinMain(GetModuleHandle(nullptr), nullptr, GetCommandLineA(), SW_SHOWDEFAULT);
}
