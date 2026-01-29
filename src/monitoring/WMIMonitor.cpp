#include "WMIMonitor.h"
#include <Windows.h>
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

namespace DriverMonitor {

WMIMonitor::WMIMonitor() : m_initialized(false) {
}

WMIMonitor::~WMIMonitor() {
}

void WMIMonitor::Initialize() {
    if (m_initialized) {
        return;
    }
    
    // Initialize COM
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    
    // Query current drivers using WMI
    HRESULT hr;
    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    
    hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
    
    if (SUCCEEDED(hr)) {
        hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &pSvc);
        
        if (SUCCEEDED(hr)) {
            CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
            
            IEnumWbemClassObject* pEnumerator = nullptr;
            hr = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_SystemDriver"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);
            
            if (SUCCEEDED(hr)) {
                IWbemClassObject* pclsObj = nullptr;
                ULONG uReturn = 0;
                
                while (pEnumerator) {
                    hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
                    if (uReturn == 0) break;
                    
                    VARIANT vtProp;
                    hr = pclsObj->Get(L"Name", 0, &vtProp, nullptr, nullptr);
                    if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
                        _bstr_t bstrName(vtProp.bstrVal);
                        m_knownDrivers.insert((const char*)bstrName);
                    }
                    VariantClear(&vtProp);
                    
                    pclsObj->Release();
                }
                
                pEnumerator->Release();
            }
            
            pSvc->Release();
        }
        
        pLoc->Release();
    }
    
    m_initialized = true;
}

DriverEvent WMIMonitor::CheckForNewDrivers() {
    if (!m_initialized) {
        Initialize();
    }
    
    DriverEvent event;
    
    // Query WMI for drivers
    HRESULT hr;
    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    
    hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
    
    if (SUCCEEDED(hr)) {
        hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &pSvc);
        
        if (SUCCEEDED(hr)) {
            CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
            
            IEnumWbemClassObject* pEnumerator = nullptr;
            hr = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_SystemDriver"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);
            
            if (SUCCEEDED(hr)) {
                IWbemClassObject* pclsObj = nullptr;
                ULONG uReturn = 0;
                
                while (pEnumerator) {
                    hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
                    if (uReturn == 0) break;
                    
                    VARIANT vtName, vtPath;
                    hr = pclsObj->Get(L"Name", 0, &vtName, nullptr, nullptr);
                    
                    if (SUCCEEDED(hr) && vtName.vt == VT_BSTR) {
                        _bstr_t bstrName(vtName.bstrVal);
                        std::string driverName = (const char*)bstrName;
                        
                        // Check if this is a new driver
                        if (m_knownDrivers.find(driverName) == m_knownDrivers.end()) {
                            m_knownDrivers.insert(driverName);
                            
                            event.driverName = driverName;
                            event.loadingMethod = "WMI - System Driver Query";
                            event.initiatedBy = "WMI Service";
                            event.processId = 0;
                            
                            // Try to get path
                            hr = pclsObj->Get(L"PathName", 0, &vtPath, nullptr, nullptr);
                            if (SUCCEEDED(hr) && vtPath.vt == VT_BSTR) {
                                _bstr_t bstrPath(vtPath.bstrVal);
                                event.installPath = (const char*)bstrPath;
                            }
                            VariantClear(&vtPath);
                            
                            VariantClear(&vtName);
                            pclsObj->Release();
                            pEnumerator->Release();
                            pSvc->Release();
                            pLoc->Release();
                            
                            return event;
                        }
                    }
                    VariantClear(&vtName);
                    
                    pclsObj->Release();
                }
                
                pEnumerator->Release();
            }
            
            pSvc->Release();
        }
        
        pLoc->Release();
    }
    
    return event;
}

} // namespace DriverMonitor
