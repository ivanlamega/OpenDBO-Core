#include "stdafx.h"
#include "BuffRepository.h"
#include "../QueryServer.h"


void CBuffRepository::DeleteBuffs(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM buffs WHERE char_id=%u", charId);
}

void CBuffRepository::DeleteBuff(CHARACTERID charId, DWORD buffIndex)
{
	GetCharDB.Execute("DELETE FROM buffs WHERE char_id=%u AND buff_index=%u", charId, buffIndex);
}

void CBuffRepository::InsertBuff(CHARACTERID charId, DWORD sourceTblidx, BYTE bySourceType, DWORD buffIndex, BYTE byBuffGroup, DWORD dwInitialDuration, DWORD dwTimeRemaining,
	float effectValue1, float effectValue2, DWORD arg1_0, DWORD arg1_1, DWORD arg1_2, DWORD arg2_0, DWORD arg2_1, DWORD arg2_2)
{
	GetCharDB.Execute("INSERT INTO buffs (char_id,source_tblidx,source_type,buff_index,buff_group,initial_duration,time_remaining,effect_value_1,effect_value_2,argument_1_0,argument_1_1,argument_1_2,argument_2_0,argument_2_1,argument_2_2) VALUES (%u,%u,%u,%u,%u,%u,%u,%f,%f,%u,%u,%u,%u,%u,%u)",
		charId, sourceTblidx, bySourceType, buffIndex, byBuffGroup, dwInitialDuration, dwTimeRemaining, effectValue1, effectValue2, arg1_0, arg1_1, arg1_2, arg2_0, arg2_1, arg2_2);
}

void CBuffRepository::UpdateBuffTime(DWORD dwTimeRemaining, DWORD arg1_1, DWORD arg1_2, CHARACTERID charId, DWORD buffIndex)
{
	GetCharDB.Execute("UPDATE buffs SET time_remaining=%u,argument_1_1=%u,argument_1_2=%u WHERE char_id=%u AND buff_index=%u", dwTimeRemaining, arg1_1, arg1_2, charId, buffIndex);
}
