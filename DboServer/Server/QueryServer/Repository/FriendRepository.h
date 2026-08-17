#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"

class CFriendRepository : public CNtlSingleton<CFriendRepository>
{

public:

	CFriendRepository() {}
	virtual ~CFriendRepository() {}

public:

	void						DeleteFriendList(CHARACTERID charId);
	void						UpdateFriendName(WCHAR* wszName, CHARACTERID friendId);

};

#define GetFriendRepository()		CFriendRepository::GetInstance()
#define g_pFriendRepository		GetFriendRepository()
