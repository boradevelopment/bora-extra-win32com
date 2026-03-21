#include "pch.h"
#include "IconHandler.h"
#include <Shlwapi.h>
#include <StrSafe.h>
#include <gdiplus.h>
#include "SysImageMgr.h"
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return {};

    int size_needed = WideCharToMultiByte(CP_UTF8, 0,
        wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);

    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0,
        wstr.c_str(), -1, &result[0], size_needed, nullptr, nullptr);

    // Remove null terminator added by WideCharToMultiByte
    result.pop_back();
    return result;
}

bool IsSvgData(const uint8_t* data, size_t size) {
    if (!data || size < 16) return false;

    size_t i = 0;

    // Skip UTF-8 BOM
    if (size >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF) {
        i = 3;
    }

    // Skip whitespace
    while (i < size && isspace(data[i])) {
        i++;
    }

    // Check "<svg"
    if (i + 4 > size) return false;

    return
        data[i] == '<' &&
        (data[i + 1] == 's' || data[i + 1] == 'S') &&
        (data[i + 2] == 'v' || data[i + 2] == 'V') &&
        (data[i + 3] == 'g' || data[i + 3] == 'G');
}


BappIconHandler::BappIconHandler() : m_refCount(1) {}
BappIconHandler::~BappIconHandler() {}

// IUnknown
STDMETHODIMP BappIconHandler::QueryInterface(REFIID riid, void** ppvObject) {
    if (riid == IID_IUnknown || riid == IID_IExtractIconW)
        *ppvObject = static_cast<IExtractIconW*>(this);
    else if (riid == IID_IPersistFile)
        *ppvObject = static_cast<IPersistFile*>(this);
    else {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) BappIconHandler::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) BappIconHandler::Release() {
    ULONG ref = InterlockedDecrement(&m_refCount);
    if (ref == 0)
        delete this;
    return ref;
}

// IPersist
STDMETHODIMP BappIconHandler::GetClassID(CLSID* pClassID) {
    *pClassID = CLSID_BappIconHandler; // Define this globally
    return S_OK;
}

// IPersistFile
STDMETHODIMP BappIconHandler::IsDirty() { return S_FALSE; }
STDMETHODIMP BappIconHandler::Load(LPCOLESTR pszFileName, DWORD) {
    //     m_filePath = pszFileName;
    //     archive.output = WStringToString(m_filePath);

    //     int res = archive.getArchive();
    //     if (res == 0) {
    //         auto logoFile = archive.header.files.find(L"logo");
    //         if (logoFile != archive.header.files.end()) {
    //             std::ifstream archiveFile(archive.output.c_str(), std::ios::binary);
    //             lData = archive.header.getV2File(archiveFile, logoFile->second, archive.iv, archive.key);
    //             if (lData.empty()) {
    //                 return E_FAIL;
    //             }

    //             archiveAvailable = true;
    //             SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, archive.output.c_str(), NULL);
    //         } 
    //     }

    // return S_OK;

    archiveAvailable = false;
    lData.clear();
    m_filePath.clear();

    m_filePath = pszFileName;
    archive.output = WStringToString(m_filePath);

    int res = archive.getArchive();
    if (res == 0) {
        auto logoFile = archive.header.files.find(L"logo");
        if (logoFile != archive.header.files.end()) {
            std::ifstream archiveFile(archive.output.c_str(), std::ios::binary);
            lData = archive.header.getV2File(
                archiveFile,
                logoFile->second,
                archive.iv,
                archive.key
            );

            if (!lData.empty()) {
                archiveAvailable = true;
            }
        }
    }

    return S_OK;
}
STDMETHODIMP BappIconHandler::Save(LPCOLESTR, BOOL) { return E_NOTIMPL; }
STDMETHODIMP BappIconHandler::SaveCompleted(LPCOLESTR) { return E_NOTIMPL; }
STDMETHODIMP BappIconHandler::GetCurFile(LPOLESTR* ppszFileName) { return E_NOTIMPL; }

// IExtractIcon
STDMETHODIMP BappIconHandler::GetIconLocation(UINT, LPWSTR szIconFile, UINT cchMax, int* piIndex, UINT* pwFlags) {
    *pwFlags = GIL_NOTFILENAME | GIL_PERINSTANCE | GIL_DONTCACHE;

    if (szIconFile && cchMax > 0) szIconFile[0] = L'\0';
    *piIndex = 0;
    return S_OK;
}

STDMETHODIMP BappIconHandler::Extract(LPCWSTR, UINT, HICON* phiconLarge, HICON* phiconSmall, UINT nIconSize) {
    if (archiveAvailable) {
        UINT largeSize = 256;
        UINT smallSize = 32;

        const auto lIco = SysImageMgr::CreateIcon(lData, {largeSize, largeSize});
        const auto sIco = SysImageMgr::CreateIcon(lData, {smallSize,smallSize});
        *phiconLarge = lIco;
        *phiconSmall = sIco;
 
        return (*phiconLarge && *phiconSmall) ? S_OK : E_FAIL;
    }
    else return E_FAIL;
}
