#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "NtlGuild.h"

class CGuildRepository : public CNtlSingleton<CGuildRepository>
{

public:

	CGuildRepository() {}
	virtual ~CGuildRepository() {}

public:

	smart_ptr<QueryResult>			LoadGuilds();
	smart_ptr<QueryResult>			LoadGuildMembers(GUILDID guildId, DWORD dwLimit);
	smart_ptr<QueryResult>			GetZeniByGuildId(GUILDID guildId);

	void							InsertGuild(GUILDID guildId, WCHAR* wszName, CHARACTERID guildMaster, QWORD qwGuildFunctionFlag);

	void							SetCharacterGuild(CHARACTERID charId, GUILDID guildId, WCHAR* wszGuildName);
	void							InsertGuildMember(GUILDID guildId, CHARACTERID charId);

	void							RemoveMemberGuildFlag(CHARACTERID charId);
	void							DeleteGuildMember(GUILDID guildId, CHARACTERID charId);

	void							RemoveAllMembersGuildFlag(GUILDID guildId);
	void							DeleteGuildMembers(GUILDID guildId);
	void							DeleteGuild(GUILDID guildId);

	void							UpdateSecondMaster(const char* columnName, CHARACTERID value, GUILDID guildId);
	void							UpdateGuildMaster(CHARACTERID masterCharId, GUILDID guildId);
	void							UpdateReputationAndFunctionFlag(DWORD reputation, QWORD functionFlag, GUILDID guildId);
	void							UpdateReputationAndPointEver(DWORD reputation, DWORD pointEver, GUILDID guildId);
	void							UpdateNotice(WCHAR* wszNoticeBy, char* rawMessage, GUILDID guildId);
	void							UpdateMark(sDBO_GUILD_MARK& mark, GUILDID guildId);
	void							InsertNameChangeLog(GUILDID guildId, WCHAR* wszCurrentName, WCHAR* wszNewName);
	bool							UpdateGuildNameWait(WCHAR* wszNewName, GUILDID guildId);
	void							UpdateZeni(DWORD zeni, GUILDID guildId);

};

#define GetGuildRepository()			CGuildRepository::GetInstance()
#define g_pGuildRepository				GetGuildRepository()
