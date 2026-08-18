#include "stdafx.h"
#include "QuickSlotRepository.h"
#include "../QueryServer.h"


void CQuickSlotRepository::DeleteQuickSlots(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM quickslot WHERE char_id=%u", charId);
}

void CQuickSlotRepository::DeleteQuickTeleports(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM quick_teleport WHERE char_id=%u", charId);
}

void CQuickSlotRepository::UpsertQuickSlot(CHARACTERID charId, TBLIDX tblidx, BYTE bySlotID, BYTE byType, ITEMID item)
{
	GetCharDB.Execute("INSERT INTO quickslot (char_id,tblidx,slot,type,item_id)VALUES(%u,%u,%u,%u,%I64u)"
		"ON DUPLICATE KEY UPDATE char_id=VALUES(char_id),tblidx=VALUES(tblidx),slot=VALUES(slot),type=VALUES(type),item_id=VALUES(item_id)",
		charId, tblidx, bySlotID, byType, item);
}

void CQuickSlotRepository::DeleteQuickSlot(CHARACTERID charId, BYTE bySlot)
{
	GetCharDB.Execute("DELETE FROM quickslot WHERE char_id=%u AND slot=%u", charId, bySlot);
}

smart_ptr<QueryResult> CQuickSlotRepository::LoadQuickTeleports(CHARACTERID charId)
{
	return GetCharDB.Query("SELECT * FROM quick_teleport WHERE char_id=%u LIMIT 10", charId); // limit NTL_QUICK_PORTAL_MAX_COUNT
}

void CQuickSlotRepository::UpsertQuickTeleport(CHARACTERID charId, BYTE bySlotNum, TBLIDX worldTblidx, float locX, float locY, float locZ, TBLIDX mapNameTblidx, BYTE day, BYTE hour, BYTE minute, BYTE month, BYTE second, WORD year)
{
	GetCharDB.Execute("INSERT INTO quick_teleport (char_id,slot_num,world_tblidx,loc_x,loc_y,loc_z,map_name_tblidx,day,hour,minute,month,second,year)"
		"VALUES (%u,%u,%u,%f,%f,%f,%u,%u,%u,%u,%u,%u,%u) "
		"ON DUPLICATE KEY UPDATE char_id=VALUES(char_id),slot_num=VALUES(slot_num),world_tblidx=VALUES(world_tblidx),loc_x=VALUES(loc_x),loc_y=VALUES(loc_y),loc_z=VALUES(loc_z),map_name_tblidx=VALUES(map_name_tblidx),day=VALUES(day),hour=VALUES(hour),minute=VALUES(minute),month=VALUES(month),second=VALUES(second),year=VALUES(year)"
		, charId, bySlotNum, worldTblidx, locX, locY, locZ, mapNameTblidx,
		day, hour, minute, month, second, year);
}

void CQuickSlotRepository::DeleteQuickTeleportBySlot(CHARACTERID charId, BYTE bySlot)
{
	GetCharDB.Execute("DELETE FROM quick_teleport WHERE char_id=%u AND slot_num=%u", charId, bySlot);
}
