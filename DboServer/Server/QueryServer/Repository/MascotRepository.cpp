#include "stdafx.h"
#include "MascotRepository.h"
#include "../QueryServer.h"


void CMascotRepository::DeleteMascots(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM mascots WHERE char_id=%u", charId);
}

void CMascotRepository::InsertMascot(CHARACTERID charId, BYTE bySlotId, DWORD tblidx, DWORD curVP, DWORD maxVP)
{
	GetCharDB.Execute("INSERT INTO mascots (char_id, slot_id, mascot_tblidx, cur_vp, max_vp) VALUES (%u,%u,%u,%u,%u)", charId, bySlotId, tblidx, curVP, maxVP);
}

void CMascotRepository::DeleteMascotBySlot(CHARACTERID charId, BYTE bySlotId)
{
	GetCharDB.Execute("DELETE FROM mascots WHERE char_id=%u AND slot_id=%u", charId, bySlotId);
}

void CMascotRepository::UpdateMascotSkill(BYTE bySlotId, DWORD skillTblidx, CHARACTERID charId, BYTE byMascotIndex)
{
	GetCharDB.Execute("UPDATE mascots SET skill_tblidx_%u=%u WHERE char_id=%u AND slot_id=%u", bySlotId, skillTblidx, charId, byMascotIndex);
}

void CMascotRepository::UpdateMascotFusion(DWORD nextMascotTblidx, DWORD dwMaxVP, CHARACTERID charId, BYTE byMascotIndex)
{
	GetCharDB.Execute("UPDATE mascots SET mascot_tblidx=%u, max_vp=%u, cur_exp=0 WHERE char_id=%u AND slot_id=%u", nextMascotTblidx, dwMaxVP, charId, byMascotIndex);
}

void CMascotRepository::UpdateMascotVpExp(DWORD curVP, DWORD curExp, CHARACTERID charId, BYTE bySlotId)
{
	GetCharDB.Execute("UPDATE mascots SET cur_vp=%u, cur_exp=%u WHERE char_id=%u AND slot_id=%u", curVP, curExp, charId, bySlotId);
}
