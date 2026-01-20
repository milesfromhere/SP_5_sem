#include "pch.h"

#include "HT.h"

namespace ht
{
	HtHandle::HtHandle()
	{
		this->capacity = 0;
		this->secSnapshotInterval = 0;
		this->maxKeyLength = 0;
		this->maxPayloadLength = 0;
		ZeroMemory(this->fileName, sizeof(this->fileName));
		this->file = NULL;
		this->fileMapping = NULL;
		this->addr = NULL;
		ZeroMemory(this->lastErrorMessage, sizeof(this->lastErrorMessage));
		this->lastSnaptime = 0;

		this->count = 0;
	}

	HtHandle::HtHandle(int capacity, int secSnapshotInterval, int maxKeyLength, int maxPayloadLength, const wchar_t* htUsersGroup, const wchar_t* fileName) : HtHandle()
	{
		this->capacity = capacity;
		this->secSnapshotInterval = secSnapshotInterval;
		this->maxKeyLength = maxKeyLength;
		this->maxPayloadLength = maxPayloadLength;
		memcpy(this->fileName, fileName, sizeof(this->fileName));
		memcpy(this->htUsersGroup, htUsersGroup, sizeof(this->htUsersGroup));
	}

	HtHandle* create(
		int	  capacity,					// емкость хранилища
		int   secSnapshotInterval,		// переодичность сохранения в сек.
		int   maxKeyLength,             // максимальный размер ключа
		int   maxPayloadLength,			// максимальный размер данных
		const wchar_t* htUsersGroup,	// имя группы OS-пользователей
		const wchar_t* fileName)		// имя файла 
	{
		if (canCreateHtFor(htUsersGroup))
		{
			HtHandle* htHandle = createHt(capacity, secSnapshotInterval, maxKeyLength, maxPayloadLength, htUsersGroup, fileName);
			runSnapshotTimer(htHandle);

			return htHandle;
		}
		else
		{
			return NULL;
		}
	}

	HtHandle* createHt(
		int	  capacity,					// емкость хранилища
		int   secSnapshotInterval,		// переодичность сохранения в сек.
		int   maxKeyLength,             // максимальный размер ключа
		int   maxPayloadLength,			// максимальный размер данных
		const wchar_t* htUsersGroup,	// имя группы OS-пользователей
		const wchar_t* fileName)		// имя файла 
	{
		HANDLE hf = CreateFile(
			fileName,
			GENERIC_WRITE | GENERIC_READ,
			NULL,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (hf == INVALID_HANDLE_VALUE)
			throw "create or open file failed";

		int sizeMap = sizeof(HtHandle) + getSizeElement(maxKeyLength, maxPayloadLength) * capacity;

		std::wstring FileMappingName = L"Global\\"; FileMappingName += fileName; FileMappingName += L"-filemapping";
		SECURITY_ATTRIBUTES SA = getSecurityAttributes();
		HANDLE hm = CreateFileMapping(
			hf,
			&SA,
			PAGE_READWRITE,
			0, sizeMap,
			FileMappingName.c_str());
		if (!hm)
			throw "create File Mapping failed";

		LPVOID lp = MapViewOfFile(
			hm,
			FILE_MAP_ALL_ACCESS,
			0, 0, 0);
		if (!lp)
			return NULL;

		ZeroMemory(lp, sizeMap);

		HtHandle* htHandle = new(lp) HtHandle(capacity, secSnapshotInterval, maxKeyLength, maxPayloadLength, htUsersGroup, fileName);
		htHandle->file = hf;
		htHandle->fileMapping = hm;
		htHandle->addr = lp;
		htHandle->lastSnaptime = time(NULL);
		std::wstring MutexName = L"Global\\"; MutexName += fileName; MutexName += L"-mutex";
		htHandle->mutex = CreateMutex(
			&SA,
			FALSE,
			MutexName.c_str());

		return htHandle;
	}

	bool canCreateHtFor(const wchar_t* htUsersGroup)
	{
		if (isExistUsersGroup(htUsersGroup))
		{
			if (isCurrentUserBelongTo(htUsersGroup) || isCurrentUserBelongTo(L"Администраторы"))
				return true;
		}

		return false;
	}

	HtHandle* open
	(
		const wchar_t* fileName,        // имя файла
		const wchar_t* htUser,			// HT-пользователь
		const wchar_t* htPassword,		// пароль
		bool isMapFile)					// true если открыть fileMapping; false если открыть файл; по умолчанию false
	{
		HtHandle* htHandle = openWithoutAuth(fileName, isMapFile);

		if (htHandle)
		{
			if (!canOpenHt(htHandle, htUser, htPassword))
			{
				close(htHandle);
				return NULL;
			}
		}

		return htHandle;
	}

	HtHandle* open
	(
		const wchar_t* fileName,         // имя файла
		bool isMapFile)					// true если открыть fileMapping; false если открыть файл; по умолчанию false
	{
		HtHandle* htHandle = openWithoutAuth(fileName, isMapFile);

		if (htHandle)
		{
			if (!canOpenHt(htHandle))
			{
				close(htHandle);
				return NULL;
			}
		}

		return htHandle;
	}

	HtHandle* openWithoutAuth
	(
		const wchar_t* fileName,         // имя файла
		bool isMapFile)					// true если открыть fileMapping; false если открыть файл; по умолчанию false
	{
		HtHandle* htHandle = NULL;

		if (isMapFile)
		{
			htHandle = openHtFromMapName(fileName);
		}
		else
		{
			htHandle = openHtFromFile(fileName);
			if (htHandle)
				runSnapshotTimer(htHandle);
		}

		return htHandle;
	}

	HtHandle* openHtFromFile(
		const wchar_t* fileName)
	{
		HANDLE hf = CreateFile(
			fileName,
			GENERIC_WRITE | GENERIC_READ,
			NULL,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (hf == INVALID_HANDLE_VALUE)
			return NULL;

		std::wstring FileMappingName = L"Global\\"; FileMappingName += fileName; FileMappingName += L"-filemapping";
		SECURITY_ATTRIBUTES SA = getSecurityAttributes();
		HANDLE hm = CreateFileMapping(
			hf,
			&SA,
			PAGE_READWRITE,
			0, 0,
			FileMappingName.c_str());
		if (!hm)
			return NULL;

		LPVOID lp = MapViewOfFile(
			hm,
			FILE_MAP_ALL_ACCESS,
			0, 0, 0);
		if (!lp)
			return NULL;

		HtHandle* htHandle = (HtHandle*)lp;
		htHandle->file = hf;
		htHandle->fileMapping = hm;
		htHandle->addr = lp;
		htHandle->lastSnaptime = time(NULL);
		std::wstring MutexName = L"Global\\"; MutexName += fileName; MutexName += L"-mutex";
		htHandle->mutex = CreateMutex(
			&SA,
			FALSE,
			MutexName.c_str());

		return htHandle;
	}

	HtHandle* openHtFromMapName(
		const wchar_t* fileName)
	{
		std::wstring FileMappingName = L"Global\\"; FileMappingName += fileName; FileMappingName += L"-filemapping";
		HANDLE hm = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,
			false,
			FileMappingName.c_str());
		if (!hm)
			return NULL;

		LPVOID lp = MapViewOfFile(
			hm,
			FILE_MAP_ALL_ACCESS,
			0, 0, 0);
		if (!lp)
			return NULL;

		HtHandle* htHandle = new HtHandle();
		memcpy(htHandle, lp, sizeof(HtHandle));
		htHandle->file = NULL;
		htHandle->fileMapping = hm;
		htHandle->addr = lp;
		htHandle->snapshotTimer = NULL;

		return htHandle;
	}

	bool canOpenHt(HtHandle* htHandle)
	{
		return isCurrentUserBelongTo(htHandle->htUsersGroup);
	}

	bool canOpenHt(HtHandle* htHandle, const wchar_t* htUser, const wchar_t* htPassword)
	{
		return isUserBelongToUsersGroup(htUser, htHandle->htUsersGroup) && verifyUser(htUser, htPassword);
	}

	BOOL runSnapshotTimer(HtHandle* htHandle)
	{
		htHandle->snapshotTimer = CreateWaitableTimer(0, FALSE, 0);
		LARGE_INTEGER Li{};
		Li.QuadPart = -(SECOND * htHandle->secSnapshotInterval);
		SetWaitableTimer(htHandle->snapshotTimer, &Li, htHandle->secSnapshotInterval * 1000, snapAsync, htHandle, FALSE);

		return true;
	}

	void CALLBACK snapAsync(LPVOID prm, DWORD, DWORD)
	{
		HtHandle* htHandle = (HtHandle*)prm;
		if (snap(htHandle))
			std::cout << "-- spanshotAsync success" << std::endl;
	}

	Element* get     //  читать элемент из хранилища
	(
		HtHandle* htHandle,            // управление HT
		const Element* element)              // элемент 
	{
		WaitForSingleObject(htHandle->mutex, INFINITE);
		int index = findIndex(htHandle, element);
		if (index < 0)
		{
			writeLastError(htHandle, "-- not found element (GET)");
			return NULL;
		}

		Element* foundElement = new Element();
		readFromMemory(htHandle, foundElement, index);
		ReleaseMutex(htHandle->mutex);

		return foundElement;
	}

	BOOL insert		// добавить элемент в хранилище
	(
		HtHandle* htHandle,            // управление HT
		const Element* element)              // элемент
	{
		if (htHandle->count >= htHandle->capacity)
		{
			writeLastError(htHandle, "-- not found free memory");
			return false;
		}

		WaitForSingleObject(htHandle->mutex, INFINITE);
		int freeIndex = findFreeIndex(htHandle, element);
		if (freeIndex < 0)
			return false;

		writeToMemory(htHandle, element, freeIndex);
		incrementCount(htHandle);
		ReleaseMutex(htHandle->mutex);

		return true;
	}

	BOOL update     //  именить элемент в хранилище
	(
		HtHandle* htHandle,            // управление HT
		const Element* oldElement,          // старый элемент (ключ, размер ключа)
		const void* newPayload,          // новые данные  
		int             newPayloadLength)     // размер новых данных
	{
		WaitForSingleObject(htHandle->mutex, INFINITE);
		int index = findIndex(htHandle, oldElement);
		if (index < 0)
		{
			writeLastError(htHandle, "-- not found element (UPDATE)");
			ReleaseMutex(htHandle->mutex);
			return false;
		}

		Element* updateElement = new Element(oldElement, newPayload, newPayloadLength);
		writeToMemory(htHandle, updateElement, index);
		ReleaseMutex(htHandle->mutex);

		return true;
	}

	BOOL removeOne      // удалить элемент в хранилище
	(
		HtHandle* htHandle,            // управление HT (ключ)
		const Element* element)				 // элемент 
	{
		WaitForSingleObject(htHandle->mutex, INFINITE);
		int index = findIndex(htHandle, element);
		if (index < 0)
		{
			writeLastError(htHandle, "-- not found element (DELETE)");
			ReleaseMutex(htHandle->mutex);
			return false;
		}

		clearMemoryByIndex(htHandle, index);
		decrementCount(htHandle);
		ReleaseMutex(htHandle->mutex);

		return true;
	}

	BOOL snap         // выполнить Snapshot
	(
		HtHandle* htHandle)           // управление HT (File, FileMapping)
	{
		WaitForSingleObject(htHandle->mutex, INFINITE);
		if (!FlushViewOfFile(htHandle->addr, NULL)) {
			writeLastError(htHandle, "-- snapshot error");
			ReleaseMutex(htHandle->mutex);
			return false;
		}
		htHandle->lastSnaptime = time(NULL);
		ReleaseMutex(htHandle->mutex);
		return true;
	}

	void print                               // распечатать элемент 
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
	}

	BOOL close        // snap и закрыть HT  и  очистить htHandle
	(
		const HtHandle* htHandle)           // управление HT (File, FileMapping)
	{
		HANDLE hf = htHandle->file;
		HANDLE hfm = htHandle->fileMapping;
		HANDLE mutex = htHandle->mutex;

		if (htHandle->snapshotTimer)
			CancelWaitableTimer(htHandle->snapshotTimer);
		UnmapViewOfFile(htHandle->addr);
		CloseHandle(hfm);
		if (hf)
			CloseHandle(hf);
		if (mutex)
			CloseHandle(mutex);

		return true;
	}

	int hashFunction(const char* key, int capacity)
	{
		unsigned long i = 0;
		for (int j = 0; key[j]; j++)
			i += key[j];
		return i % capacity;
	}

	int nextHash(int currentHash, const char* key, int capacity)
	{
		return ++currentHash;
	}

	int findFreeIndex(
		const HtHandle* htHandle,           // управление HT
		const Element* element)				// элемент
	{
		int index = hashFunction((char*)element->key, htHandle->capacity);

		Element* foundElement = new Element();
		do
		{
			if (index >= htHandle->capacity ||
				foundElement->key != NULL && memcmp(foundElement->key, element->key, element->keyLength) == NULL)
			{
				index = -1;
				break;
			}

			readFromMemory(htHandle, foundElement, index);
			index = nextHash(index, (char*)element->key, htHandle->capacity);
		} while (
			foundElement->keyLength != NULL &&
			foundElement->payloadLength != NULL);

		delete foundElement;
		return index - 1;
	}

	int findIndex(
		const HtHandle* htHandle,           // управление HT
		const Element* element)				// элемент
	{
		int index = hashFunction((char*)element->key, htHandle->capacity);

		Element* foundElement = new Element();
		do
		{
			if (index >= htHandle->capacity)
			{
				index = -1;
				break;
			}

			readFromMemory(htHandle, foundElement, index);
			index = nextHash(index, (char*)element->key, htHandle->capacity);
		} while (
			memcmp(foundElement->key, element->key, element->keyLength) != NULL);

		delete foundElement;
		return index - 1;
	}

	BOOL writeToMemory(const HtHandle* const htHandle, const Element* const element, int index)
	{
		LPVOID lp = htHandle->addr;

		lp = (HtHandle*)lp + 1;
		lp = (byte*)lp + getSizeElement(htHandle->maxKeyLength, htHandle->maxPayloadLength) * index;

		memcpy(lp, element->key, element->keyLength);
		lp = (byte*)lp + htHandle->maxKeyLength;
		memcpy(lp, &element->keyLength, sizeof(int));
		lp = (int*)lp + 1;
		memcpy(lp, element->payload, element->payloadLength);
		lp = (byte*)lp + htHandle->maxPayloadLength;
		memcpy(lp, &element->payloadLength, sizeof(int));

		return true;
	}

	int incrementCount(HtHandle* const htHandle)
	{
		return ++htHandle->count;
	}

	Element* readFromMemory(const HtHandle* const htHandle, Element* const element, int index)
	{
		LPVOID lp = htHandle->addr;

		lp = (HtHandle*)lp + 1;
		lp = (byte*)lp + getSizeElement(htHandle->maxKeyLength, htHandle->maxPayloadLength) * index;

		element->key = lp;
		lp = (byte*)lp + htHandle->maxKeyLength;
		element->keyLength = *(int*)lp;
		lp = (int*)lp + 1;
		element->payload = lp;
		lp = (byte*)lp + htHandle->maxPayloadLength;
		element->payloadLength = *(int*)lp;

		return element;
	}

	BOOL clearMemoryByIndex(const HtHandle* const htHandle, int index)
	{
		LPVOID lp = htHandle->addr;
		int sizeElement = getSizeElement(htHandle->maxKeyLength, htHandle->maxPayloadLength);

		lp = (HtHandle*)lp + 1;
		lp = (byte*)lp + sizeElement * index;

		ZeroMemory(lp, sizeElement);

		return true;
	}

	int decrementCount(HtHandle* const htHandle)
	{
		return --htHandle->count;
	}

	const char* writeLastError(HtHandle* const htHandle, const char* msg)
	{
		memcpy(htHandle->lastErrorMessage, msg, sizeof(htHandle->lastErrorMessage));
		return htHandle->lastErrorMessage;
	}

	const char* getLastError  // получить сообщение о последней ошибке
	(
		const HtHandle* htHandle)				// управление HT
	{
		return htHandle->lastErrorMessage;
	}

	SECURITY_ATTRIBUTES getSecurityAttributes()
	{
		const wchar_t* sdd = L"D:"
			L"(D;OICI;GA;;;BG)" //Deny guests
			L"(D;OICI;GA;;;AN)" //Deny anonymous
			L"(A;OICI;GA;;;AU)" //Allow read, write and execute for Users
			L"(A;OICI;GA;;;BA)"; //Allow all for Administrators
		SECURITY_ATTRIBUTES SA;
		ZeroMemory(&SA, sizeof(SA));
		SA.nLength = sizeof(SA);
		ConvertStringSecurityDescriptorToSecurityDescriptor(
			sdd,
			SDDL_REVISION_1,
			&SA.lpSecurityDescriptor,
			NULL);

		return SA;
	}


}