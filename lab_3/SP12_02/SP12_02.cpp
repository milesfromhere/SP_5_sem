#include <iostream>      
#include <Windows.h>      // для CoCreateInstance, HRESULT, CLSID
#include <objbase.h>      // для CoInitialize, CoFreeUnusedLibraries
#include <initguid.h>     // для DEFINE_GUID
#include "../SP02_COM/IAdder.h"
#include "../SP02_COM/IMultiplier.h"
    

DEFINE_GUID(CLSID_SP02, 0x02fe8a5e, 0x281b, 0x4e3a, 0x8f, 0x87, 0x6c, 0x54, 0x4d, 0xed, 0xa9, 0x94);
DEFINE_GUID(IID_IAdder, 0xe04d1823, 0x1f2e, 0x114d, 0x22, 0xae, 0x4e, 0xab, 0x4c, 0xed, 0x64, 0xd8);
DEFINE_GUID(IID_IMultiplier, 0xe0d12322, 0xad3a, 0x294e, 0x42, 0x98, 0x8d, 0x3c, 0x3e, 0xed, 0x64, 0xd8);



#define IERR(s)    std::cout<<"error "<<s<<std::endl
#define IRES(s,r)  std::cout<<s<<r<<std::endl

IAdder* pIAdder = nullptr;
IMultiplier* pMultiplier = nullptr;


int main()
{
	IUnknown* pIUnknown = NULL;
	CoInitialize(NULL);                        // инициализация библиотеки OLE32
	HRESULT hr0 = CoCreateInstance(CLSID_SP02, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pIUnknown);
	if (SUCCEEDED(hr0) && pIUnknown)
	{
		std::cout << "CoCreateInstance succeeded" << std::endl;
		if (SUCCEEDED(pIUnknown->QueryInterface(IID_IAdder, (void**)&pIAdder)))
		{
			{
				double z = 0.0;
				if (!SUCCEEDED(pIAdder->Add(2.0, 3.0, z)))  IERR("IAdder::Add");
				else IRES("IAdder::Add = ", z);
			}
			{
				double z = 0.0;
				if (!SUCCEEDED(pIAdder->Sub(2.0, 3.0, z)))  IERR("IAdder::Sub");
				else IRES("IAdder::Sub = ", z);
			}
			if (SUCCEEDED(pIAdder->QueryInterface(IID_IMultiplier, (void**)&pMultiplier)))
			{
				{
					double z = 0.0;
					if (!SUCCEEDED(pMultiplier->Mul(2.0, 3.0, z))) IERR("IMultiplier::Mul");
					else IRES("Multiplier::Mul = ", z);
				}
				{
					double z = 0.0;
					if (!SUCCEEDED(pMultiplier->Div(2.0, 3.0, z))) IERR("IMultiplier::Div");
					else IRES("IMultiplier::Div = ", z);
				}
				if (SUCCEEDED(pMultiplier->QueryInterface(IID_IAdder, (void**)&pIAdder)))
				{
					double z = 0.0;
					if (!SUCCEEDED(pIAdder->Add(2.0, 3.0, z))) IERR("IAdder::Add");
					else IRES("IAdder::Add = ", z);
					pIAdder->Release();
				}
				else  IERR("IMultiplier->IAdder");
				pMultiplier->Release();
			}
			else IERR("IAdder->IMultiplier");
			pIAdder->Release();
			pIUnknown->Release();
		}
		else  IERR("IAdder");
	}
	else {
		std::cout << "CoCreateInstance error, hr=" << std::hex << hr0 << std::endl;
		if (pIUnknown) pIUnknown->Release(); // защита от nullptr
	}
	CoFreeUnusedLibraries();                   // завершение работы с библиотекой      
	CoUninitialize();
	return 0;
}


