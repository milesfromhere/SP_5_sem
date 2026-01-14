//#### **5. MathFactory.h/cpp** - Фабрика классов
#include "pch.h"
#include "MathFactory.h"

ULONG g_lObjs = 0;
ULONG g_lLocks = 0;

MathFactory::MathFactory() : m_lRef(1) {}

MathFactory::~MathFactory() {};


// Методы IUnknown
STDMETHODIMP_(HRESULT __stdcall) MathFactory::QueryInterface(REFIID riid, LPVOID* ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (riid == IID_IUnknown) {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (riid == IID_IClassFactory) {
        *ppv = static_cast<IClassFactory*>(this);
    }
    else {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) MathFactory::AddRef() {
    return InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG) MathFactory::Release() {
    ULONG res = InterlockedDecrement(&m_lRef);
    if (res == 0) { delete this; }
    return res;
}



// Экземпляр класса
STDMETHODIMP MathFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* ppvObj) {

    HRESULT hr = E_UNEXPECTED;

    SP02* pSP = nullptr;

    if (pUnkOuter != NULL)
        hr = CLASS_E_NOAGGREGATION;
    else if ((pSP = new SP02()) == NULL)
        hr = E_OUTOFMEMORY;
    else {
        hr = pSP->QueryInterface(riid, ppvObj);
        pSP->Release();
    }

    if (FAILED(hr))
        delete pSP;

    return hr;
}

// lockServer(true) запрещает разрушение экземпляра фабрики классов
// lockServer(false) разрешает
STDMETHODIMP  MathFactory::LockServer(BOOL fLock) {
    if (fLock)
        InterlockedIncrement(&g_lLocks);
    else
        InterlockedDecrement(&g_lLocks);

    return S_OK;
}