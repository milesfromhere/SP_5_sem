#include <Windows.h>
#include <iostream>
#include "framework.h"

int main()  //базовый сценарий
{
    SetConsoleOutputCP(1251);

    HT::HTHANDLE* ht = nullptr;
    try
    {
        ht = HT::Create(1000, 3, 10, 256, "C:\\Users\\nikit\\OneDrive\\Рабочий стол\\СП\\lab_1\\OS10\\HTspace.ht"); 
        if (ht) std::cout << "-- Create: success\n";
        else throw "-- Create: error";

        if (HT::Insert(ht, new HT::Element("key222", 7, "payload", 8))) std::cout << "-- Insert:success\n";
        else throw "-- insert:error";

        HT::Element* hte = HT::Get(ht, new HT::Element("key222", 7));
        if (hte) std::cout << "Get: success\n";
        else throw "-- Get: error";

        HT::print(hte);

        if (HT::Snap(ht)) std::cout << "Snap: success\n";
        else throw "-- Snap: error";

        if (HT::Update(ht, hte, "newpayload", 11)) std::cout << "-- Update:success\n";
        else throw "-- Update:error";

        HT::Element* htel = HT::Get(ht, new HT::Element("key222", 7));
        if (htel) std::cout << "Get: success\n";
        else throw "-- Get: error";

        HT::print(htel);

        if (HT::Delete(ht, htel)) std::cout << "Delete: success\n";
        else throw "- Delete: error";

        if (HT::Close(ht)) std::cout << "Close: success\n";
        else throw "- Close: error";
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }
}