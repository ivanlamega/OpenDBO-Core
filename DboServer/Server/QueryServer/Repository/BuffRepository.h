#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "NtlSharedType.h"

class CBuffRepository : public CNtlSingleton<CBuffRepository>
{

public:

	CBuffRepository() {}
	virtual ~CBuffRepository() {}

public:

	void						DeleteBuffs(CHARACTERID charId);
	void						DeleteBuff(CHARACTERID charId, DWORD buffIndex);

	void						InsertBuff(CHARACTERID charId, DWORD sourceTblidx, BYTE bySourceType, DWORD buffIndex, BYTE byBuffGroup, DWORD dwInitialDuration, DWORD dwTimeRemaining,
									float effectValue1, float effectValue2, DWORD arg1_0, DWORD arg1_1, DWORD arg1_2, DWORD arg2_0, DWORD arg2_1, DWORD arg2_2);

	void						UpdateBuffTime(DWORD dwTimeRemaining, DWORD arg1_1, DWORD arg1_2, CHARACTERID charId, DWORD buffIndex);

};

#define GetBuffRepository()		CBuffRepository::GetInstance()
#define g_pBuffRepository			GetBuffRepository()
