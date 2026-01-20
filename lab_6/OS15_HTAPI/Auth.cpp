#include "pch.h"

#include "Auth.h"

namespace ht
{
	bool isExistUsersGroup(const wchar_t* usersGroup)
	{
		bool rc = false;

#pragma region Получить_коллекцию_групп

		NET_API_STATUS netStatus;
		PLOCALGROUP_INFO_0 pBuffer;
		DWORD entriesread, totalentries;

		netStatus = NetLocalGroupEnum(NULL, 0, (LPBYTE*)&pBuffer, MAX_PREFERRED_LENGTH, &entriesread, &totalentries, NULL);

		//for (int i = 0; i < entriesread; i++)
		//	std::wcout << L"Group: " << pBuffer[i].lgrpi0_name << std::endl;

#pragma endregion

		for (int i = 0; i < entriesread; i++)
			if (wcscmp(pBuffer[i].lgrpi0_name, usersGroup) == 0)
			{
				rc = true;
				break;
			}

		NetApiBufferFree(pBuffer);

		return rc;
	}

	bool isCurrentUserBelongTo(const wchar_t* usersGroup)
	{
		bool rc = false;

#pragma region Получить_имя_текущего_пользователя

		DWORD lenUn = 512;
		wchar_t un[512];

		if (!GetUserName(un, &lenUn))
			return rc;

		//std::wcout << L"GetUserName: " << un << std::endl;

#pragma endregion

		if (isUserBelongToUsersGroup(un, usersGroup))
			rc = true;

		return rc;
	}

	bool isUserBelongToUsersGroup(const wchar_t* user, const wchar_t* usersGroup)
	{
		bool rc = false;

#pragma region Получить_коллекцию_групп_пользователя

		GROUP_USERS_INFO_0* buf3;
		DWORD uc3 = 0, tc3 = 0;

		NET_API_STATUS ns3 = NetUserGetLocalGroups(NULL, user, 0, LG_INCLUDE_INDIRECT, (LPBYTE*)&buf3, MAX_PREFERRED_LENGTH, &uc3, &tc3);

		//if (ns3 == NERR_Success)
		//	for (int i = 0; i < uc3; i++)
		//		std::wcout << buf3[i].grui0_name << std::endl;

#pragma endregion

		if (ns3 == NERR_Success)
			for (int i = 0; i < uc3; i++)
				if (wcscmp(buf3[i].grui0_name, usersGroup) == 0)
				{
					rc = true;
					break;
				}

		NetApiBufferFree(buf3);

		return rc;
	}

	bool verifyUser(const wchar_t* user, const wchar_t* password)
	{
		bool rc = false;

#pragma region Верификация

		HANDLE pHandle;
		if (LogonUserW(user, NULL, password, LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &pHandle) != 0)
			rc = true;

#pragma endregion

		return rc;
	}
}