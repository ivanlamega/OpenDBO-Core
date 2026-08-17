#include "stdafx.h"
#include "GuildRepository.h"
#include "../QueryServer.h"


smart_ptr<QueryResult> CGuildRepository::LoadGuilds()
{
	return GetCharDB.Query("SELECT * FROM guilds");
}

smart_ptr<QueryResult> CGuildRepository::LoadGuildMembers(GUILDID guildId, DWORD dwLimit)
{
	return GetCharDB.Query("SELECT CharID,CharName,AccountID,Level,Race,Class,Reputation FROM characters WHERE GuildID=%u LIMIT %u", guildId, dwLimit);
}

smart_ptr<QueryResult> CGuildRepository::GetZeniByGuildId(GUILDID guildId)
{
	return GetCharDB.Query("SELECT Zeni FROM guilds WHERE GuildID=%u LIMIT 1", guildId);
}

void CGuildRepository::InsertGuild(GUILDID guildId, WCHAR* wszName, CHARACTERID guildMaster, QWORD qwGuildFunctionFlag)
{
	GetCharDB.WaitExecute("INSERT INTO guilds(GuildID,GuildName,GuildMaster,FunctionFlag)VALUES(%u, \"%ls\", %u, %I64u)", guildId, wszName, guildMaster, qwGuildFunctionFlag);
}

void CGuildRepository::SetCharacterGuild(CHARACTERID charId, GUILDID guildId, WCHAR* wszGuildName)
{
	GetCharDB.Execute("UPDATE characters SET GuildID=%u, GuildName=\"%ls\" WHERE CharID=%u", guildId, wszGuildName, charId);
}

void CGuildRepository::InsertGuildMember(GUILDID guildId, CHARACTERID charId)
{
	GetCharDB.Execute("INSERT INTO guild_members(GuildID,CharID)VALUES(%u,%u)", guildId, charId);
}

void CGuildRepository::RemoveMemberGuildFlag(CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET GuildID=0, GuildName=(null) WHERE CharID=%u", charId);
}

void CGuildRepository::DeleteGuildMember(GUILDID guildId, CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM guild_members WHERE GuildID=%u AND CharID=%u", guildId, charId);
}

void CGuildRepository::RemoveAllMembersGuildFlag(GUILDID guildId)
{
	GetCharDB.Execute("UPDATE characters SET GuildID=0, GuildName=(null) WHERE GuildID=%u", guildId);
}

void CGuildRepository::DeleteGuildMembers(GUILDID guildId)
{
	GetCharDB.Execute("DELETE FROM guild_members WHERE GuildID=%u", guildId);
}

void CGuildRepository::DeleteGuild(GUILDID guildId)
{
	GetCharDB.Execute("DELETE FROM guilds WHERE GuildID=%u", guildId);
}

void CGuildRepository::UpdateSecondMaster(const char* columnName, CHARACTERID value, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET %s=%u WHERE GuildID=%u", columnName, value, guildId);
}

void CGuildRepository::UpdateGuildMaster(CHARACTERID masterCharId, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET GuildMaster=%u WHERE GuildID=%u", masterCharId, guildId);
}

void CGuildRepository::UpdateReputationAndFunctionFlag(DWORD reputation, QWORD functionFlag, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET GuildReputation=%u, FunctionFlag=%I64u WHERE GuildID=%u", reputation, functionFlag, guildId);
}

void CGuildRepository::UpdateReputationAndPointEver(DWORD reputation, DWORD pointEver, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET GuildReputation=%u, GuildPointEver=%u WHERE GuildID=%u", reputation, pointEver, guildId);
}

void CGuildRepository::UpdateNotice(WCHAR* wszNoticeBy, char* rawMessage, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET NoticeBy=\"%ls\", GuildNotice=\"%s\" WHERE GuildID=%u", wszNoticeBy, GetCharDB.EscapeString(rawMessage).c_str(), guildId);
}

void CGuildRepository::UpdateMark(sDBO_GUILD_MARK& mark, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET MarkInColor=%u, MarkInLine=%u, MarkMain=%u, MarkMainColor=%u, MarkOutColor=%u, MarkOutLine=%u WHERE GuildID=%u",
		mark.byMarkInColor, mark.byMarkInLine, mark.byMarkMain, mark.byMarkMainColor, mark.byMarkOutColor, mark.byMarkOutLine, guildId);
}

void CGuildRepository::InsertNameChangeLog(GUILDID guildId, WCHAR* wszCurrentName, WCHAR* wszNewName)
{
	GetLogDB.Execute("INSERT INTO guild_name_change_log (GuildID,CurrentName,NewName) VALUES (%u,\"%ls\",\"%ls\")", guildId, wszCurrentName, wszNewName);
}

bool CGuildRepository::UpdateGuildNameWait(WCHAR* wszNewName, GUILDID guildId)
{
	return GetCharDB.WaitExecute("UPDATE guilds SET GuildName=\"%ls\" WHERE GuildID=%u", wszNewName, guildId); //must be wait to avoid 2 people creating the same name in the same time
}

void CGuildRepository::UpdateZeni(DWORD zeni, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET Zeni=%u WHERE GuildID=%u", zeni, guildId);
}
