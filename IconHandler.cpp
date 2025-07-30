#include "pch.h"
#include "IconHandler.h"
#include <Shlwapi.h>
#include <StrSafe.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;


HICON LoadImageToIcon(const BYTE* buffer, UINT size, BOOL largeIcon) {
    if (!buffer || size == 0)
        return nullptr;

    // Copy buffer to global memory
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hMem) return nullptr;

    void* pData = GlobalLock(hMem);
    memcpy(pData, buffer, size);
    GlobalUnlock(hMem);

    // Create a stream from global memory
    IStream* pStream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(hMem, TRUE, &pStream))) {
        GlobalFree(hMem);
        return nullptr;
    }

    // Load the image from stream using GDI+
    Bitmap* bitmap = Bitmap::FromStream(pStream);
    pStream->Release();

    if (!bitmap || bitmap->GetLastStatus() != Ok) {
        delete bitmap;
        return nullptr;
    }

    // Create mask for icon transparency
    HBITMAP hBitmap = nullptr;
    if (bitmap->GetHBITMAP(Color(0, 0, 0, 0), &hBitmap) != Ok) {
        delete bitmap;
        return nullptr;
    }

    // Create a dummy mask bitmap (black + opaque)
    BITMAP bmpInfo = {};
    GetObject(hBitmap, sizeof(BITMAP), &bmpInfo);
    HBITMAP hMonoMask = CreateBitmap(bmpInfo.bmWidth, bmpInfo.bmHeight, 1, 1, nullptr);

    // Create ICONINFO
    ICONINFO iconInfo = {};
    iconInfo.fIcon = TRUE;
    iconInfo.hbmMask = hMonoMask;
    iconInfo.hbmColor = hBitmap;

    // Create final HICON
    HICON hIcon = CreateIconIndirect(&iconInfo);

    // Clean up
    DeleteObject(hBitmap);
    DeleteObject(hMonoMask);
    delete bitmap;

    return hIcon;
}


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
    m_filePath = pszFileName;
    archive.output = WStringToString(m_filePath);


    int res = archive.getArchive();
    if (res != 0) {
        archive.~V2Archive();
        return E_FAIL;
        archiveAvailable = false;
    }

    auto logoFile = archive.header.files.find("logo");
    if (logoFile != archive.header.files.end()) {
        std::ifstream archiveFile(archive.output.c_str(), std::ios::binary);
        lData = archive.header.getV2File(archiveFile, logoFile->second, archive.iv, archive.key);
        if (lData.empty()) {
            archiveAvailable = false;
            return E_FAIL;
        }
        return S_OK;
        archiveAvailable = true;
    }
    else {
        archiveAvailable = false;
        return E_FAIL;
    }

}
STDMETHODIMP BappIconHandler::Save(LPCOLESTR, BOOL) { return E_NOTIMPL; }
STDMETHODIMP BappIconHandler::SaveCompleted(LPCOLESTR) { return E_NOTIMPL; }
STDMETHODIMP BappIconHandler::GetCurFile(LPOLESTR* ppszFileName) { return E_NOTIMPL; }

// IExtractIcon
STDMETHODIMP BappIconHandler::GetIconLocation(UINT, LPWSTR szIconFile, UINT cchMax, int* piIndex, UINT* pwFlags) {
    // Tell the shell not to cache this icon and that we're not returning a filename
    *pwFlags = GIL_DONTCACHE | GIL_NOTFILENAME;

    // Optionally, set dummy values because GIL_NOTFILENAME means we won't be returning a path
    if (szIconFile && cchMax > 0) {
        szIconFile[0] = L'\0';
    }
    *piIndex = 0;

    return S_OK;
}

STDMETHODIMP BappIconHandler::Extract(LPCWSTR, UINT, HICON* phiconLarge, HICON* phiconSmall, UINT) {
    if (archiveAvailable) {
        auto icoLarge = LoadImageToIcon(lData.data(), lData.size(), TRUE);
        auto icoSmall = LoadImageToIcon(lData.data(), lData.size(), TRUE);
        *phiconLarge = icoLarge;
        *phiconSmall = icoSmall;

     
        return (*phiconLarge && *phiconSmall) ? S_OK : E_FAIL;
    }
    else return E_FAIL;
}
