#include <Windows.h>
#include <iostream>
#include "framework.h"

int main()  //многопоточность 
{
    SetConsoleOutputCP(1251);

    HT::HTHANDLE* ht = nullptr;
    try
    {
        // first
        ht = HT::Open("C:\\Users\\nikit\\OneDrive\\Рабочий стол\\СП\\lab_1\\OS10\\HTspace.ht");
        if (ht) std::cout << "-- Open first stream: success\n";
        else throw "-- Open: error";

        // second
        if (HT::Insert(ht, new HT::Element("key333", 7, "payload12", 10))) std::cout << "-- Insert:success\n";
        else throw "-- insert:error";

        // first
        if (HT::Insert(ht, new HT::Element("key111", 7, "payload", 8))) std::cout << "-- Insert:success\n";
        else throw "-- insert:error";

        // second
        HT::Element* hte1 = HT::Get(ht, new HT::Element("key333", 7));
        if (hte1) std::cout << "Get: success\n";
        else throw "-- Get: error";

        // first
        HT::Element* hte2 = HT::Get(ht, new HT::Element("key111", 7));
        if (hte2) std::cout << "Get: success\n";
        else throw "-- Get: error";
        
        HT::print(hte1);

        HT::print(hte2);

        // first
        if (HT::Delete(ht, new HT::Element("key111", 7))) std::cout << "Delete: success\n";
        else throw "- Delete: error";

        // second
        if (HT::Delete(ht, new HT::Element("key333", 7))) std::cout << "Delete: success\n";
        else throw "- Delete: error";
        
        // first
        if (HT::Close(ht)) std::cout << "Close first stream: success\n";
        else throw "- Close: error";
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }
}