#include "stdafx.h"
#include "MascotRepository.h"
#include "../QueryServer.h"


void CMascotRepository::DeleteMascots(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM mascots WHERE CharID=%u", charId);
}

void CMascotRepository::InsertMascot(CHARACTERID charId, BYTE bySlotId, DWORD tblidx, DWORD curVP, DWORD maxVP)
{
	GetCharDB.Execute("INSERT INTO mascots (CharID, SlotID, MascotTblidx, CurVP, MaxVP) VALUES (%u,%u,%u,%u,%u)", charId, bySlotId, tblidx, curVP, maxVP);
}

void CMascotRepository::DeleteMascotBySlot(CHARACTERID charId, BYTE bySlotId)
{
	GetCharDB.Execute("DELETE FROM mascots WHERE CharID=%u AND SlotID=%u", charId, bySlotId);
}

void CMascotRepository::UpdateMascotSkill(BYTE bySlotId, DWORD skillTblidx, CHARACTERID charId, BYTE byMascotIndex)
{
	GetCharDB.Execute("UPDATE mascots SET skillTblidx%u=%u WHERE CharID=%u AND SlotID=%u", bySlotId, skillTblidx, charId, byMascotIndex);
}

void CMascotRepository::UpdateMascotFusion(DWORD nextMascotTblidx, DWORD dwMaxVP, CHARACTERID charId, BYTE byMascotIndex)
{
	GetCharDB.Execute("UPDATE mascots SET MascotTblidx=%u, MaxVP=%u, CurExp=0 WHERE CharID=%u AND SlotID=%u", nextMascotTblidx, dwMaxVP, charId, byMascotIndex);
}

void CMascotRepository::UpdateMascotVpExp(DWORD curVP, DWORD curExp, CHARACTERID charId, BYTE bySlotId)
{
	GetCharDB.Execute("UPDATE mascots SET CurVP=%u, CurExp=%u WHERE CharID=%u AND SlotID=%u", curVP, curExp, charId, bySlotId);
}
