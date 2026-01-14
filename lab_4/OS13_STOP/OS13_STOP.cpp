#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
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

        // Открываем событие ActiveEvent
        std::wstring eventName = std::wstring(fileName) + L"_ActiveEvent";
        HANDLE hActiveEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName.c_str());

        if (!hActiveEvent)
            throw "Cannot open ActiveEvent";

        // Выполняем Snap через COM
        OS13HANDEL h1 = OS13::Init();
        HTHANDLE* ht;
        ht = OS13::Manipulator::Open(h1, fileName, true);

        if (ht)
        {
            if (!OS13::Manipulator::Snap(h1, ht))
                throw "Error while Snap in HT";

            std::cout << "Snapshot completed. Press any key to PAUSE all operations..." << std::endl;
            _getch();

            // ПАУЗИМ ВСЁ - и хранилище, и клиентов
            ResetEvent(hActiveEvent);
            std::cout << "*** PAUSED *** Press any key to RESUME..." << std::endl;

            _getch();

            // ВОЗОБНОВЛЯЕМ ВСЁ
            SetEvent(hActiveEvent);
            std::cout << "*** RESUMED *** Press any key to exit..." << std::endl;

            _getch();
        }
        else
            throw "Error while opening a storage";

        CloseHandle(hActiveEvent);
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
