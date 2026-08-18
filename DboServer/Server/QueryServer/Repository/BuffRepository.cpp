#include "stdafx.h"
#include "BuffRepository.h"
#include "../QueryServer.h"


void CBuffRepository::DeleteBuffs(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM buffs WHERE CharID=%u", charId);
}

void CBuffRepository::DeleteBuff(CHARACTERID charId, DWORD buffIndex)
{
	GetCharDB.Execute("DELETE FROM buffs WHERE CharID=%u AND BuffIndex=%u", charId, buffIndex);
}

void CBuffRepository::InsertBuff(CHARACTERID charId, DWORD sourceTblidx, BYTE bySourceType, DWORD buffIndex, BYTE byBuffGroup, DWORD dwInitialDuration, DWORD dwTimeRemaining,
	float effectValue1, float effectValue2, DWORD arg1_0, DWORD arg1_1, DWORD arg1_2, DWORD arg2_0, DWORD arg2_1, DWORD arg2_2)
{
	GetCharDB.Execute("INSERT INTO buffs (CharID,SourceTblidx,SourceType,BuffIndex,BuffGroup,InitialDuration,TimeRemaining,effectValue1,effectValue2,Argument1_0,Argument1_1,Argument1_2,Argument2_0,Argument2_1,Argument2_2) VALUES (%u,%u,%u,%u,%u,%u,%u,%f,%f,%u,%u,%u,%u,%u,%u)",
		charId, sourceTblidx, bySourceType, buffIndex, byBuffGroup, dwInitialDuration, dwTimeRemaining, effectValue1, effectValue2, arg1_0, arg1_1, arg1_2, arg2_0, arg2_1, arg2_2);
}

void CBuffRepository::UpdateBuffTime(DWORD dwTimeRemaining, DWORD arg1_1, DWORD arg1_2, CHARACTERID charId, DWORD buffIndex)
{
	GetCharDB.Execute("UPDATE buffs SET TimeRemaining=%u,Argument1_1=%u,Argument1_2=%u WHERE CharID=%u AND BuffIndex=%u", dwTimeRemaining, arg1_1, arg1_2, charId, buffIndex);
}
