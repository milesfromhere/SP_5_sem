#include <Windows.h>
#include <iostream>
#include "../OS10_HTAPI/framework.h"

int main()
{
    SetConsoleOutputCP(1251);

    HT::HTHANDLE* ht = nullptr;

                        //Open && Create
    std::cout << "********** Create Test **********\n";
    //default
    try
    {
        ht = HT::Create(1000, 3, 10, 256, "C:\\Users\\nikit\\OneDrive\\Рабочий стол\\СП\\lab_1\\OS10\\HTspace.ht"); 
        HT::Close(ht);
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

    //with invalid parameters
    try 
    {
        ht = HT::Create(-12, 3, 12, 256, "C:\\Users\\nikit\\OneDrive\\Рабочий стол\\СП\\lab_1\\OS10\\HTspace.ht");
    }
    catch(char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

    std::cout << "\n\n\n********** Open Test **********\n";

    //default
    try
    {
        ht = HT::Open("C:\\Users\\nikit\\OneDrive\\Рабочий стол\\СП\\lab_1\\OS10\\HTspace.ht");
        if (ht) std::cout << "-- Open first stream: success\n";
        HT::Close(ht);
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

    //with invalid parameters
    try
    {
        ht = HT::Open("D:\\Лабы\\Лабы5сем\\СП\\Лабы\\OS10\\aboba.ht");
        if (ht) std::cout << "-- Open first stream: success\n";
        HT::Close(ht);
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }


                        //Insert
    std::cout << "\n\n\n********** Insert Test **********\n";
    //default
    try
    {
        ht = HT::Open("C:\\Users\\nikit\\OneDrive\\Рабочий стол\\СП\\lab_1\\OS10\\HTspace.ht");
        if (HT::Insert(ht, new HT::Element("key333", 7, "payload12", 10))) std::cout << "-- Insert:success\n";

    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

    //with same key
    try
    {
        if (HT::Insert(ht, new HT::Element("key333", 7, "payload12", 10))) std::cout << "-- Insert:success\n";
        else std::cout << "-- Insert: failed - " << HT::GetLastError(ht) << "\n";
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

    //with diff key long
    try
    {
        if (HT::Insert(ht, new HT::Element("key52222222", 7, "payload12", 10))) std::cout << "-- Insert:success\n";
        else std::cout << "-- Insert: failed - " << HT::GetLastError(ht) << "\n";
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

                                //Update
    std::cout << "\n\n\n********** Update Test **********\n";

    //default
    try
    {
        HT::Element* htel = new HT::Element("key812", 7, "payload", 10);
        if (HT::Update(ht, htel,"newpayload52",12)) std::cout << "-- Update:success\n";
        else std::cout << "-- Update: failed - " << HT::GetLastError(ht) << "\n";
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

    //doesn't exist
    try
    {
        HT::Element* htel = new HT::Element();
        if (HT::Update(ht, htel, "newpayload52", 12)) std::cout << "-- Update:success\n";
        else std::cout << "-- Update: failed - " << HT::GetLastError(ht) << "\n";
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

    //with diff key long
    try
    {
        HT::Element* htel = new HT::Element("key812", 7, "payload", 10);
        if (HT::Update(ht, htel, "newpayload52", 11)) std::cout << "-- Update:success\n";
        else std::cout << "-- Update: failed - " << HT::GetLastError(ht) << "\n";
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

                                //Delete
    std::cout << "\n\n\n********** Delete Test **********\n";

    //default
    try
    {
        HT::Element* htel = HT::Get(ht, new HT::Element("key333", 7));
        if (HT::Delete(ht, htel)) std::cout << "-- Delete:success\n";
        else std::cout << "-- Delete: failed - " << HT::GetLastError(ht) << "\n";
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

    //doesn't exist
    try
    {
        HT::Element* htel = new HT::Element("key000",6);
        if (HT::Delete(ht, htel)) std::cout << "-- Delete:success\n";
        else std::cout << "-- Delete: failed - " << HT::GetLastError(ht) << "\n";
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

    //with null key 
    try
    {
        HT::Element* htel = new HT::Element(NULL, 6);
        if (HT::Delete(ht, htel)) std::cout << "-- Delete:success\n";
        else std::cout << "-- Delete: failed - " << HT::GetLastError(ht) << "\n";
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }


                                    //Get
    std::cout << "\n\n\n********** Get Test **********\n";

    //default
    try
    {
        HT::Insert(ht, new HT::Element("key333", 7, "payload12", 10));
        HT::Element* htel = HT::Get(ht, new HT::Element("key333", 7));
        if (htel) std::cout << "-- Get:success\n";
        else std::cout << "-- Get: failed - " << HT::GetLastError(ht) << "\n";
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

    //doesn't exist
    try
    {
        HT::Element * htel = HT::Get(ht, new HT::Element("key000", 6)); 
        if (htel) std::cout << "-- Get:success\n";
        else std::cout << "-- Get: failed - " << HT::GetLastError(ht) << "\n";
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

    //with null key 
    try
    {
        HT::Element* htel = HT::Get(ht, new HT::Element(NULL, 6));
         if (htel) std::cout << "-- Get:success\n";
        else std::cout << "-- Get: failed - " << HT::GetLastError(ht) << "\n";
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

                                    //Print
    std::cout << "\n\n\n********** Print Test **********\n";

    //default
    try
    {
        HT::Element* htel = HT::Get(ht, new HT::Element("key333", 7));
        HT::print(htel);
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }

    //doesn't exist
    try
    {
        HT::print(NULL);
    }
    catch (char* msg)
    {
        std::cout << msg << "\n";
        if (ht != nullptr) std::cout << HT::GetLastError(ht);
    }
}    
