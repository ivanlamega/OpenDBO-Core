#include "stdafx.h"
#include "QuickSlotRepository.h"
#include "QueryServer.h"


void CQuickSlotRepository::DeleteQuickSlots(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM quickslot WHERE CharID=%u", charId);
}

void CQuickSlotRepository::DeleteQuickTeleports(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM quick_teleport WHERE CharID=%u", charId);
}

void CQuickSlotRepository::UpsertQuickSlot(CHARACTERID charId, TBLIDX tblidx, BYTE bySlotID, BYTE byType, ITEMID item)
{
	GetCharDB.Execute("INSERT INTO quickslot (CharID,Tblidx,Slot,Type,Item)VALUES(%u,%u,%u,%u,%I64u)"
		"ON DUPLICATE KEY UPDATE CharID=VALUES(CharID),Tblidx=VALUES(Tblidx),Slot=VALUES(Slot),Type=VALUES(Type),Item=VALUES(Item)",
		charId, tblidx, bySlotID, byType, item);
}

void CQuickSlotRepository::DeleteQuickSlot(CHARACTERID charId, BYTE bySlot)
{
	GetCharDB.Execute("DELETE FROM quickslot WHERE CharID=%u AND Slot=%u", charId, bySlot);
}

smart_ptr<QueryResult> CQuickSlotRepository::LoadQuickTeleports(CHARACTERID charId)
{
	return GetCharDB.Query("SELECT * FROM quick_teleport WHERE CharID=%u LIMIT 10", charId); // limit NTL_QUICK_PORTAL_MAX_COUNT
}

void CQuickSlotRepository::UpsertQuickTeleport(CHARACTERID charId, BYTE bySlotNum, TBLIDX worldTblidx, float locX, float locY, float locZ, TBLIDX mapNameTblidx, BYTE day, BYTE hour, BYTE minute, BYTE month, BYTE second, WORD year)
{
	GetCharDB.Execute("INSERT INTO quick_teleport (CharID,SlotNum,WorldTblidx,LocX,LocY,LocZ,MapNameTblidx,day,hour,minute,month,second,year)"
		"VALUES (%u,%u,%u,%f,%f,%f,%u,%u,%u,%u,%u,%u,%u) "
		"ON DUPLICATE KEY UPDATE CharID=VALUES(CharID),SlotNum=VALUES(SlotNum),WorldTblidx=VALUES(WorldTblidx),LocX=VALUES(LocX),LocY=VALUES(LocY),LocZ=VALUES(LocZ),MapNameTblidx=VALUES(MapNameTblidx),day=VALUES(day),hour=VALUES(hour),minute=VALUES(minute),month=VALUES(month),second=VALUES(second),year=VALUES(year)"
		, charId, bySlotNum, worldTblidx, locX, locY, locZ, mapNameTblidx,
		day, hour, minute, month, second, year);
}

void CQuickSlotRepository::DeleteQuickTeleportBySlot(CHARACTERID charId, BYTE bySlot)
{
	GetCharDB.Execute("DELETE FROM quick_teleport WHERE CharID=%u AND SlotNum=%u", charId, bySlot);
}
