#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <wchar.h>
#include <string>
#include <conio.h>
#pragma comment(lib, "../x64/Debug/OS13_HTCOM_LIB.lib")

#include "../OS13_HTCOM_LIB/HT.h"
#include "../OS13_HTCOM_LIB/OS13.h"


int main(int argc, char* argv[])
{
    wchar_t* fileName;
    try
    {
        if (argv[1])
        {
            const size_t cSize = strlen(argv[1]) + 1;
            wchar_t* wc = new wchar_t[cSize];
            mbstowcs(wc, argv[1], cSize);
            fileName = wc;
        }
        else
        {
            throw "Invalid file name";
        }

        OS13HANDEL h1 = OS13::Init();
        HTHANDLE* ht;
        ht = OS13::Manipulator::Open(h1, fileName);

        if (ht)
        {
            std::cout << "HT-Storage Created " << std::endl;
            std::wcout << "Filename: " << fileName << std::endl;
            std::cout << "SnapshotIntervalSec: " << ht->SecSnapshotInterval << std::endl;
            std::cout << "Capacity: " << ht->Capacity << std::endl;
            std::cout << "MaxKeyLength: " << ht->MaxKeyLength << std::endl;
            std::cout << "MaxPayloadLength: " << ht->MaxPayloadLength << std::endl;
            std::cout << "\nStorage is running. Press any key to close..." << std::endl;

            // Ждём нажатия любой клавиши с поддержкой APC (для снапшотов)
            while (!_kbhit())
            {
                SleepEx(100, TRUE);
            }

            _getch();

            std::cout << "\nClosing storage..." << std::endl;

            // СНАЧАЛА блокируем все операции через ActiveEvent
            std::wstring eventName = std::wstring(fileName) + L"_ActiveEvent";
            HANDLE hActiveEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName.c_str());

            if (hActiveEvent) {
                ResetEvent(hActiveEvent);  // Блокируем новые операции
                std::cout << "Active operations blocked." << std::endl;
                CloseHandle(hActiveEvent);
            }

            // Даём время клиентам обнаружить закрытие
            Sleep(1000);

            // Теперь закрываем хранилище
            OS13::Manipulator::Close(h1, ht);

            std::cout << "Storage closed successfully." << std::endl;
        }
        else
            throw "Error while opening a storage";

        OS13::Dispose(h1);
    }
    catch (const char* err)
    {
        std::cout << err << std::endl;
        return -1;
    }
    catch (int err)
    {
        std::cout << "Error code: " << err << std::endl;
        return -1;
    }
    catch (const std::exception&)
    {
        std::cout << "An error has occurred. Check settings and try again" << std::endl;
        return -1;
    }

    return 0;
}