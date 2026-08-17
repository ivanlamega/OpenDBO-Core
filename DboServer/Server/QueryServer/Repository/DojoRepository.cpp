#include "stdafx.h"
#include "DojoRepository.h"
#include "../QueryServer.h"


smart_ptr<QueryResult> CDojoRepository::LoadDojos()
{
	return GetCharDB.Query("SELECT * FROM dojos LIMIT 7");
}

void CDojoRepository::DeleteByGuildId(GUILDID guildId)
{
	GetCharDB.Execute("DELETE FROM dojos WHERE GuildId=%u", guildId);
}

void CDojoRepository::InsertDojo(GUILDID guildId, TBLIDX dojoTblidx, WCHAR* wszGuildName)
{
	GetCharDB.Execute("INSERT INTO dojos (GuildId, DojoTblidx, GuildName) VALUES (%u,%u,\"%ls\")", guildId, dojoTblidx, wszGuildName);
}

void CDojoRepository::ResetDojo(GUILDID guildId, WCHAR* wszGuildName, GUILDID challengeGuildId, TBLIDX dojoTblidx)
{
	GetCharDB.Execute("UPDATE dojos SET GuildId=%u, Level=0, PeaceStatus=0, PeacePoints=0, GuildName=\"%ls\", LeaderName=(null), Notice=(null), ChallengeGuildId=%u, SeedCharName=(null) WHERE DojoTblidx=%u",
		guildId, wszGuildName, challengeGuildId, dojoTblidx);
}

void CDojoRepository::DeleteByTblidx(TBLIDX dojoTblidx)
{
	GetCharDB.Execute("DELETE FROM dojos WHERE DojoTblidx=%u", dojoTblidx);
}

void CDojoRepository::UpdateLevel(BYTE byLevel, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE dojos SET Level=%u WHERE GuildID=%u", byLevel, guildId);
}

void CDojoRepository::UpdateSeedCharName(WCHAR* wszCharName, TBLIDX dojoTblidx)
{
	GetCharDB.Execute("UPDATE dojos SET SeedCharName=\"%ls\" WHERE DojoTblidx=%u", wszCharName, dojoTblidx);
}

void CDojoRepository::ClearSeedCharName(TBLIDX dojoTblidx)
{
	GetCharDB.Execute("UPDATE dojos SET SeedCharName=(Null) WHERE DojoTblidx=%u", dojoTblidx);
}

void CDojoRepository::UpdateGuildName(GUILDID guildId, WCHAR* wszGuildName)
{
	GetCharDB.Execute("UPDATE dojos SET GuildName=\"%ls\" WHERE GuildID=%u", wszGuildName, guildId);
}

void CDojoRepository::UpdateNotice(WCHAR* wszLeaderName, char* rawMessage, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE dojos SET LeaderName=\"%ls\", Notice=\"%s\" WHERE GuildId=%u", wszLeaderName, GetCharDB.EscapeString(rawMessage).c_str(), guildId);
}
