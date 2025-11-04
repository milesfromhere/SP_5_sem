#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include "HT.h"


// ранняя версия реализации
namespace HT {


    HTHANDLE::HTHANDLE()
        : Capacity(0),                    
        SecSnapshotInterval(0),         
        MaxKeyLength(0),                
        MaxPayloadLength(0),            
        File(INVALID_HANDLE_VALUE),     
        FileMapping(NULL),              
        Addr(nullptr),                  
        lastsnaptime(0)                 
    {
        FileName[0] = '\0';
        LastErrorMessage[0] = '\0';
    }


    HTHANDLE::HTHANDLE(int Capacity, int SecSnapshotInterval, int MaxKeyLength, int MaxPayloadLength, const char FileName[512])
        : Capacity(Capacity),
        SecSnapshotInterval(SecSnapshotInterval),
        MaxKeyLength(MaxKeyLength),
        MaxPayloadLength(MaxPayloadLength),
        File(INVALID_HANDLE_VALUE),
        FileMapping(NULL),
        Addr(nullptr),
        lastsnaptime(0)
    {
        strcpy_s(this->FileName, sizeof(this->FileName), FileName);
        LastErrorMessage[0] = '\0';
    }

    Element::Element()
        : key(nullptr),
        keylength(0),
        payload(nullptr),
        payloadlength(0)
    {}

    Element::Element(const void* key, int keylength)
        : key(key),
        keylength(keylength),
        payload(nullptr),
        payloadlength(0)
    {}

    Element::Element(const void* key, int keylength, const void* payload, int  payloadlength)
        : key(key),
        keylength(keylength),
        payload(payload),
        payloadlength(payloadlength)
    {}

    Element::Element(Element* oldelement, const void* newpayload, int  newpayloadlength)
        : key(oldelement->key),
        keylength(oldelement->keylength),
        payload(newpayload),
        payloadlength(newpayloadlength)
    {}

    HTHANDLE* Create(
        int Capacity,
        int SecSnapshotInterval,
        int MaxKeyLength,
        int MaxPayloadLength,
        const char FileName[512]
    )
    {
        if (Capacity <= 0 || SecSnapshotInterval <= 0 || MaxKeyLength <= 0 || MaxPayloadLength <= 0) {
            std::cout << "Invalid parameters\n";
            return nullptr;
        }

        HTHANDLE* ht = new HTHANDLE(Capacity, SecSnapshotInterval, MaxKeyLength, MaxPayloadLength, FileName);
        if (!ht) {
            std::cout << "Memory allocation failed\n";
            return nullptr;
        }

        ht->File = CreateFileA(
            FileName,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (ht->File == INVALID_HANDLE_VALUE) {
            strcpy_s(ht->LastErrorMessage, "CreateFile failed");
            delete ht;
            return nullptr;
        }

        DWORD fileSize = sizeof(HTHANDLE) + Capacity * (MaxKeyLength + MaxPayloadLength + 2 * sizeof(int));

        SetFilePointer(ht->File, fileSize, NULL, FILE_BEGIN);
        SetEndOfFile(ht->File);

        ht->FileMapping = CreateFileMapping(
            ht->File,
            NULL,
            PAGE_READWRITE,
            0, fileSize,
            NULL
        );

        if (!ht->FileMapping) {
            strcpy_s(ht->LastErrorMessage, "CreateFileMapping failed");
            CloseHandle(ht->File);
            delete ht;
            return nullptr;
        }

        ht->Addr = MapViewOfFile(
            ht->FileMapping,
            FILE_MAP_ALL_ACCESS,
            0, 0, fileSize
        );

        if (!ht->Addr) {
            strcpy_s(ht->LastErrorMessage, "MapViewOfFile failed");
            CloseHandle(ht->FileMapping);
            CloseHandle(ht->File);
            delete ht;
            return nullptr;
        }

        memcpy(ht->Addr, ht, sizeof(HTHANDLE));

        char* dataStart = (char*)ht->Addr + sizeof(HTHANDLE);
        memset(dataStart, 0, fileSize - sizeof(HTHANDLE));

        ht->lastsnaptime = time(nullptr);

        std::cout << "HT created successfully: " << FileName << "\n";
        std::cout << "Capacity: " << Capacity << "\n";
        std::cout << "Snapshot interval: " << SecSnapshotInterval << " sec\n";
        std::cout << "Max key length: " << MaxKeyLength << "\n";
        std::cout << "Max payload length: " << MaxPayloadLength << "\n";

        return ht;
    }

    HTHANDLE* Open(const char FileName[512]) {
        
        HANDLE hFile = CreateFileA(
            FileName,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,  
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            std::cout << "Cannot open HT file: " << FileName << "\n";
            return nullptr;
        }

        HANDLE hMapping = CreateFileMappingA(
            hFile,
            NULL,
            PAGE_READWRITE,
            0, 0,  
            NULL
        );

        if (!hMapping) {
            CloseHandle(hFile);
            return nullptr;
        }

        LPVOID pData = MapViewOfFile(
            hMapping,
            FILE_MAP_ALL_ACCESS,
            0, 0, 0
        );

        if (!pData) {
            CloseHandle(hMapping);
            CloseHandle(hFile);
            return nullptr;
        }

        HTHANDLE* fileHeader = (HTHANDLE*)pData;

        HTHANDLE* ht = new HTHANDLE(
            fileHeader->Capacity,
            fileHeader->SecSnapshotInterval,
            fileHeader->MaxKeyLength,
            fileHeader->MaxPayloadLength,
            fileHeader->FileName
        );

        ht->File = hFile;
        ht->FileMapping = hMapping;
        ht->Addr = pData;

        std::cout << "HT opened successfully: " << FileName << "\n";
        std::cout << "Capacity: " << ht->Capacity << " elements\n";

        return ht;
    }

    BOOL Snap(const HTHANDLE* hthandle) {

        if (!hthandle || !hthandle->Addr) {
            return FALSE;  
        }

        DWORD error = ::GetLastError();

        BOOL success = FlushViewOfFile(hthandle->Addr, 0);

        if (success) {

            HTHANDLE* nonConstHandle = (HTHANDLE*)hthandle;
            nonConstHandle->lastsnaptime = time(nullptr);

            std::cout << "Snapshot performed at: " << nonConstHandle->lastsnaptime << "\n";
        }
        else {

            HTHANDLE* nonConstHandle = (HTHANDLE*)hthandle;
            
            std::cout << "Snapshot failed! Error: " << error << "\n";
            strcpy_s(nonConstHandle->LastErrorMessage, "FlushViewOfFile failed");
        }

        return success;
    }

    BOOL Close(const HTHANDLE* hthandle)
    {
        if (!hthandle || !hthandle->Addr) {
            return FALSE;
        }

        HTHANDLE* ht = (HTHANDLE*)hthandle;

        BOOL success = TRUE;

        std::cout << "Performing final snapshot...\n";
        if (!Snap(hthandle)) {
            std::cout << "Warning: Snapshot failed during close!\n";
            success = FALSE;
        }
        else {
            std::cout << "Snapshot completed successfully\n";
        }

        if (ht->Addr) {
            if (!FlushViewOfFile(ht->Addr, 0)) {
                std::cout << "Warning: FlushViewOfFile failed\n";
            }
            if (!UnmapViewOfFile(ht->Addr)) {
                std::cout << "Warning: UnmapViewOfFile failed\n";
                success = FALSE;
            }
            else {
                std::cout << "Memory unmapped successfully\n";
            }
            ht->Addr = nullptr;
        }

        if (ht->FileMapping) {
            if (!CloseHandle(ht->FileMapping)) {
                std::cout << "Warning: CloseHandle for FileMapping failed\n";
                success = FALSE;
            }
            else {
                std::cout << "File mapping closed\n";
            }
            ht->FileMapping = NULL;
        }

        if (ht->File != INVALID_HANDLE_VALUE) {
            if (!CloseHandle(ht->File)) {
                std::cout << "Warning: CloseHandle for File failed\n";
                success = FALSE;
            }
            else {
                std::cout << "File closed\n";
            }
            ht->File = INVALID_HANDLE_VALUE;
        }

        std::cout << "Freeing HT handle...\n";
        delete ht;

        std::cout << "HT storage closed " << (success ? "successfully" : "with warnings") << "\n";
        return success;

    }

    BOOL Insert(const HTHANDLE* hthandle, const Element* element)
    {
        if (!hthandle || !element)
        {
            return FALSE;
        }

        HTHANDLE* ht = (HTHANDLE*)hthandle;

        if (!element->key || element->keylength <= 0) {
            strcpy_s(ht->LastErrorMessage, "Invalid key");
            return FALSE;
        }

        if (element->keylength > ht->MaxKeyLength) {
            strcpy_s(ht->LastErrorMessage, "Key too long");
            return FALSE;
        }

        if (element->payload && element->payloadlength > ht->MaxPayloadLength) {
            strcpy_s(ht->LastErrorMessage, "Payload too long");
            return FALSE;
        }

        char* memory = (char*)ht->Addr;
        Element* storage = (Element*)(memory + sizeof(HTHANDLE));

        for (int i = 0; i < ht->Capacity; i++) {

            if (storage[i].keylength == 0) {

                storage[i].key = malloc(element->keylength);
                if (!storage[i].key) {
                    strcpy_s(ht->LastErrorMessage, "Memory allocation failed");
                    return FALSE;
                }
                memcpy((void*)storage[i].key, element->key, element->keylength);
                storage[i].keylength = element->keylength;


                if (element->payload && element->payloadlength > 0) {
                    storage[i].payload = malloc(element->payloadlength);
                    if (!storage[i].payload) {
                        free((void*)storage[i].key);
                        storage[i].key = nullptr;
                        storage[i].keylength = 0;
                        strcpy_s(ht->LastErrorMessage, "Memory allocation failed");
                        return FALSE;
                    }
                    memcpy((void*)storage[i].payload, element->payload, element->payloadlength);
                    storage[i].payloadlength = element->payloadlength;
                }
                else {
                    storage[i].payload = nullptr;
                    storage[i].payloadlength = 0;
                }

                std::cout << "Element inserted at position " << i << "\n";
                return TRUE;
            }

        }

        strcpy_s(ht->LastErrorMessage, "Storage is full");
        return FALSE;
    }

    BOOL Update(const HTHANDLE* hthandle,
        const Element* oldelement,
        const void* newpayload,
        int newpayloadlength)
    {
        if (!hthandle || !oldelement) {
            return FALSE;
        }

        HTHANDLE* ht = (HTHANDLE*)hthandle;

        if (!oldelement->key || oldelement->keylength <= 0) {
            strcpy_s(ht->LastErrorMessage, "Invalid key for update");
            return FALSE;
        }

        if (newpayloadlength > ht->MaxPayloadLength) {
            strcpy_s(ht->LastErrorMessage, "New payload too long");
            return FALSE;
        }

        if (!ht->Addr) {
            strcpy_s(ht->LastErrorMessage, "Storage not initialized");
            return FALSE;
        }

        char* memory = (char*)ht->Addr;
        Element* storage = (Element*)(memory + sizeof(HTHANDLE));


        for (int i = 0; i < ht->Capacity; i++) {
            if (storage[i].keylength == 0) {
                continue;
            }

            if (storage[i].keylength == oldelement->keylength &&
                memcmp(storage[i].key, oldelement->key, oldelement->keylength) == 0) {

                if (storage[i].payload) {
                    free((void*)storage[i].payload);
                    storage[i].payload = nullptr;
                    storage[i].payloadlength = 0;
                }

                if (newpayload && newpayloadlength > 0) {
                    storage[i].payload = malloc(newpayloadlength);
                    if (!storage[i].payload) {
                        strcpy_s(ht->LastErrorMessage, "Memory allocation failed for new payload");
                        storage[i].payloadlength = 0;
                        return FALSE;
                    }

                    memcpy((void*)storage[i].payload, newpayload, newpayloadlength);
                    storage[i].payloadlength = newpayloadlength;
                }
                else {
                    storage[i].payload = nullptr;
                    storage[i].payloadlength = 0;
                }

                std::cout << "Element updated at position " << i << "\n";
                return TRUE;
            }
        }
    }

    BOOL Delete(const HTHANDLE* hthandle, const Element* element)
    {
        if (!hthandle || !element)
        {
            return FALSE;
        }

        HTHANDLE* ht = (HTHANDLE*)hthandle;

        if (!element->key || element->keylength <= 0) {
            strcpy_s(ht->LastErrorMessage, "Invalid key");
            return FALSE;
        }

        char* memory = (char*)ht->Addr;
        Element* storage = (Element*)(memory + sizeof(HTHANDLE));

        for (int i = 0; i < ht->Capacity; i++)
        {
            if (storage[i].keylength == 0)
            {
                continue;
            }

            if (storage[i].keylength == element->keylength &&
                memcmp(storage[i].key, element->key, element->keylength) == 0)
            {
                if (storage[i].key) {
                    free((void*)storage[i].key);
                }
                if (storage[i].payload) {
                    free((void*)storage[i].payload);
                }

                storage[i].key = nullptr;
                storage[i].keylength = 0;
                storage[i].payload = nullptr;
                storage[i].payloadlength = 0;

                std::cout << "Element deleted from position " << i << "\n";
                return TRUE;
            }
        }

        strcpy_s(ht->LastErrorMessage, "Element not found for deletion");
        return FALSE;
    }

    Element* Get(const HTHANDLE* hthandle, const Element* element)
    {
        if (!hthandle || !element) {
            return nullptr;
        }

        HTHANDLE* ht = (HTHANDLE*)hthandle;

        if (!element->key || element->keylength <= 0) {
            strcpy_s(ht->LastErrorMessage, "Invalid key for search");
            return nullptr;
        }

        if (!ht->Addr) {
            strcpy_s(ht->LastErrorMessage, "Storage not initialized");
            return nullptr;
        }

        char* memory = (char*)ht->Addr;
        Element* storage = (Element*)(memory + sizeof(HTHANDLE));

        for (int i = 0; i < ht->Capacity; i++) {
            if (storage[i].keylength == 0) {
                continue;
            }

            if (storage[i].keylength == element->keylength &&
                memcmp(storage[i].key, element->key, element->keylength) == 0) {

                Element* result = new Element();

                result->key = malloc(storage[i].keylength);
                if (!result->key) {
                    strcpy_s(ht->LastErrorMessage, "Memory allocation failed");
                    delete result;
                    return nullptr;
                }
                memcpy((void*)result->key, storage[i].key, storage[i].keylength);
                result->keylength = storage[i].keylength;

                if (storage[i].payload && storage[i].payloadlength > 0) {
                    result->payload = malloc(storage[i].payloadlength);
                    if (!result->payload) {
                        free((void*)result->key);
                        delete result;
                        strcpy_s(ht->LastErrorMessage, "Memory allocation failed");
                        return nullptr;
                    }
                    memcpy((void*)result->payload, storage[i].payload, storage[i].payloadlength);
                    result->payloadlength = storage[i].payloadlength;
                }
                else {
                    result->payload = nullptr;
                    result->payloadlength = 0;
                }

                std::cout << "Element found at position " << i << "\n";
                return result;
            }
        }

        strcpy_s(ht->LastErrorMessage, "Element not found");
        return nullptr;
    }

    void print(const Element* element)
    {
        if (!element)
        {
            std::cout << "Element doesn't exist or null\n";
        }

        std::cout << "=== Element Info ===\n";

        std::cout << "Key: ";
        if (element->key && element->keylength > 0) {
            std::cout << "\"";
            for (int i = 0; i < element->keylength; i++) {
                char c = static_cast<const char*>(element->key)[i];
                if (c >= 32 && c <= 126) {
                    std::cout << c;
                }
            }
            std::cout << "\"";
        }
        else {
            std::cout << "[empty]";
        }
        std::cout << " (length: " << element->keylength << " bytes)\n";

        
        std::cout << "Payload: ";
        if (element->payload && element->payloadlength > 0) {
            std::cout << "\"";
            for (int i = 0; i < element->payloadlength; i++) {
                char c = static_cast<const char*>(element->payload)[i];
                if (c >= 32 && c <= 126) { 
                    std::cout << c;
                }
            }
            std::cout << "\"";
        }
        else {
            std::cout << "[empty]";
        }
        std::cout << " (length: " << element->payloadlength << " bytes)\n";

        std::cout << "====================\n";

    }

    char* GetLastError(HTHANDLE* ht)
    {
        if (!ht) {
            static char nullError[] = "Error: Null handle provided\n";
            return nullError;
        }

        if (ht->LastErrorMessage[0] == '\0') {
            static char noError[] = "No error";
            return noError;
        }

        static char fullError[512];
        if (!ht->Addr) {
            sprintf_s(fullError, sizeof(fullError), "%s (Storage not mapped)\n", ht->LastErrorMessage);
            return fullError;
        }

        if (ht->File == INVALID_HANDLE_VALUE) {
            sprintf_s(fullError, sizeof(fullError), "%s (File not open)\n", ht->LastErrorMessage);
            return fullError;
        }

        return ht->LastErrorMessage;
    }
}   