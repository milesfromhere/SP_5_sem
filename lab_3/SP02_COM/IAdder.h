//**IAdder.h * *→ интерфейс сложения / вычитания
#pragma once
#include <objbase.h>
#include <Unknwn.h>

extern const IID IID_IAdder;

__interface IAdder : IUnknown {
	virtual HRESULT __stdcall Add(const double, const double, double&) = 0;
	virtual HRESULT __stdcall Sub(const double, const double, double&) = 0;
};