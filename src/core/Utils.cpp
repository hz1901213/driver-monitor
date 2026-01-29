#include "Utils.h"
#include <Windows.h>
#include <WinTrust.h>
#include <SoftPub.h>
#include <wincrypt.h>
#include <psapi.h>
#include <algorithm>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "wintrust.lib")
#pragma comment(lib, "crypt32.lib")

namespace DriverMonitor {

std::string Utils::GetTimestamp() {
    time_t now = time(nullptr);
    struct tm timeInfo;
    localtime_s(&timeInfo, &now);
    
    char buffer[64];
    strftime(buffer, sizeof(buffer), "[%H:%M:%S]", &timeInfo);
    return std::string(buffer);
}

std::string Utils::GetProcessName(unsigned long pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) {
        return "Unknown";
    }
    
    char processName[MAX_PATH] = {0};
    DWORD size = MAX_PATH;
    
    if (QueryFullProcessImageNameA(hProcess, 0, processName, &size)) {
        std::string fullPath(processName);
        size_t pos = fullPath.find_last_of("\\/");
        if (pos != std::string::npos) {
            CloseHandle(hProcess);
            return fullPath.substr(pos + 1);
        }
    }
    
    // Fallback to old method
    if (GetModuleBaseNameA(hProcess, nullptr, processName, MAX_PATH)) {
        CloseHandle(hProcess);
        return std::string(processName);
    }
    
    CloseHandle(hProcess);
    return "Unknown";
}

std::string Utils::GetSignerInfo(const std::string& filePath) {
    std::wstring wFilePath(filePath.begin(), filePath.end());
    
    WINTRUST_FILE_INFO fileInfo = {0};
    fileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    fileInfo.pcwszFilePath = wFilePath.c_str();
    
    WINTRUST_DATA trustData = {0};
    trustData.cbStruct = sizeof(WINTRUST_DATA);
    trustData.dwUIChoice = WTD_UI_NONE;
    trustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    trustData.dwUnionChoice = WTD_CHOICE_FILE;
    trustData.pFile = &fileInfo;
    trustData.dwStateAction = WTD_STATEACTION_VERIFY;
    trustData.dwProvFlags = WTD_SAFER_FLAG;
    
    GUID policyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    
    LONG status = WinVerifyTrust(nullptr, &policyGUID, &trustData);
    
    // Close the trust data
    trustData.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(nullptr, &policyGUID, &trustData);
    
    if (status == ERROR_SUCCESS) {
        // Try to get publisher name
        HCERTSTORE hStore = nullptr;
        HCRYPTMSG hMsg = nullptr;
        PCMSG_SIGNER_INFO pSignerInfo = nullptr;
        DWORD dwEncoding = 0, dwContentType = 0, dwFormatType = 0;
        
        if (CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                            wFilePath.c_str(),
                            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
                            CERT_QUERY_FORMAT_FLAG_BINARY,
                            0,
                            &dwEncoding,
                            &dwContentType,
                            &dwFormatType,
                            &hStore,
                            &hMsg,
                            nullptr)) {
            
            DWORD dwSignerInfo = 0;
            if (CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, nullptr, &dwSignerInfo)) {
                pSignerInfo = (PCMSG_SIGNER_INFO)LocalAlloc(LPTR, dwSignerInfo);
                if (pSignerInfo && CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, pSignerInfo, &dwSignerInfo)) {
                    CERT_INFO certInfo = {0};
                    certInfo.Issuer = pSignerInfo->Issuer;
                    certInfo.SerialNumber = pSignerInfo->SerialNumber;
                    
                    PCCERT_CONTEXT pCertContext = CertFindCertificateInStore(
                        hStore,
                        dwEncoding,
                        0,
                        CERT_FIND_SUBJECT_CERT,
                        &certInfo,
                        nullptr);
                    
                    if (pCertContext) {
                        char szName[512] = {0};
                        CertGetNameStringA(pCertContext, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, nullptr, szName, sizeof(szName));
                        
                        CertFreeCertificateContext(pCertContext);
                        
                        if (hMsg) CryptMsgClose(hMsg);
                        if (hStore) CertCloseStore(hStore, 0);
                        if (pSignerInfo) LocalFree(pSignerInfo);
                        
                        std::string signerName(szName);
                        if (!signerName.empty()) {
                            return "Signed by " + signerName;
                        }
                    }
                }
                if (pSignerInfo) LocalFree(pSignerInfo);
            }
            
            if (hMsg) CryptMsgClose(hMsg);
            if (hStore) CertCloseStore(hStore, 0);
        }
        
        return "Signed (Trusted)";
    }
    
    return "Not Signed";
}

bool Utils::IsMicrosoftSigned(const std::string& signerInfo) {
    return ContainsIgnoreCase(signerInfo, "Microsoft");
}

bool Utils::IsWindowsSigned(const std::string& signerInfo) {
    return ContainsIgnoreCase(signerInfo, "Microsoft Windows") ||
           ContainsIgnoreCase(signerInfo, "Microsoft Corporation") ||
           ContainsIgnoreCase(signerInfo, "Signed (Trusted)");
}

ThreatLevel Utils::AssessThreatLevel(const DriverEvent& event) {
    // Microsoft signed = Low threat
    if (IsMicrosoftSigned(event.signerInfo)) {
        return ThreatLevel::Low;
    }
    
    // Signed by someone else = Medium threat
    if (event.signerInfo.find("Signed") != std::string::npos) {
        return ThreatLevel::Medium;
    }
    
    // Unsigned or suspicious loading method = High threat
    if (ContainsIgnoreCase(event.loadingMethod, "Manual Map") ||
        ContainsIgnoreCase(event.loadingMethod, "Direct Load")) {
        return ThreatLevel::High;
    }
    
    // Unsigned = Medium-High threat
    return ThreatLevel::Medium;
}

EventType Utils::DetermineEventType(const std::string& signerInfo, const std::string& loadingMethod) {
    // Suspicious loading methods
    if (ContainsIgnoreCase(loadingMethod, "Manual Map") ||
        ContainsIgnoreCase(loadingMethod, "Direct Load") ||
        ContainsIgnoreCase(loadingMethod, "Unknown")) {
        return EventType::Suspicious;
    }
    
    // Microsoft signed
    if (IsMicrosoftSigned(signerInfo) || IsWindowsSigned(signerInfo)) {
        return EventType::Signed;
    }
    
    // Not signed
    if (signerInfo.find("Not Signed") != std::string::npos) {
        return EventType::Unsigned;
    }
    
    // Other signed (non-Microsoft)
    if (signerInfo.find("Signed") != std::string::npos) {
        return EventType::Unsigned; // Treat as unsigned/warning
    }
    
    return EventType::Unsigned;
}

std::string Utils::ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool Utils::ContainsIgnoreCase(const std::string& haystack, const std::string& needle) {
    std::string lowerHaystack = ToLower(haystack);
    std::string lowerNeedle = ToLower(needle);
    return lowerHaystack.find(lowerNeedle) != std::string::npos;
}

std::string Utils::FormatUptime(int seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setfill('0') << std::setw(2) << minutes << ":"
        << std::setfill('0') << std::setw(2) << secs;
    return oss.str();
}

} // namespace DriverMonitor
