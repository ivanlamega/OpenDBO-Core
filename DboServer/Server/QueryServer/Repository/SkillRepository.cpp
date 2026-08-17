#include "stdafx.h"
#include "SkillRepository.h"
#include "QueryServer.h"


void CSkillRepository::InsertSkill(DWORD skillId, CHARACTERID ownerId, int slotId)
{
	GetCharDB.Execute("INSERT INTO skills (skill_id,owner_id,SlotID) VALUES (%u,%u,%u)", skillId, ownerId, slotId);
}

void CSkillRepository::DeleteSkills(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM skills WHERE owner_id=%u", charId);
}

void CSkillRepository::DeleteHtbSkills(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM htb_skills WHERE owner_id=%u", charId);
}

void CSkillRepository::UpdateSkill(DWORD skillId, BYTE bIsRpBonusAuto, BYTE byRpBonusType, DWORD nRemainSec, CHARACTERID charId, BYTE skillIndex)
{
	GetCharDB.Execute("UPDATE skills SET skill_id=%u, RpBonusAuto=%u, RpBonusType=%u, TimeRemaining=%u WHERE owner_id=%u AND SlotID=%u", skillId, bIsRpBonusAuto, byRpBonusType, nRemainSec, charId, skillIndex);
}

void CSkillRepository::UpdateSkillId(DWORD newSkillId, CHARACTERID charId, BYTE bySlot)
{
	GetCharDB.Execute("UPDATE skills SET skill_id=%u WHERE owner_id=%u and SlotID=%u", newSkillId, charId, bySlot);
}

void CSkillRepository::UpdateSkillTimeRemaining(DWORD nRemainSec, CHARACTERID charId, BYTE skillIndex)
{
	GetCharDB.Execute("UPDATE skills SET TimeRemaining=%u WHERE owner_id=%u AND SlotID=%u", nRemainSec, charId, skillIndex);
}

void CSkillRepository::UpdateHtbSkillTimeRemaining(DWORD dwSkillTime, DWORD skillId, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE htb_skills SET TimeRemaining=%u WHERE skill_id=%u AND owner_id=%u", dwSkillTime, skillId, charId);
}

void CSkillRepository::InsertHtbSkill(DWORD skillId, CHARACTERID charId, BYTE skillIndex)
{
	GetCharDB.Execute("INSERT INTO htb_skills (skill_id,owner_id,SlotID) VALUES (%u,%u,%u)", skillId, charId, skillIndex);
}

void CSkillRepository::DeleteSkillBySlot(BYTE bySlotId, CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM skills WHERE SlotID=%u AND owner_id=%u", bySlotId, charId);
}

void CSkillRepository::UpdateSkillIdAndBonus(DWORD newSkillTblidx, BYTE bRpBonusAuto, BYTE byRpBonusType, BYTE bySlotId, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE skills SET skill_id=%u, RpBonusAuto=%i, RpBonusType=%u WHERE SlotID=%u AND owner_id=%u", newSkillTblidx, bRpBonusAuto, byRpBonusType, bySlotId, charId);
}
