#include "pch.h"
#include "SP12_LIB.h"
#include <Windows.h>
#include <objbase.h>
#include "../SP02_COM/IAdder.h"
#include "../SP02_COM/IMultiplier.h"
#include <initguid.h>     // для DEFINE_GUID


DEFINE_GUID(CLSID_SP02, 0x02fe8a5e, 0x281b, 0x4e3a, 0x8f, 0x87, 0x6c, 0x54, 0x4d, 0xed, 0xa9, 0x94);
DEFINE_GUID(IID_IAdder, 0xe04d1823, 0x1f2e, 0x114d, 0x22, 0xae, 0x4e, 0xab, 0x4c, 0xed, 0x64, 0xd8);
DEFINE_GUID(IID_IMultiplier, 0xe0d12322, 0xad3a, 0x294e, 0x42, 0x98, 0x8d, 0x3c, 0x3e, 0xed, 0x64, 0xd8);

struct OS12Context {
    IUnknown* pIUnknown;
};

OS12HANDEL OS12::Init() {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) throw (int)hr;

    OS12Context* ctx = new OS12Context;
    hr = CoCreateInstance(CLSID_SP02, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&ctx->pIUnknown);
    if (FAILED(hr)) {
        CoUninitialize();
        delete ctx;
        throw (int)hr;
    }

    return ctx;
}

double OS12::Adder::Add(OS12HANDEL h, double x, double y) {
    OS12Context* ctx = (OS12Context*)h;
    IAdder* pIAdder = nullptr;
    HRESULT hr = ctx->pIUnknown->QueryInterface(IID_IAdder, (void**)&pIAdder);
    if (FAILED(hr)) throw (int)hr;

    double result = 0.0;
    hr = pIAdder->Add(x, y, result);
    pIAdder->Release();
    if (FAILED(hr)) throw (int)hr;

    return result;
}

double OS12::Adder::Sub(OS12HANDEL h, double x, double y) {
    OS12Context* ctx = (OS12Context*)h;
    IAdder* pIAdder = nullptr;
    HRESULT hr = ctx->pIUnknown->QueryInterface(IID_IAdder, (void**)&pIAdder);
    if (FAILED(hr)) throw (int)hr;

    double result = 0.0;
    hr = pIAdder->Sub(x, y, result);
    pIAdder->Release();
    if (FAILED(hr)) throw (int)hr;

    return result;
}

double OS12::Multiplier::Mul(OS12HANDEL h, double x, double y) {
    OS12Context* ctx = (OS12Context*)h;
    IMultiplier* pIMult = nullptr;
    HRESULT hr = ctx->pIUnknown->QueryInterface(IID_IMultiplier, (void**)&pIMult);
    if (FAILED(hr)) throw (int)hr;

    double result = 0.0;
    hr = pIMult->Mul(x, y, result);
    pIMult->Release();
    if (FAILED(hr)) throw (int)hr;

    return result;
}

double OS12::Multiplier::Div(OS12HANDEL h, double x, double y) {
    OS12Context* ctx = (OS12Context*)h;
    IMultiplier* pIMult = nullptr;
    HRESULT hr = ctx->pIUnknown->QueryInterface(IID_IMultiplier, (void**)&pIMult);
    if (FAILED(hr)) throw (int)hr;

    double result = 0.0;
    hr = pIMult->Div(x, y, result);
    pIMult->Release();
    if (FAILED(hr)) throw (int)hr;

    return result;
}

void OS12::Dispose(OS12HANDEL h) {
    OS12Context* ctx = (OS12Context*)h;
    if (ctx->pIUnknown) ctx->pIUnknown->Release();
    delete ctx;
    CoUninitialize();
}
