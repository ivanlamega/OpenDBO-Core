#include "stdafx.h"
#include "QuestRepository.h"
#include "QueryServer.h"


void CQuestRepository::DeleteQuestItems(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM questitems WHERE CharID=%u", charId);
}

void CQuestRepository::DeleteQuests(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM quests WHERE CharID=%u", charId);
}

void CQuestRepository::UpdateQuestItemAmount(BYTE byCurCount, CHARACTERID charId, BYTE byPos)
{
	GetCharDB.Execute("UPDATE questitems SET amount=%u WHERE CharID=%u AND pos=%u", byCurCount, charId, byPos);
}

void CQuestRepository::InsertQuestItem(CHARACTERID charId, TBLIDX itemTblidx, BYTE byCurCount, BYTE byPos)
{
	GetCharDB.Execute("INSERT INTO questitems (CharID,tblidx,amount,pos) VALUES (%u,%u,%u,%u)", charId, itemTblidx, byCurCount, byPos);
}

void CQuestRepository::DeleteQuestItem(CHARACTERID charId, BYTE byPos)
{
	GetCharDB.Execute("DELETE FROM questitems WHERE CharID=%u AND pos=%u", charId, byPos);
}

void CQuestRepository::UpdateQuestItemPos(BYTE byDestPos, CHARACTERID charId, BYTE bySrcPos)
{
	GetCharDB.Execute("UPDATE questitems SET pos=%u WHERE CharID=%u AND pos=%u", byDestPos, charId, bySrcPos);
}

void CQuestRepository::UpdateQuestItemPosByTblidx(BYTE byDestPos, CHARACTERID charId, TBLIDX tblidx, BYTE bySrcPos)
{
	GetCharDB.Execute("UPDATE questitems SET pos=%u WHERE CharID=%u AND tblidx=%u AND pos=%u", byDestPos, charId, tblidx, bySrcPos);
}

void CQuestRepository::DeleteQuestProgress(CHARACTERID charId, DWORD questId)
{
	GetCharDB.Execute("DELETE FROM quests WHERE CharID=%u AND QuestID=%u", charId, questId);
}

void CQuestRepository::UpsertQuestProgress(CHARACTERID charId, DWORD questId, DWORD tcQuestInfo, DWORD taQuestInfo, DWORD tgExcCGroup, DWORD tcPreId, DWORD tcCurId, DWORD tcId, DWORD taId,
	DWORD userData0, DWORD userData1, DWORD userData2, DWORD userData3, DWORD exceptTcId, DWORD exceptTaId, DWORD remainTime, DWORD qState)
{
	GetCharDB.Execute("INSERT INTO quests (CharID,QuestID,tcQuestInfo,taQuestInfo,tgExcCGroup,tcPreId,tcCurId,tcId,taId,evtUserData,evtUserData2,evtUserData3,evtUserData4,tcTimeInfo,taTimeInfo,TimeLeft,QState)"
		"VALUES(%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)"
		"ON DUPLICATE KEY UPDATE "
		"CharID=VALUES(CharID),QuestID=VALUES(QuestID),tcQuestInfo=VALUES(tcQuestInfo),taQuestInfo=VALUES(taQuestInfo),tgExcCGroup=VALUES(tgExcCGroup),tcPreId=VALUES(tcPreId),tcCurId=VALUES(tcCurId),tcId=VALUES(tcId),taId=VALUES(taId),"
		"evtUserData=VALUES(evtUserData),evtUserData2=VALUES(evtUserData2),evtUserData3=VALUES(evtUserData3),evtUserData4=VALUES(evtUserData4),tcTimeInfo=VALUES(tcTimeInfo),taTimeInfo=VALUES(taTimeInfo),TimeLeft=VALUES(TimeLeft),QState=VALUES(QState)",
		charId, questId,
		tcQuestInfo, taQuestInfo, tgExcCGroup,
		tcPreId, tcCurId, tcId, taId,
		userData0, userData1, userData2, userData3,
		exceptTcId, exceptTaId, remainTime, qState);
}
