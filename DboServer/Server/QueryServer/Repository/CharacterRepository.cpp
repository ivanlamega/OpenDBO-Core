#include "stdafx.h"
#include "CharacterRepository.h"
#include "../QueryServer.h"
#include "../PlayerCache.h"
#include "../GameServerSession.h"


smart_ptr<QueryResult> CCharacterRepository::GetMaxCharId()
{
	return GetCharDB.Query("SELECT MAX(CharID) FROM characters");
}

smart_ptr<QueryResult> CCharacterRepository::GetByName(WCHAR* wszName)
{
	return GetCharDB.Query("SELECT CharID FROM characters WHERE CharName='%ls'", wszName);
}

smart_ptr<QueryResult> CCharacterRepository::GetCharIdByName(WCHAR* wszName)
{
	return GetCharDB.Query("SELECT CharID FROM characters WHERE CharName=\"%ls\"", wszName);
}

smart_ptr<QueryResult> CCharacterRepository::GetGuildIdByCharId(CHARACTERID charId)
{
	return GetCharDB.Query("SELECT GuildID FROM characters WHERE CharID=%u", charId);
}

void CCharacterRepository::DeleteCharacter(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM characters WHERE CharID=%u", charId);
}

void CCharacterRepository::InsertBind(CHARACTERID charId, DWORD worldId, float locX, float locY, float locZ, float dirX, float dirY, float dirZ)
{
	GetCharDB.Execute("INSERT INTO bind (CharID,WorldID,LocX,LocY,LocZ,DirX,DirY,DirZ) VALUES (%u,%u,%f,%f,%f,%f,%f,%f)",
		charId, worldId, locX, locY, locZ, dirX, dirY, dirZ);
}

void CCharacterRepository::DeleteBind(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM bind WHERE CharID=%u", charId);
}

void CCharacterRepository::InsertPortal(CHARACTERID charId, DWORD pointId)
{
	GetCharDB.Execute("INSERT INTO portals (CharID,Point) VALUES (%u,%u)", charId, pointId);
}

void CCharacterRepository::DeletePortals(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM portals WHERE CharID=%u", charId);
}

void CCharacterRepository::DeleteTitles(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM titles WHERE CharID=%u", charId);
}

void CCharacterRepository::DeleteWarfog(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM warfog WHERE CharID=%u", charId);
}

void CCharacterRepository::DeleteRankBattle(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM rank_battle WHERE CharID=%u", charId);
}

void CCharacterRepository::UpdateMoney(CHARACTERID charId, DWORD money)
{
	GetCharDB.Execute("UPDATE characters SET Money=%u WHERE CharID=%u", money, charId);
}

void CCharacterRepository::UpdateGuildNameForMembers(GUILDID guildId, WCHAR* wszGuildName)
{
	GetCharDB.Execute("UPDATE characters SET GuildName=\"%ls\" WHERE GuildID=%u", wszGuildName, guildId);
}

void CCharacterRepository::UpdateWaguPoint(CHARACTERID charId, DWORD waguPoint)
{
	GetCharDB.Execute("UPDATE characters SET WaguPoint=%u WHERE CharID=%u", waguPoint, charId);
}

void CCharacterRepository::UpdateSpPoint(DWORD sp, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET SpPoint=%u WHERE CharID=%u", sp, charId);
}

void CCharacterRepository::UpdateLevelExpSp(BYTE byLevel, DWORD dwExp, DWORD dwSp, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET Level=%u,Exp=%u,SpPoint=%u WHERE CharID=%u", byLevel, dwExp, dwSp, charId);
}

void CCharacterRepository::SavePcData(DWORD dwExp, float posX, float posY, float posZ, float dirX, float dirZ, WORD worldId, TBLIDX worldTblidx, DWORD mapInfoIndex,
	DWORD tutorialHint, DWORD charLp, WORD wEP, WORD wRP, DWORD charAP, TBLIDX charTitle, TBLIDX mascotTblidx, BYTE byCurRPBall, char* ip, DWORD eAirState, DWORD addPlayTime, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET Exp=%u,CurLocX=%f,CurLocY=%f,CurLocZ=%f,CurDirX=%f,CurDirZ=%f,WorldID=%u,WorldTable=%u,MapInfoIndex=%u,TutorialHint=%u,CurLP=%u,CurEP=%u,CurRP=%u,CurAP=%u,Title=%u,Mascot=%u,RpBall=%u,IP=\"%s\",AirState=%u,PlayTime=PlayTime+%u WHERE CharID=%u",
		dwExp, posX, posY, posZ, dirX, dirZ, worldId, worldTblidx, mapInfoIndex,
		tutorialHint, charLp, wEP, wRP, charAP,
		charTitle, mascotTblidx, byCurRPBall, ip, eAirState, addPlayTime,
		charId);
}

void CCharacterRepository::UpdateBind(DWORD worldId, TBLIDX bindObjectTblidx, BYTE byBindType, float locX, float locY, float locZ, float dirX, float dirY, float dirZ, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE bind SET WorldID=%u,BindObjectTblIdx=%u,LocX=%f,LocY=%f,LocZ=%f,DirX=%f,DirY=%f,DirZ=%f,Type=%u WHERE CharID=%u",
		worldId, bindObjectTblidx, locX, locY, locZ, dirX, dirY, dirZ, byBindType, charId);
}

void CCharacterRepository::UpdateClass(BYTE byClass, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET Class=%u WHERE CharID=%u", byClass, charId);
}

void CCharacterRepository::UpdateGender(BYTE byGender, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET Gender=%u WHERE CharID=%u", byGender, charId);
}

void CCharacterRepository::UpdateNetpy(DWORD points, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET Netpy=%u WHERE CharID=%u", points, charId);
}

void CCharacterRepository::UpdateTutorialFlag(int bTutorialFlag, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET TutorialFlag=%i WHERE CharID=%u", bTutorialFlag, charId);
}

void CCharacterRepository::UpdateMudosaPoint(DWORD mudosaPoints, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET MudosaPoint=%u WHERE CharID=%u", mudosaPoints, charId);
}

void CCharacterRepository::UpsertRankBattle(CHARACTERID charId, DWORD win, DWORD draw, DWORD lose, WORD straightKOWin, WORD maxStraightKOWin, WORD maxStraightWin, WORD straightWin, float points)
{
	GetCharDB.Execute("INSERT INTO rank_battle (CharID,Win,Draw,Lose,StraightKOWin,MaxStraightKOWin,MaxStraightWin,StraightWin,Points) VALUES (%u,%u,%u,%u,%u,%u,%u,%u,%f) ON DUPLICATE KEY UPDATE "
		"Win=VALUES(Win),Draw=VALUES(Draw),Lose=VALUES(Lose),StraightKOWin=VALUES(StraightKOWin),MaxStraightKOWin=VALUES(MaxStraightKOWin),MaxStraightWin=VALUES(MaxStraightWin),StraightWin=VALUES(StraightWin),Points=VALUES(Points)",
		charId, win, draw, lose, straightKOWin, maxStraightKOWin, maxStraightWin, straightWin, points);
}

smart_ptr<QueryResult> CCharacterRepository::GetCharIdAndAwayByName(char* nameUtf8)
{
	return GetCharDB.Query("SELECT CharID, MailIsAway FROM characters WHERE CharName=\"%s\"", GetCharDB.EscapeString(nameUtf8).c_str());
}

smart_ptr<QueryResult> CCharacterRepository::GetCharIdByNameUtf8(const char* nameUtf8)
{
	return GetCharDB.Query("SELECT CharID FROM characters WHERE CharName=\"%s\" LIMIT 1", nameUtf8);
}

void CCharacterRepository::UpdateIsMailAway(BOOL bIsAway, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET MailIsAway=%i WHERE CharID=%u", bIsAway, charId);
}

void CCharacterRepository::InsertWarFog(CHARACTERID charId, DWORD contentsTblidx)
{
	GetCharDB.Execute("INSERT INTO warfog VALUES (%u,%u)", charId, contentsTblidx);
}

void CCharacterRepository::UpdateMoneyAndMudosa(DWORD money, DWORD mudosaPoint, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET Money=%u,MudosaPoint=%u WHERE CharID=%u", money, mudosaPoint, charId);
}

void CCharacterRepository::UpdateSpPointAdd(DWORD addSP, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET SpPoint=SpPoint+%u WHERE CharID=%u", addSP, charId);
}

void CCharacterRepository::UpdateMoneyAndHoipoi(DWORD money, BYTE byMixLevel, DWORD mixExp, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET Money=%u, Hoipoi_MixLevel=%u, Hoipoi_MixExp=%u WHERE CharID=%u", money, byMixLevel, mixExp, charId);
}

void CCharacterRepository::SaveRunTimeData(DWORD dwExp, float locX, float locY, float locZ, float dirX, float dirY, float dirZ, WORD worldId, TBLIDX worldTblidx, DWORD addPlayTime, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET Exp=%u, CurLocX=%f,CurLocY=%f,CurLocZ=%f, CurDirX=%f,CurDirY=%f,CurDirZ=%f, WorldID=%u,WorldTable=%u, PlayTime=PlayTime+%u WHERE CharID=%u",
		dwExp, locX, locY, locZ, dirX, dirY, dirZ, worldId, worldTblidx, addPlayTime, charId);
}

void CCharacterRepository::InsertTitle(CHARACTERID charId, TBLIDX titleTblidx)
{
	GetCharDB.Execute("INSERT INTO titles (CharID,TitleTblidx) VALUES (%u,%u)", charId, titleTblidx);
}

void CCharacterRepository::DeleteTitle(CHARACTERID charId, TBLIDX titleTblidx)
{
	GetCharDB.Execute("DELETE FROM titles WHERE CharID=%u AND TitleTblidx=%u", charId, titleTblidx);
}

void CCharacterRepository::UpdateAdult(BOOL bIsAdult, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET Adult=%i WHERE CharID=%u", bIsAdult, charId);
}

void CCharacterRepository::UpdateMoneyCharId64(DWORD money, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET Money=%u WHERE CharID=%I64u", money, charId);
}

smart_ptr<QueryResult> CCharacterRepository::GetAccountIdByCharName(WCHAR* wszName)
{
	return GetCharDB.Query("SELECT AccountID FROM characters WHERE CharName=\"%ls\"", wszName);
}

void CCharacterRepository::UpdateInvisibleCostume(BOOL bInvisibleCostume, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE characters SET InvisibleCostume=%i WHERE CharID=%u", bInvisibleCostume, charId);
}

smart_ptr<QueryResult> CCharacterRepository::GetByNameTrailingSpace(WCHAR* wszName)
{
	return GetCharDB.Query("SELECT CharID FROM characters WHERE CharName=\"%ls\" ", wszName);
}

bool CCharacterRepository::UpdateCharNameWait(WCHAR* wszName, CHARACTERID charId)
{
	return GetCharDB.WaitExecute("UPDATE characters SET CharName=\"%ls\" WHERE CharID=%u", wszName, charId); //required waitexecute so none can get the same name
}

void CCharacterRepository::CheckAccountForCharacterAsync(CGameServerSession* pSession, CHARACTERID charId, ACCOUNTID accountId)
{
	SQLCallbackBase* pCallBack3 = new SQLClassCallbackP2<CGameServerSession, CHARACTERID, ACCOUNTID>(pSession, &CGameServerSession::OnLoadPcCheck, charId, accountId);
	AsyncQuery * q3 = new AsyncQuery(pCallBack3);
	q3->AddQuery("SELECT AccountID FROM characters WHERE CharID=%u", charId);
	GetCharDB.QueueAsyncQuery(q3);
}

void CCharacterRepository::LoadCharacterCoreDataAsync(CPlayerCache* pPlayerCache, CHARACTERID charId)
{
	SQLCallbackBase* pCallBack = new SQLClassCallbackP0<CPlayerCache>(pPlayerCache, &CPlayerCache::OnLoadPcData);
	AsyncQuery * q = new AsyncQuery(pCallBack);
	q->AddQuery("SELECT * FROM characters WHERE CharID=%u", charId);
	q->AddQuery("SELECT * FROM bind WHERE CharID=%u", charId);
	q->AddQuery("SELECT id, SenderType, IsRead FROM mail WHERE CharID=%u", charId); //NO NEED LIMIT HERE. IT WILL RELOAD ALL
	q->AddQuery("SELECT * FROM rank_battle WHERE CharID=%u", charId);
	q->AddQuery("SELECT TitleTblidx FROM titles WHERE CharID=%u LIMIT 64", charId); // LIMIT NTL_MAX_CHAR_TITLE_COUNT_IN_FLAG
	q->AddQuery("SELECT WarFog FROM warfog WHERE CharID=%u", charId);
	GetCharDB.QueueAsyncQuery(q);
}

void CCharacterRepository::LoadCharacterExtendedDataAsync(CPlayerCache* pPlayerCache, CHARACTERID charId)
{
	SQLCallbackBase* pCallBack2 = new SQLClassCallbackP0<CPlayerCache>(pPlayerCache, &CPlayerCache::OnLoadPcData2);
	AsyncQuery * q2 = new AsyncQuery(pCallBack2);
	q2->AddQuery("SELECT * FROM items WHERE owner_id=%u AND place < 7 LIMIT 166", charId); // NTL_MAX_BAGSLOT_COUNT + NTL_MAX_ITEM_SLOT + (NTL_MAX_BAG_ITEM_SLOT * 4) + EQUIP_SLOT_TYPE_COUNT | 7 = CONTAINER_TYPE_BANKSLOT
	q2->AddQuery("SELECT * FROM skills WHERE owner_id=%u LIMIT 60", charId);
	q2->AddQuery("SELECT * FROM htb_skills WHERE owner_id=%u LIMIT 2", charId); // limit NTL_HTB_MAX_PC_HAVE_HTB_SKILL
	q2->AddQuery("SELECT * FROM buffs WHERE CharID=%u LIMIT 49", charId);
	q2->AddQuery("SELECT * FROM questitems WHERE CharID=%u LIMIT 30", charId);// LIMIT NTL_QUEST_INVENTORY_SLOT_COUNT
	q2->AddQuery("SELECT * FROM quests WHERE CharID=%u", charId);
	q2->AddQuery("SELECT * FROM quickslot WHERE CharID=%u LIMIT 48", charId);
	q2->AddQuery("SELECT * FROM mascots WHERE CharID=%u LIMIT 40", charId);
	q2->AddQuery("SELECT * FROM hoipoi_recipe WHERE CharID=%u LIMIT 200", charId);
	q2->AddQuery("SELECT * FROM items_cd WHERE CharID=%u LIMIT 19", charId);
	q2->AddQuery("SELECT Point FROM portals WHERE CharID=%u LIMIT 100", charId);
	GetCharDB.QueueAsyncQuery(q2);
}

void CCharacterRepository::InsertCharacter(sPC_SUMMARY& sSum, ACCOUNTID accountId)
{
	GetCharDB.Execute("INSERT INTO characters (CharID,CharName,AccountID,Race,Class,Gender,Face,Hair,HairColor,SkinColor,CurLocX,CurLocY,CurLocZ,WorldID,WorldTable,MapInfoIndex,CreateTime)"
		"VALUES (%u,'%ls',%u,%u,%u,%u,%u,%u,%u,%u,%f,%f,%f,%u,%u,%u,%I64u)",
		sSum.charId, sSum.awchName, accountId, sSum.byRace, sSum.byClass, sSum.byGender, sSum.byFace, sSum.byHair, sSum.byHairColor, sSum.bySkinColor,
		sSum.fPositionX, sSum.fPositionY, sSum.fPositionZ,
		sSum.worldId, sSum.worldTblidx, sSum.dwMapInfoIndex, time(0));
}
