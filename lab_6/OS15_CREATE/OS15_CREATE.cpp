#pragma warning(disable : 4996)

#include <iostream>
#include <windows.h>

#include "../OS15_HTCOM_LIB/pch.h"
#include "../OS15_HTCOM_LIB/OS15_HTCOM_LIB.h"

#ifdef _WIN64
#pragma comment(lib, "../x64/Debug/OS15_HTCOM_LIB.lib")
#else
#pragma comment(lib, "../Debug/OS15_HTCOM_LIB.lib")
#endif

using namespace std;

wchar_t* getWC(const char* c);

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");

	try
	{
		cout << "Инициализация компонента:" << endl;
		OS15_HTCOM_HANDEL h = OS15_HTCOM::Init();

		ht::HtHandle* ht = OS15_HTCOM::HT::create(h, atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), getWC(argv[5]), getWC(argv[6]));

		if (ht)
		{
			cout << "HT-Storage Created" << endl;
			wcout << "filename: " << ht->fileName << endl;
			cout << "secSnapshotInterval: " << ht->secSnapshotInterval << endl;
			cout << "capacity: " << ht->capacity << endl;
			cout << "maxKeyLength: " << ht->maxKeyLength << endl;
			cout << "maxPayloadLength: " << ht->maxPayloadLength << endl;

			OS15_HTCOM::HT::close(h, ht);
		}
		else
			cout << "-- create: error" << endl;

		cout << endl << "Удалить компонент и выгрузить dll, если можно:" << endl;
		OS15_HTCOM::Dispose(h);
	}
	catch (const char* e) { cout << e << endl; }
	catch (int e) { cout << "HRESULT: " << e << endl; }

}

wchar_t* getWC(const char* c)
{
	wchar_t* wc = new wchar_t[strlen(c) + 1];
	mbstowcs(wc, c, strlen(c) + 1);

	return wc;
}