//#### **4. SP02.cpp** - Реализация класса SP02
#include "pch.h"

SP02::SP02() : counter(1) {}

SP02::~SP02() {}

HRESULT __stdcall SP02::QueryInterface(const IID& iid, void** ppv) {
	if (!ppv) return E_POINTER;
	*ppv = nullptr;

	if (iid == IID_IUnknown || iid == IID_IAdder) {
		*ppv = static_cast<IAdder*>(this);
	}
	else if (iid == IID_IMultiplier) {
		*ppv = static_cast<IMultiplier*>(this);
	}
	else {
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG __stdcall SP02::AddRef() {
	return InterlockedIncrement(&counter);
}

ULONG __stdcall SP02::Release() {
	ULONG res = InterlockedDecrement(&counter);
	if (res == 0) {
		delete this;
	}
	return res;
}


HRESULT __stdcall SP02::Add(const double x, const double y, double& c) {
	c = x + y;
	return S_OK;
}

HRESULT __stdcall SP02::Sub(const double x, const double y, double& c) {
	c = x - y;
	return S_OK;
}

HRESULT __stdcall SP02::Mul(const double x, const double y, double& c) {
	c = x * y;
	return S_OK;
}

HRESULT __stdcall SP02::Div(const double x, const double y, double& c) {
	if (y == 0.0) return E_INVALIDARG;
	c = x / y;
	return S_OK;
}
