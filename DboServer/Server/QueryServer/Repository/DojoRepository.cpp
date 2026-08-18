#include "stdafx.h"
#include "DojoRepository.h"
#include "../QueryServer.h"


smart_ptr<QueryResult> CDojoRepository::LoadDojos()
{
	return GetCharDB.Query("SELECT * FROM dojos LIMIT 7");
}

void CDojoRepository::DeleteByGuildId(GUILDID guildId)
{
	GetCharDB.Execute("DELETE FROM dojos WHERE guild_id=%u", guildId);
}

void CDojoRepository::InsertDojo(GUILDID guildId, TBLIDX dojoTblidx, WCHAR* wszGuildName)
{
	GetCharDB.Execute("INSERT INTO dojos (guild_id, dojo_tblidx, guild_name) VALUES (%u,%u,\"%ls\")", guildId, dojoTblidx, wszGuildName);
}

void CDojoRepository::ResetDojo(GUILDID guildId, WCHAR* wszGuildName, GUILDID challengeGuildId, TBLIDX dojoTblidx)
{
	GetCharDB.Execute("UPDATE dojos SET guild_id=%u, level=0, peace_status=0, peace_points=0, guild_name=\"%ls\", leader_name=(null), notice=(null), challenge_guild_id=%u, seed_char_name=(null) WHERE dojo_tblidx=%u",
		guildId, wszGuildName, challengeGuildId, dojoTblidx);
}

void CDojoRepository::DeleteByTblidx(TBLIDX dojoTblidx)
{
	GetCharDB.Execute("DELETE FROM dojos WHERE dojo_tblidx=%u", dojoTblidx);
}

void CDojoRepository::UpdateLevel(BYTE byLevel, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE dojos SET level=%u WHERE guild_id=%u", byLevel, guildId);
}

void CDojoRepository::UpdateSeedCharName(WCHAR* wszCharName, TBLIDX dojoTblidx)
{
	GetCharDB.Execute("UPDATE dojos SET seed_char_name=\"%ls\" WHERE dojo_tblidx=%u", wszCharName, dojoTblidx);
}

void CDojoRepository::ClearSeedCharName(TBLIDX dojoTblidx)
{
	GetCharDB.Execute("UPDATE dojos SET seed_char_name=(Null) WHERE dojo_tblidx=%u", dojoTblidx);
}

void CDojoRepository::UpdateGuildName(GUILDID guildId, WCHAR* wszGuildName)
{
	GetCharDB.Execute("UPDATE dojos SET guild_name=\"%ls\" WHERE guild_id=%u", wszGuildName, guildId);
}

void CDojoRepository::UpdateNotice(WCHAR* wszLeaderName, char* rawMessage, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE dojos SET leader_name=\"%ls\", notice=\"%s\" WHERE guild_id=%u", wszLeaderName, GetCharDB.EscapeString(rawMessage).c_str(), guildId);
}
