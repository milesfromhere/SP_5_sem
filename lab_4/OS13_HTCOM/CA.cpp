#include "pch.h"
#include "CA.h"
#include "SEQLOG.h"

static void CALLBACK SnapAsyncWrapper(LPVOID prm, DWORD dwTimerLowValue, DWORD dwTimerHighValue);

CA::CA() :m_Ref(1) 
{
	SEQ;
	InterlockedIncrement((LONG*)&g_Components);
	LOG("OS13::Adder g_Components = ", g_Components);
};

CA::~CA() 
{
	SEQ;
	InterlockedDecrement((LONG*)&g_Components);
	LOG("OS13::~Adder g_Components = ", g_Components);
};

HRESULT STDMETHODCALLTYPE CA::QueryInterface(REFIID riid, void** ppv)
{
	SEQ;
	HRESULT rc = S_OK;
	*ppv = NULL;
	if (riid == IID_IUnknown)
		*ppv = (IHTManipulator*)this;
	else if (riid == IID_IHTManipulator)
		*ppv = (IHTManipulator*)this;
	else if (riid == IID_IHTDataManipulator)
		*ppv = (IHTDataManipulator*)this;
	else if (riid == IID_IHTUtil)
		*ppv = (IHTUtil*)this;
	else if (riid == IID_IElement)
		*ppv = (IElement*)this;
	else rc = E_NOINTERFACE;

	if (rc == S_OK) this->AddRef();
	LOG("OS13::QueryInterface rc = ", rc);
	return rc;
};

ULONG STDMETHODCALLTYPE CA::AddRef(void) 
{
	SEQ;
	InterlockedIncrement((LONG*)&(this->m_Ref));
	LOG("OS13::AddRef m_Ref = ", this->m_Ref);
	return this->m_Ref;
};

ULONG STDMETHODCALLTYPE CA::Release(void) 
{
	SEQ;
	ULONG rc = this->m_Ref;
	if ((rc = InterlockedDecrement((LONG*)&(this->m_Ref))) == 0) delete this;
	LOG("OS13::Release rc = ", rc);
	return rc;
};

HRESULT STDMETHODCALLTYPE CA::Create(
	HTHANDLE** handle,
	int	  capacity,					// емкость хранилища
	int   secSnapshotInterval,		// переодичность сохранения в сек.
	int   maxKeyLength,             // максимальный размер ключа
	int   maxPayloadLength,			// максимальный размер данных
	const wchar_t* fileName)		// имя файла 
{
	HANDLE hf = CreateFile(
		fileName,
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hf == INVALID_HANDLE_VALUE)
		throw "Create or open file failed";

	int sizeMap = sizeof(HTHANDLE) + GetElementSize(maxKeyLength, maxPayloadLength) * capacity;
	HANDLE hm = CreateFileMapping(
		hf,
		NULL,
		PAGE_READWRITE,
		0, sizeMap,
		fileName);
	if (!hm)
		throw "Create File Mapping failed";

	LPVOID lp = MapViewOfFile(
		hm,
		FILE_MAP_ALL_ACCESS,
		0, 0, 0);
	if (!lp)
	{
		*handle = NULL;
		return S_OK;
	}

	ZeroMemory(lp, sizeMap);

	HTHANDLE* htHandle = new(lp) HTHANDLE(capacity, secSnapshotInterval, maxKeyLength, maxPayloadLength, fileName);
	htHandle->File = hf;
	htHandle->FileMapping = hm;
	htHandle->Addr = lp;
	htHandle->LastSnaptime = time(NULL);
	htHandle->Mutex = CreateMutex(NULL, FALSE, fileName);

	std::wstring eventName = std::wstring(fileName) + L"_ActiveEvent";
	htHandle->ActiveEvent = CreateEvent(
		NULL,
		TRUE,   // Manual Reset
		TRUE,   // Initially Signaled
		eventName.c_str());
	if (!htHandle->ActiveEvent)
		throw "Create ActiveEvent failed";

	SetEvent(htHandle->ActiveEvent);

	htHandle->SnapshotTimer = CreateWaitableTimer(0, FALSE, 0);
	LARGE_INTEGER Li{};
	Li.QuadPart = -(10000000 * htHandle->SecSnapshotInterval);
	SetWaitableTimer(htHandle->SnapshotTimer, &Li, htHandle->SecSnapshotInterval * 1000, SnapAsyncWrapper, htHandle, FALSE);

	*handle = htHandle;
	return S_OK;
}

static void CALLBACK SnapAsyncWrapper(LPVOID prm, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	CA* caInstance = new CA();
	caInstance->snapAsync(prm, dwTimerLowValue, dwTimerHighValue);
	delete caInstance;
}

void CALLBACK CA::snapAsync(LPVOID prm, DWORD, DWORD)
{
	HTHANDLE* htHandle = (HTHANDLE*)prm;
	BOOL rc;
	Snap(rc, htHandle);
	if (rc)
		std::cout << "SpanshotAsync success" << std::endl;
}

HRESULT STDMETHODCALLTYPE CA::Open
(
	HTHANDLE** handle,
	const wchar_t* fileName,
	bool isMapFile)         // имя файла
{
	HTHANDLE* htHandle;
	if (isMapFile)
	{
		htHandle = OpenHTFromMapName(fileName);
	}
	else
	{
		htHandle = OpenHTFromFile(fileName);
		if (htHandle)
		{
			htHandle->SnapshotTimer = CreateWaitableTimer(0, FALSE, 0);
			LARGE_INTEGER Li{};
			Li.QuadPart = -(10000000 * htHandle->SecSnapshotInterval);
			SetWaitableTimer(htHandle->SnapshotTimer, &Li, htHandle->SecSnapshotInterval * 1000, SnapAsyncWrapper, htHandle, FALSE);
		}
	}
	*handle = htHandle;
	return S_OK;
}

HTHANDLE* CA::OpenHTFromFile(const wchar_t* fileName)
{
	HANDLE hf = CreateFile(
		fileName,
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return NULL;

	HANDLE hm = CreateFileMapping(
		hf,
		NULL,
		PAGE_READWRITE,
		0, 0,
		fileName);
	if (!hm)
		return NULL;

	LPVOID lp = MapViewOfFile(
		hm,
		FILE_MAP_ALL_ACCESS,
		0, 0, 0);
	if (!lp)
		return NULL;

	HTHANDLE* htHandle = (HTHANDLE*)lp;
	htHandle->File = hf;
	htHandle->FileMapping = hm;
	htHandle->Addr = lp;
	htHandle->LastSnaptime = time(NULL);
	htHandle->Mutex = CreateMutex(NULL, FALSE, fileName);


	std::wstring eventName = std::wstring(fileName) + L"_ActiveEvent";
	htHandle->ActiveEvent = CreateEvent(NULL, TRUE, TRUE, eventName.c_str());
	if (!htHandle->ActiveEvent)
		return NULL;

	SetEvent(htHandle->ActiveEvent);

	return htHandle;
}

HTHANDLE* CA::OpenHTFromMapName(const wchar_t* fileName)
{
	HANDLE hm = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		false,
		fileName);
	if (!hm)
		return NULL;

	LPVOID lp = MapViewOfFile(
		hm,
		FILE_MAP_ALL_ACCESS,
		0, 0, 0);
	if (!lp)
		return NULL;

	HTHANDLE* htHandle = new HTHANDLE();
	memcpy(htHandle, lp, sizeof(HTHANDLE));
	htHandle->File = NULL;
	htHandle->FileMapping = hm;
	htHandle->Addr = lp;
	htHandle->SnapshotTimer = NULL;

	std::wstring eventName = std::wstring(fileName) + L"_ActiveEvent";
	htHandle->ActiveEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName.c_str());
	if (!htHandle->ActiveEvent) {
		WriteLastError(htHandle, "Open ActiveEvent failed");
		return NULL;
	}

	return htHandle;
}


HRESULT STDMETHODCALLTYPE CA::Get     //  читать элемент из хранилища
(
	Element** resultElement,
	HTHANDLE* htHandle,            // управление HT
	const Element* element)              // элемент 
{
	DWORD waitResult = WaitForSingleObject(htHandle->ActiveEvent, INFINITE);

	if (waitResult == WAIT_FAILED) {
		// Событие недействительно - переподключаемся
		CloseHandle(htHandle->ActiveEvent);

		std::wstring eventName = std::wstring(htHandle->FileName) + L"_ActiveEvent";
		htHandle->ActiveEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName.c_str());

		if (!htHandle->ActiveEvent) {
			WriteLastError(htHandle, "Storage is unavailable");
			*resultElement = NULL;
			return S_OK;
		}

		// Снова ждём после переподключения
		waitResult = WaitForSingleObject(htHandle->ActiveEvent, INFINITE);
	}

	if (waitResult != WAIT_OBJECT_0) {
		WriteLastError(htHandle, "Wait for ActiveEvent failed");
		*resultElement = NULL;
		return S_OK;
	}
	int index = findIndex(htHandle, element);
	if (index < 0)
	{
		WriteLastError(htHandle, "Not found element (GET)");
		*resultElement = NULL;
		return S_OK;
	}

	Element* foundElement = new Element();
	readFromMemory(htHandle, foundElement, index);

	*resultElement = foundElement;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CA::Insert		// добавить элемент в хранилище
(
	BOOL& rc,
	HTHANDLE* htHandle,            // управление HT
	const Element* element)              // элемент
{
	DWORD waitResult = WaitForSingleObject(htHandle->ActiveEvent, INFINITE);

	if (waitResult == WAIT_FAILED) {
		// Событие недействительно - переподключаемся
		CloseHandle(htHandle->ActiveEvent);

		std::wstring eventName = std::wstring(htHandle->FileName) + L"_ActiveEvent";
		htHandle->ActiveEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName.c_str());

		if (!htHandle->ActiveEvent) {
			WriteLastError(htHandle, "Storage is unavailable");
			rc = false;
			return S_OK;
		}

		// Снова ждём после переподключения
		waitResult = WaitForSingleObject(htHandle->ActiveEvent, INFINITE);
	}

	if (waitResult != WAIT_OBJECT_0) {
		WriteLastError(htHandle, "Wait for ActiveEvent failed");
		rc = false;
		return S_OK;
	}

	if (htHandle->Count >= htHandle->Capacity)
	{
		WriteLastError(htHandle, "Not found free memory");
		rc = false;
		return S_OK;
	}

	WaitForSingleObject(htHandle->Mutex, INFINITE);
	int freeIndex = findFreeIndex(htHandle, element);

	if (freeIndex < 0)
	{
		WriteLastError(htHandle, "Key already exists");
		ReleaseMutex(htHandle->Mutex);
		rc = false;
		return S_OK;
	}

	writeToMemory(htHandle, element, freeIndex);
	incrementCount(htHandle);
	ReleaseMutex(htHandle->Mutex);

	rc = true;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CA::Update     //  именить элемент в хранилище
(
	BOOL& rc,
	HTHANDLE* htHandle,            // управление HT
	const Element* oldElement,          // старый элемент (ключ, размер ключа)
	const void* newPayload,          // новые данные  
	int             newPayloadLength)     // размер новых данных
{
	DWORD waitResult = WaitForSingleObject(htHandle->ActiveEvent, INFINITE);

	if (waitResult == WAIT_FAILED) {
		// Событие недействительно - переподключаемся
		CloseHandle(htHandle->ActiveEvent);

		std::wstring eventName = std::wstring(htHandle->FileName) + L"_ActiveEvent";
		htHandle->ActiveEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName.c_str());

		if (!htHandle->ActiveEvent) {
			WriteLastError(htHandle, "Storage is unavailable");
			rc = false;
			return S_OK;
		}

		// Снова ждём после переподключения
		waitResult = WaitForSingleObject(htHandle->ActiveEvent, INFINITE);
	}

	if (waitResult != WAIT_OBJECT_0) {
		WriteLastError(htHandle, "Wait for ActiveEvent failed");
		rc = false;
		return S_OK;
	}

	WaitForSingleObject(htHandle->Mutex, INFINITE);
	int index = findIndex(htHandle, oldElement);
	if (index < 0)
	{
		WriteLastError(htHandle, "Not found element (UPDATE)");
		ReleaseMutex(htHandle->Mutex);
		rc = false;
		return S_OK;
	}

	Element* updateElement = new Element(oldElement, newPayload, newPayloadLength);
	writeToMemory(htHandle, updateElement, index);
	ReleaseMutex(htHandle->Mutex);

	rc = true;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CA::Delete      // удалить элемент в хранилище
(
	BOOL& rc,
	HTHANDLE* htHandle,            // управление HT (ключ)
	const Element* element)				 // элемент 
{
	DWORD waitResult = WaitForSingleObject(htHandle->ActiveEvent, INFINITE);

	if (waitResult == WAIT_FAILED) {
		// Событие недействительно - переподключаемся
		CloseHandle(htHandle->ActiveEvent);

		std::wstring eventName = std::wstring(htHandle->FileName) + L"_ActiveEvent";
		htHandle->ActiveEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName.c_str());

		if (!htHandle->ActiveEvent) {
			WriteLastError(htHandle, "Storage is unavailable");
			rc = false;
			return S_OK;
		}

		// Снова ждём после переподключения
		waitResult = WaitForSingleObject(htHandle->ActiveEvent, INFINITE);
	}

	if (waitResult != WAIT_OBJECT_0) {
		WriteLastError(htHandle, "Wait for ActiveEvent failed");
		rc = false;
		return S_OK;
	}

	WaitForSingleObject(htHandle->Mutex, INFINITE);
	int index = findIndex(htHandle, element);
	if (index < 0)
	{
		WriteLastError(htHandle, "Not found element (DELETE)");
		ReleaseMutex(htHandle->Mutex);
		rc = false;
		return S_OK;
	}

	clearMemoryByIndex(htHandle, index);
	decrementCount(htHandle);
	ReleaseMutex(htHandle->Mutex);

	rc = true;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CA::Snap         // выполнить Snapshot
(
	BOOL& rc,
	HTHANDLE* htHandle)           // управление HT (File, FileMapping)
{
	DWORD waitResult = WaitForSingleObject(htHandle->ActiveEvent, INFINITE);

	if (waitResult == WAIT_FAILED) {
		CloseHandle(htHandle->ActiveEvent);
		std::wstring eventName = std::wstring(htHandle->FileName) + L"_ActiveEvent";
		htHandle->ActiveEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName.c_str());

		if (!htHandle->ActiveEvent) {
			WriteLastError(htHandle, "Storage is unavailable");
			rc = false;
			return S_OK;
		}

		waitResult = WaitForSingleObject(htHandle->ActiveEvent, INFINITE);
	}

	if (waitResult != WAIT_OBJECT_0) {
		WriteLastError(htHandle, "Wait for ActiveEvent failed");
		rc = false;
		return S_OK;
	}

	WaitForSingleObject(htHandle->Mutex, INFINITE);
	if (!FlushViewOfFile(htHandle->Addr, NULL)) 
	{
		WriteLastError(htHandle, "Snapshot error");
		ReleaseMutex(htHandle->Mutex);
		rc = false;
		return S_OK;
	}
	htHandle->LastSnaptime = time(NULL);
	ReleaseMutex(htHandle->Mutex);

	rc = true;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CA::print                               // распечатать элемент 
(
	const Element* element)              // элемент 
{
	std::cout << "Element:" << std::endl;
	std::cout << "{" << std::endl;
	std::cout << "\t\"key\": \"" << (char*)element->key << "\"," << std::endl;
	std::cout << "\t\"keyLength\": " << element->keyLength << "," << std::endl;
	std::cout << "\t\"payload\": \"" << (char*)element->payload << "\"," << std::endl;
	std::cout << "\t\"payloadLength\": " << element->payloadLength << std::endl;
	std::cout << "}" << std::endl;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CA::Close
(
	BOOL& rc,
	const HTHANDLE* htHandle)
{
	HANDLE hf = htHandle->File;
	HANDLE hfm = htHandle->FileMapping;
	HANDLE hEvent = htHandle->ActiveEvent;
	HANDLE hTimer = htHandle->SnapshotTimer;
	HANDLE hMutex = htHandle->Mutex;

	// Только сервер (владелец файла) закрывает хранилище
	if (hf != NULL && hTimer != NULL) {
		// Отменяем таймер снапшотов
		CancelWaitableTimer(hTimer);

		// Ждём освобождения мьютекса (завершения операций)
		DWORD mutexResult = WaitForSingleObject(hMutex, 3000);
		if (mutexResult == WAIT_OBJECT_0) {
			// Блокируем новые операции
			ResetEvent(hEvent);
			ReleaseMutex(hMutex);
		}

		Sleep(500);  // Даём время клиентам обнаружить сброс
	}

	// Отключаем маппинг (и для сервера, и для клиентов)
	if (htHandle->Addr)
		UnmapViewOfFile(htHandle->Addr);

	// Закрываем дескрипторы
	if (hEvent)
		CloseHandle(hEvent);

	if (hfm)
		CloseHandle(hfm);

	// Только сервер закрывает мьютекс и файл
	if (hf != NULL) {
		if (hMutex)
			CloseHandle(hMutex);
		CloseHandle(hf);
	}

	rc = true;
	return S_OK;
}



int CA::GetElementSize(int maxKeyLength, int maxPayloadLength)
{
	return maxKeyLength + maxPayloadLength + sizeof(int) * 2;
}

int CA::hashFunction(const char* key, int capacity)
{
	unsigned long i = 0;
	for (int j = 0; key[j]; j++)
		i += key[j];
	return i % capacity;
}

int CA::nextHash(int currentHash, const char* key, int capacity)
{
	return ++currentHash;
}

int CA::findFreeIndex(
	const HTHANDLE* htHandle,           // управление HT
	const Element* element)				// элемент
{
	int index = hashFunction((char*)element->key, htHandle->Capacity);

	Element* foundElement = new Element();
	do
	{
		if (index >= htHandle->Capacity || foundElement->key != NULL &&
			memcmp(foundElement->key, element->key, element->keyLength) == NULL)
		{
			index = -1;
			break;
		}

		readFromMemory(htHandle, foundElement, index);
		index = nextHash(index, (char*)element->key, htHandle->Capacity);
	} while (
		foundElement->keyLength != NULL &&
		foundElement->payloadLength != NULL);

	delete foundElement;
	return index - 1;
}

int CA::findIndex(
	const HTHANDLE* htHandle,           // управление HT
	const Element* element)				// элемент
{
	int index = hashFunction((char*)element->key, htHandle->Capacity);

	Element* foundElement = new Element();
	do
	{
		if (index >= htHandle->Capacity)
		{
			index = -1;
			break;
		}

		readFromMemory(htHandle, foundElement, index);
		index = nextHash(index, (char*)element->key, htHandle->Capacity);
	} while (
		memcmp(foundElement->key, element->key, element->keyLength) != NULL);

	delete foundElement;
	return index - 1;
}

BOOL CA::writeToMemory(const HTHANDLE* const htHandle, const Element* const element, int index)
{
	LPVOID lp = htHandle->Addr;

	lp = (HTHANDLE*)lp + 1;
	lp = (BYTE*)lp + GetElementSize(htHandle->MaxKeyLength, htHandle->MaxPayloadLength) * index;

	memcpy(lp, element->key, element->keyLength);
	lp = (BYTE*)lp + htHandle->MaxKeyLength;
	memcpy(lp, &element->keyLength, sizeof(int));
	lp = (int*)lp + 1;
	memcpy(lp, element->payload, element->payloadLength);
	lp = (BYTE*)lp + htHandle->MaxPayloadLength;
	memcpy(lp, &element->payloadLength, sizeof(int));

	return true;
}

int CA::incrementCount(HTHANDLE* const htHandle)
{
	return ++htHandle->Count;
}

Element* CA::readFromMemory(const HTHANDLE* const htHandle, Element* const element, int index)
{
	LPVOID lp = htHandle->Addr;

	lp = (HTHANDLE*)lp + 1;
	lp = (BYTE*)lp + GetElementSize(htHandle->MaxKeyLength, htHandle->MaxPayloadLength) * index;

	element->key = lp;
	lp = (BYTE*)lp + htHandle->MaxKeyLength;
	element->keyLength = *(int*)lp;
	lp = (int*)lp + 1;
	element->payload = lp;
	lp = (BYTE*)lp + htHandle->MaxPayloadLength;
	element->payloadLength = *(int*)lp;

	return element;
}

BOOL CA::clearMemoryByIndex(const HTHANDLE* const htHandle, int index)
{
	LPVOID lp = htHandle->Addr;
	int sizeElement = GetElementSize(htHandle->MaxKeyLength, htHandle->MaxPayloadLength);

	lp = (HTHANDLE*)lp + 1;
	lp = (BYTE*)lp + sizeElement * index;

	ZeroMemory(lp, sizeElement);

	return true;
}

int CA::decrementCount(HTHANDLE* const htHandle)
{
	return --htHandle->Count;
}

const char* CA::WriteLastError(HTHANDLE* const htHandle, const char* msg)
{
	memcpy(htHandle->LastErrorMessage, msg, sizeof(htHandle->LastErrorMessage));
	return htHandle->LastErrorMessage;
}

HRESULT STDMETHODCALLTYPE CA::getLastError  // получить сообщение о последней ошибке
(
	const char** error,
	const HTHANDLE* htHandle                         // управление HT
)
{
	*error = htHandle->LastErrorMessage;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CA::CreateElementGet
(
	Element** element,
	const void* key,
	int keyLength
)
{
	*element = new Element(key, keyLength);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CA::CreateElementInsert
(
	Element** element,
	const void* key,
	int keyLength,
	const void* payload,
	int payloadLength
)
{
	*element = new Element(key, keyLength, payload, payloadLength);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CA::CreateElementUpdate
(
	Element** element,
	const Element* oldElement,
	const void* newPayload,
	int newPayloadLength
)
{
	*element = new Element(oldElement, newPayload, newPayloadLength);
	return S_OK;
}