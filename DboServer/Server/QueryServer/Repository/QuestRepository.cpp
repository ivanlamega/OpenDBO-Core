#include "stdafx.h"
#include "QuestRepository.h"
#include "../QueryServer.h"


void CQuestRepository::DeleteQuestItems(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM questitems WHERE char_id=%u", charId);
}

void CQuestRepository::DeleteQuests(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM quests WHERE char_id=%u", charId);
}

void CQuestRepository::UpdateQuestItemAmount(BYTE byCurCount, CHARACTERID charId, BYTE byPos)
{
	GetCharDB.Execute("UPDATE questitems SET amount=%u WHERE char_id=%u AND pos=%u", byCurCount, charId, byPos);
}

void CQuestRepository::InsertQuestItem(CHARACTERID charId, TBLIDX itemTblidx, BYTE byCurCount, BYTE byPos)
{
	GetCharDB.Execute("INSERT INTO questitems (char_id,tblidx,amount,pos) VALUES (%u,%u,%u,%u)", charId, itemTblidx, byCurCount, byPos);
}

void CQuestRepository::DeleteQuestItem(CHARACTERID charId, BYTE byPos)
{
	GetCharDB.Execute("DELETE FROM questitems WHERE char_id=%u AND pos=%u", charId, byPos);
}

void CQuestRepository::UpdateQuestItemPos(BYTE byDestPos, CHARACTERID charId, BYTE bySrcPos)
{
	GetCharDB.Execute("UPDATE questitems SET pos=%u WHERE char_id=%u AND pos=%u", byDestPos, charId, bySrcPos);
}

void CQuestRepository::UpdateQuestItemPosByTblidx(BYTE byDestPos, CHARACTERID charId, TBLIDX tblidx, BYTE bySrcPos)
{
	GetCharDB.Execute("UPDATE questitems SET pos=%u WHERE char_id=%u AND tblidx=%u AND pos=%u", byDestPos, charId, tblidx, bySrcPos);
}

void CQuestRepository::DeleteQuestProgress(CHARACTERID charId, DWORD questId)
{
	GetCharDB.Execute("DELETE FROM quests WHERE char_id=%u AND quest_id=%u", charId, questId);
}

void CQuestRepository::UpsertQuestProgress(CHARACTERID charId, DWORD questId, DWORD tcQuestInfo, DWORD taQuestInfo, DWORD tgExcCGroup, DWORD tcPreId, DWORD tcCurId, DWORD tcId, DWORD taId,
	DWORD userData0, DWORD userData1, DWORD userData2, DWORD userData3, DWORD exceptTcId, DWORD exceptTaId, DWORD remainTime, DWORD qState)
{
	GetCharDB.Execute("INSERT INTO quests (char_id,quest_id,tc_quest_info,ta_quest_info,tg_exc_c_group,tc_pre_id,tc_cur_id,tc_id,ta_id,evt_user_data,evt_user_data_2,evt_user_data_3,evt_user_data_4,tc_time_info,ta_time_info,time_left,q_state)"
		"VALUES(%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)"
		"ON DUPLICATE KEY UPDATE "
		"char_id=VALUES(char_id),quest_id=VALUES(quest_id),tc_quest_info=VALUES(tc_quest_info),ta_quest_info=VALUES(ta_quest_info),tg_exc_c_group=VALUES(tg_exc_c_group),tc_pre_id=VALUES(tc_pre_id),tc_cur_id=VALUES(tc_cur_id),tc_id=VALUES(tc_id),ta_id=VALUES(ta_id),"
		"evt_user_data=VALUES(evt_user_data),evt_user_data_2=VALUES(evt_user_data_2),evt_user_data_3=VALUES(evt_user_data_3),evt_user_data_4=VALUES(evt_user_data_4),tc_time_info=VALUES(tc_time_info),ta_time_info=VALUES(ta_time_info),time_left=VALUES(time_left),q_state=VALUES(q_state)",
		charId, questId,
		tcQuestInfo, taQuestInfo, tgExcCGroup,
		tcPreId, tcCurId, tcId, taId,
		userData0, userData1, userData2, userData3,
		exceptTcId, exceptTaId, remainTime, qState);
}
