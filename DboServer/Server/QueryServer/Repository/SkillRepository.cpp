#include "stdafx.h"
#include "SkillRepository.h"
#include "../QueryServer.h"


void CSkillRepository::InsertSkill(DWORD skillId, CHARACTERID ownerId, int slotId)
{
	GetCharDB.Execute("INSERT INTO skills (skill_id,char_id,slot_id) VALUES (%u,%u,%u)", skillId, ownerId, slotId);
}

void CSkillRepository::DeleteSkills(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM skills WHERE char_id=%u", charId);
}

void CSkillRepository::DeleteHtbSkills(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM htb_skills WHERE char_id=%u", charId);
}

void CSkillRepository::UpdateSkill(DWORD skillId, BYTE bIsRpBonusAuto, BYTE byRpBonusType, DWORD nRemainSec, CHARACTERID charId, BYTE skillIndex)
{
	GetCharDB.Execute("UPDATE skills SET skill_id=%u, rp_bonus_auto=%u, rp_bonus_type=%u, time_remaining=%u WHERE char_id=%u AND slot_id=%u", skillId, bIsRpBonusAuto, byRpBonusType, nRemainSec, charId, skillIndex);
}

void CSkillRepository::UpdateSkillId(DWORD newSkillId, CHARACTERID charId, BYTE bySlot)
{
	GetCharDB.Execute("UPDATE skills SET skill_id=%u WHERE char_id=%u and slot_id=%u", newSkillId, charId, bySlot);
}

void CSkillRepository::UpdateSkillTimeRemaining(DWORD nRemainSec, CHARACTERID charId, BYTE skillIndex)
{
	GetCharDB.Execute("UPDATE skills SET time_remaining=%u WHERE char_id=%u AND slot_id=%u", nRemainSec, charId, skillIndex);
}

void CSkillRepository::UpdateHtbSkillTimeRemaining(DWORD dwSkillTime, DWORD skillId, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE htb_skills SET time_remaining=%u WHERE skill_id=%u AND char_id=%u", dwSkillTime, skillId, charId);
}

void CSkillRepository::InsertHtbSkill(DWORD skillId, CHARACTERID charId, BYTE skillIndex)
{
	GetCharDB.Execute("INSERT INTO htb_skills (skill_id,char_id,slot_id) VALUES (%u,%u,%u)", skillId, charId, skillIndex);
}

void CSkillRepository::DeleteSkillBySlot(BYTE bySlotId, CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM skills WHERE slot_id=%u AND char_id=%u", bySlotId, charId);
}

void CSkillRepository::UpdateSkillIdAndBonus(DWORD newSkillTblidx, BYTE bRpBonusAuto, BYTE byRpBonusType, BYTE bySlotId, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE skills SET skill_id=%u, rp_bonus_auto=%i, rp_bonus_type=%u WHERE slot_id=%u AND char_id=%u", newSkillTblidx, bRpBonusAuto, byRpBonusType, bySlotId, charId);
}
