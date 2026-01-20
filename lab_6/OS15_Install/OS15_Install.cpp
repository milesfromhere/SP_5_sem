#include <Windows.h>
#include <iostream>

#define SERVICENAME L"OS15_HTService"

#ifdef _WIN64
#define SERVICEPATH L"..\\Debug\\OS15_HTService.exe"
#else
#define SERVICEPATH L"..\\Debug\\OS15_HTService.exe"
#endif

char* errortxt(const char* msg, int code)
{
	char* buf = new char[512];

	sprintf_s(buf, 512, "%s: error code = %d\n", msg, code);

	return buf;
}

int main()
{
	SC_HANDLE schService = NULL, schSCManager = NULL;
	try
	{
		schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);//имя  компьютера, имя базы SCM, запрашиваемые права доступа

		if (!schSCManager)
		{
			throw errortxt("OpenSCManager", GetLastError());
		}
		else
		{
			schService = CreateService(
				schSCManager, //SCM
				SERVICENAME, //имя службы
				SERVICENAME, //отображаемое иям
				SERVICE_ALL_ACCESS, //права доступа
				SERVICE_WIN32_SHARE_PROCESS, //тип службы
				SERVICE_AUTO_START, //режим запуска
				SERVICE_ERROR_NORMAL, //уровень критичности ошибок
				SERVICEPATH, //путь к exe
				NULL,
				NULL,
				NULL,
				NULL,
				NULL
			);

			if (!schService)
			{
				throw errortxt("CreateService", GetLastError());
			}
		}
	}
	catch (char* txt)
	{
		std::cout << txt << std::endl;
	}


	if (schSCManager)
	{
		CloseServiceHandle(schSCManager);
	}

	if (schService)
	{
		CloseServiceHandle(schService);
	}

	return 0;
}