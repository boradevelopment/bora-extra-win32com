#pragma once

#include <Windows.h>
#include <ShlObj.h> // For IExtractIconW, IPersistFile
#include <string>
#include "TAZA.h"

class BappIconHandler : public IExtractIconW, public IPersistFile {
private:
    ULONG m_refCount;
    std::wstring m_filePath;
    V2Archive archive;
    std::vector<uint8_t> lData;
    std::wstring path;
    bool archiveAvailable;
public:
    BappIconHandler();
    ~BappIconHandler();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IPersist
    STDMETHODIMP GetClassID(CLSID* pClassID);

    // IPersistFile
    STDMETHODIMP IsDirty();
    STDMETHODIMP Load(LPCOLESTR pszFileName, DWORD dwMode);
    STDMETHODIMP Save(LPCOLESTR pszFileName, BOOL fRemember);
    STDMETHODIMP SaveCompleted(LPCOLESTR pszFileName);
    STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName);

    // IExtractIcon
    STDMETHODIMP GetIconLocation(UINT uFlags, LPWSTR szIconFile, UINT cchMax, int* piIndex, UINT* pwFlags);
    STDMETHODIMP Extract(LPCWSTR pszFile, UINT nIconIndex, HICON* phiconLarge, HICON* phiconSmall, UINT nIconSize);
};
