#pragma once

#include <lm.h>
#pragma comment(lib, "netapi32.lib")

namespace ht
{
	bool isExistUsersGroup(const wchar_t* usersGroup);

	bool isCurrentUserBelongTo(const wchar_t* usersGroup);

	bool isUserBelongToUsersGroup(const wchar_t* user, const wchar_t* usersGroup);
	bool verifyUser(const wchar_t* user, const wchar_t* password);
}