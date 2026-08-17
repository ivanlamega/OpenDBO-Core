#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "NtlSharedType.h"

class CMascotRepository : public CNtlSingleton<CMascotRepository>
{

public:

	CMascotRepository() {}
	virtual ~CMascotRepository() {}

public:

	void						DeleteMascots(CHARACTERID charId);
	void						InsertMascot(CHARACTERID charId, BYTE bySlotId, DWORD tblidx, DWORD curVP, DWORD maxVP);
	void						DeleteMascotBySlot(CHARACTERID charId, BYTE bySlotId);
	void						UpdateMascotSkill(BYTE bySlotId, DWORD skillTblidx, CHARACTERID charId, BYTE byMascotIndex);
	void						UpdateMascotFusion(DWORD nextMascotTblidx, DWORD dwMaxVP, CHARACTERID charId, BYTE byMascotIndex);
	void						UpdateMascotVpExp(DWORD curVP, DWORD curExp, CHARACTERID charId, BYTE bySlotId);

};

#define GetMascotRepository()		CMascotRepository::GetInstance()
#define g_pMascotRepository		GetMascotRepository()
