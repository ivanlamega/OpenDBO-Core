#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "NtlSharedType.h"

class CSkillRepository : public CNtlSingleton<CSkillRepository>
{

public:

	CSkillRepository() {}
	virtual ~CSkillRepository() {}

public:

	void						InsertSkill(DWORD skillId, CHARACTERID ownerId, int slotId);
	void						DeleteSkills(CHARACTERID charId);
	void						DeleteHtbSkills(CHARACTERID charId);

	void						UpdateSkill(DWORD skillId, BYTE bIsRpBonusAuto, BYTE byRpBonusType, DWORD nRemainSec, CHARACTERID charId, BYTE skillIndex);
	void						UpdateSkillId(DWORD newSkillId, CHARACTERID charId, BYTE bySlot);
	void						UpdateSkillTimeRemaining(DWORD nRemainSec, CHARACTERID charId, BYTE skillIndex);
	void						UpdateHtbSkillTimeRemaining(DWORD dwSkillTime, DWORD skillId, CHARACTERID charId);
	void						InsertHtbSkill(DWORD skillId, CHARACTERID charId, BYTE skillIndex);
	void						DeleteSkillBySlot(BYTE bySlotId, CHARACTERID charId);
	void						UpdateSkillIdAndBonus(DWORD newSkillTblidx, BYTE bRpBonusAuto, BYTE byRpBonusType, BYTE bySlotId, CHARACTERID charId);

};

#define GetSkillRepository()		CSkillRepository::GetInstance()
#define g_pSkillRepository			GetSkillRepository()
