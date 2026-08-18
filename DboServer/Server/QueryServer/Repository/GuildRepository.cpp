#include "stdafx.h"
#include "GuildRepository.h"
#include "../QueryServer.h"


smart_ptr<QueryResult> CGuildRepository::LoadGuilds()
{
	return GetCharDB.Query("SELECT * FROM guilds");
}

smart_ptr<QueryResult> CGuildRepository::LoadGuildMembers(GUILDID guildId, DWORD dwLimit)
{
	return GetCharDB.Query("SELECT id,char_name,account_id,level,race,class,reputation FROM characters WHERE guild_id=%u LIMIT %u", guildId, dwLimit);
}

smart_ptr<QueryResult> CGuildRepository::GetZeniByGuildId(GUILDID guildId)
{
	return GetCharDB.Query("SELECT zeni FROM guilds WHERE id=%u LIMIT 1", guildId);
}

void CGuildRepository::InsertGuild(GUILDID guildId, WCHAR* wszName, CHARACTERID guildMaster, QWORD qwGuildFunctionFlag)
{
	GetCharDB.WaitExecute("INSERT INTO guilds(id,guild_name,master_char_id,function_flag)VALUES(%u, \"%ls\", %u, %I64u)", guildId, wszName, guildMaster, qwGuildFunctionFlag);
}

void CGuildRepository::SetCharacterGuild(CHARACTERID charId, GUILDID guildId, WCHAR* wszGuildName)
{
	GetCharDB.Execute("UPDATE characters SET guild_id=%u, guild_name=\"%ls\" WHERE id=%u", guildId, wszGuildName, charId);
}

void CGuildRepository::InsertGuildMember(GUILDID guildId, CHARACTERID charId)
{
	GetCharDB.Execute("INSERT INTO guild_members(guild_id,char_id)VALUES(%u,%u)", guildId, charId);
}

void CGuildRepository::RemoveMemberGuildFlag(CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET guild_id=0, guild_name=(null) WHERE id=%u", charId);
}

void CGuildRepository::DeleteGuildMember(GUILDID guildId, CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM guild_members WHERE guild_id=%u AND char_id=%u", guildId, charId);
}

void CGuildRepository::RemoveAllMembersGuildFlag(GUILDID guildId)
{
	GetCharDB.Execute("UPDATE characters SET guild_id=0, guild_name=(null) WHERE guild_id=%u", guildId);
}

void CGuildRepository::DeleteGuildMembers(GUILDID guildId)
{
	GetCharDB.Execute("DELETE FROM guild_members WHERE guild_id=%u", guildId);
}

void CGuildRepository::DeleteGuild(GUILDID guildId)
{
	GetCharDB.Execute("DELETE FROM guilds WHERE id=%u", guildId);
}

void CGuildRepository::UpdateSecondMaster(const char* columnName, CHARACTERID value, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET %s=%u WHERE id=%u", columnName, value, guildId);
}

void CGuildRepository::UpdateGuildMaster(CHARACTERID masterCharId, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET master_char_id=%u WHERE id=%u", masterCharId, guildId);
}

void CGuildRepository::UpdateReputationAndFunctionFlag(DWORD reputation, QWORD functionFlag, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET guild_reputation=%u, function_flag=%I64u WHERE id=%u", reputation, functionFlag, guildId);
}

void CGuildRepository::UpdateReputationAndPointEver(DWORD reputation, DWORD pointEver, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET guild_reputation=%u, guild_point_ever=%u WHERE id=%u", reputation, pointEver, guildId);
}

void CGuildRepository::UpdateNotice(WCHAR* wszNoticeBy, char* rawMessage, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET notice_by=\"%ls\", guild_notice=\"%s\" WHERE id=%u", wszNoticeBy, GetCharDB.EscapeString(rawMessage).c_str(), guildId);
}

void CGuildRepository::UpdateMark(sDBO_GUILD_MARK& mark, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET mark_in_color=%u, mark_in_line=%u, mark_main=%u, mark_main_color=%u, mark_out_color=%u, mark_out_line=%u WHERE id=%u",
		mark.byMarkInColor, mark.byMarkInLine, mark.byMarkMain, mark.byMarkMainColor, mark.byMarkOutColor, mark.byMarkOutLine, guildId);
}

void CGuildRepository::InsertNameChangeLog(GUILDID guildId, WCHAR* wszCurrentName, WCHAR* wszNewName)
{
	GetLogDB.Execute("INSERT INTO guild_name_change_log (guild_id,current_name,new_name) VALUES (%u,\"%ls\",\"%ls\")", guildId, wszCurrentName, wszNewName);
}

bool CGuildRepository::UpdateGuildNameWait(WCHAR* wszNewName, GUILDID guildId)
{
	return GetCharDB.WaitExecute("UPDATE guilds SET guild_name=\"%ls\" WHERE id=%u", wszNewName, guildId); //must be wait to avoid 2 people creating the same name in the same time
}

void CGuildRepository::UpdateZeni(DWORD zeni, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE guilds SET zeni=%u WHERE id=%u", zeni, guildId);
}
