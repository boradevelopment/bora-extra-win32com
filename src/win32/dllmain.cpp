// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <string>
//#pragma comment(lib, "gdiplus.lib")
#include <gdiplus.h>
#include "IconHandler.h"
using namespace Gdiplus;

extern "C" IMAGE_DOS_HEADER __ImageBase;
ULONG_PTR g_gdiplusToken;

#include <Windows.h>
#include <Unknwn.h>

class ClassFactory : public IClassFactory {
private:
    LONG m_refCount;
    REFCLSID m_clsid;
    HRESULT(*m_createInstance)(REFIID riid, void** ppv);

public:
    ClassFactory(REFCLSID clsid, HRESULT(*createFunc)(REFIID, void**))
        : m_refCount(1), m_clsid(clsid), m_createInstance(createFunc) {
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown || riid == IID_IClassFactory) {
            *ppv = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&m_refCount);
    }

    STDMETHODIMP_(ULONG) Release() override {
        LONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) delete this;
        return count;
    }

    // IClassFactory
    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) override {
        if (pUnkOuter != nullptr) return CLASS_E_NOAGGREGATION;
        return m_createInstance(riid, ppv);
    }

    STDMETHODIMP LockServer(BOOL) override {
        return S_OK;
    }
};

HRESULT CreateBappIconHandlerInstance(REFIID riid, void** ppv) {
    *ppv = nullptr;
    auto* handler = new (std::nothrow) BappIconHandler();
    if (!handler) return E_OUTOFMEMORY;
    HRESULT hr = handler->QueryInterface(riid, ppv);
    handler->Release();  // balance ref count
    return hr;
}



BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID /*lpvReserved*/) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH: {
        // Disable thread notifications for performance if you don't need them
        DisableThreadLibraryCalls(hinstDLL);

        GdiplusStartupInput gdiStartupInput;
        GdiplusStartup(&g_gdiplusToken, &gdiStartupInput, nullptr);

        break;
    }

        case DLL_PROCESS_DETACH:
            GdiplusShutdown(g_gdiplusToken);
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            // Usually no code needed here
            break;
    }
    return TRUE;  
}


HRESULT RegisterKeyAndValue(HKEY root, LPCWSTR subKey, LPCWSTR valueName, LPCWSTR data) {
    HKEY hKey;
    if (RegCreateKeyExW(root, subKey, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
        return E_FAIL;

    if (data != nullptr) {
        if (RegSetValueExW(hKey, valueName, 0, REG_SZ, (const BYTE*)data, ((DWORD)wcslen(data) + 1) * sizeof(WCHAR)) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return E_FAIL;
        }
    }

    RegCloseKey(hKey);
    return S_OK;
}

STDAPI  DllRegisterServer() {
    WCHAR clsidStr[64];
    StringFromGUID2(CLSID_BappIconHandler, clsidStr, ARRAYSIZE(clsidStr));

    WCHAR modulePath[MAX_PATH];
    GetModuleFileNameW((HMODULE)&__ImageBase, modulePath, MAX_PATH);  // __ImageBase gives the DLL handle

    // 1. Register CLSID
    std::wstring clsidKey = L"CLSID\\";
    clsidKey += clsidStr;

    HRESULT hr = RegisterKeyAndValue(HKEY_CLASSES_ROOT, clsidKey.c_str(), nullptr, L"Bapp Icon Handler");
    if (FAILED(hr)) return hr;

    std::wstring inprocKey = clsidKey + L"\\InprocServer32";
    hr = RegisterKeyAndValue(HKEY_CLASSES_ROOT, inprocKey.c_str(), nullptr, modulePath);
    if (FAILED(hr)) return hr;

    hr = RegisterKeyAndValue(HKEY_CLASSES_ROOT, inprocKey.c_str(), L"ThreadingModel", L"Apartment");
    if (FAILED(hr)) return hr;

    // 2. Associate with .bapp extension
    hr = RegisterKeyAndValue(HKEY_CLASSES_ROOT, L".bapp", nullptr, L"BappFile");
    if (FAILED(hr)) return hr;

    // 3. Tell a name
    hr = RegisterKeyAndValue(HKEY_CLASSES_ROOT, L"BappFile", nullptr, L"BORA Application");
    if (FAILED(hr)) return hr;

    // 4. Register shell extension under .bapp
    std::wstring iconHandlerKey = L"BappFile\\shellex\\IconHandler";
    hr = RegisterKeyAndValue(HKEY_CLASSES_ROOT, iconHandlerKey.c_str(), nullptr, clsidStr);
    if (FAILED(hr)) return hr;
    // use BORA_RT_PATH in future
    RegisterKeyAndValue(HKEY_CLASSES_ROOT, L"BappFile\\shell\\open\\command", nullptr, L"\"C:\\bora\\runtime\\builds\\debug\\BORA.exe\" \"%1\" %*");
    return S_OK; 
}

STDAPI   DllUnregisterServer() {
    WCHAR clsidStr[64];
    StringFromGUID2(CLSID_BappIconHandler, clsidStr, ARRAYSIZE(clsidStr));

    std::wstring clsidKey = L"CLSID\\";
    clsidKey += clsidStr;
    SHDeleteKeyW(HKEY_CLASSES_ROOT, clsidKey.c_str());
    SHDeleteKeyW(HKEY_CLASSES_ROOT, L"BappFile");
    SHDeleteKeyW(HKEY_CLASSES_ROOT, L".bapp");

    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
    if (rclsid == CLSID_BappIconHandler) {
        ClassFactory* factory = new (std::nothrow) ClassFactory(rclsid, CreateBappIconHandlerInstance);
        if (!factory) return E_OUTOFMEMORY;
        HRESULT hr = factory->QueryInterface(riid, ppv);
        factory->Release();  // balance ref count
        return hr;
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI  DllCanUnloadNow() {
    return S_FALSE;  // or S_OK if your global ref count is 0
}
