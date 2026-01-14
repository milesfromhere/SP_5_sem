//### **2. IMultiplier.h** - Интерфейс для умножения/деления
#pragma once
#include <objbase.h>
#include <Unknwn.h>

extern const IID IID_IMultiplier;

__interface IMultiplier : IUnknown {
	virtual HRESULT __stdcall Mul(const double, const double, double&) = 0;
	virtual HRESULT __stdcall Div(const double, const double, double&) = 0;
};