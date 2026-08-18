#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "NtlDojo.h"

class CDojoRepository : public CNtlSingleton<CDojoRepository>
{

public:

	CDojoRepository() {}
	virtual ~CDojoRepository() {}

public:

	smart_ptr<QueryResult>			LoadDojos();

	void							DeleteByGuildId(GUILDID guildId);
	void							InsertDojo(GUILDID guildId, TBLIDX dojoTblidx, WCHAR* wszGuildName);
	void							ResetDojo(GUILDID guildId, WCHAR* wszGuildName, GUILDID challengeGuildId, TBLIDX dojoTblidx);
	void							DeleteByTblidx(TBLIDX dojoTblidx);
	void							UpdateLevel(BYTE byLevel, GUILDID guildId);
	void							UpdateSeedCharName(WCHAR* wszCharName, TBLIDX dojoTblidx);
	void							ClearSeedCharName(TBLIDX dojoTblidx);
	void							UpdateGuildName(GUILDID guildId, WCHAR* wszGuildName);
	void							UpdateNotice(WCHAR* wszLeaderName, char* rawMessage, GUILDID guildId);

};

#define GetDojoRepository()			CDojoRepository::GetInstance()
#define g_pDojoRepository				GetDojoRepository()
