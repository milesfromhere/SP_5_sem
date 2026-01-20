#pragma once
#include "../OS15_HTCOM/Interface.h"

#define OS15_HTCOM_HANDEL void*

namespace OS15_HTCOM
{
	OS15_HTCOM_HANDEL Init();                                // инициализация OS12
	//   if CoCreateInstance(... IID_Unknown)!= succesfull --> throw (int)HRESULT  
	namespace HT
	{
		ht::HtHandle* create(OS15_HTCOM_HANDEL h, int capacity, int secSnapshotInterval, int maxKeyLength, int maxPayloadLength, const wchar_t* htUsersGroup, const wchar_t* fileName);
		ht::HtHandle* open(OS15_HTCOM_HANDEL h, const wchar_t* fileName, bool isMapFile = false);
		ht::HtHandle* open(OS15_HTCOM_HANDEL h, const wchar_t* fileName, const wchar_t* htUser, const wchar_t* htPassword, bool isMapFile = false);
		BOOL snap(OS15_HTCOM_HANDEL h, ht::HtHandle* htHandle);
		BOOL close(OS15_HTCOM_HANDEL h, ht::HtHandle* htHandle);
		BOOL insert(OS15_HTCOM_HANDEL h, ht::HtHandle* htHandle, const ht::Element* element);
		BOOL removeOne(OS15_HTCOM_HANDEL h, ht::HtHandle* htHandle, const ht::Element* element);
		ht::Element* get(OS15_HTCOM_HANDEL h, ht::HtHandle* htHandle, const ht::Element* element);
		BOOL update(OS15_HTCOM_HANDEL h, ht::HtHandle* htHandle, const ht::Element* oldElement, const void* newPayload, int newPayloadLength);
		const char* getLastError(OS15_HTCOM_HANDEL h, ht::HtHandle* htHandle);
		void print(OS15_HTCOM_HANDEL h, const ht::Element* element);
	}
	namespace Element
	{
		ht::Element* createGetElement(OS15_HTCOM_HANDEL h, const void* key, int keyLength);
		ht::Element* createInsertElement(OS15_HTCOM_HANDEL h, const void* key, int keyLength, const void* payload, int  payloadLength);
		ht::Element* createUpdateElement(OS15_HTCOM_HANDEL h, const ht::Element* oldElement, const void* newPayload, int  newPayloadLength);
	}
	void Dispose(OS15_HTCOM_HANDEL h);                       // завершение работы с OS12                  
}