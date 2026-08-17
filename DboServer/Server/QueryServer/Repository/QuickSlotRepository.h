#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "NtlItem.h"

class CQuickSlotRepository : public CNtlSingleton<CQuickSlotRepository>
{

public:

	CQuickSlotRepository() {}
	virtual ~CQuickSlotRepository() {}

public:

	void						DeleteQuickSlots(CHARACTERID charId);
	void						DeleteQuickTeleports(CHARACTERID charId);

	void						UpsertQuickSlot(CHARACTERID charId, TBLIDX tblidx, BYTE bySlotID, BYTE byType, ITEMID item);
	void						DeleteQuickSlot(CHARACTERID charId, BYTE bySlot);

	smart_ptr<QueryResult>		LoadQuickTeleports(CHARACTERID charId);
	void						UpsertQuickTeleport(CHARACTERID charId, BYTE bySlotNum, TBLIDX worldTblidx, float locX, float locY, float locZ, TBLIDX mapNameTblidx, BYTE day, BYTE hour, BYTE minute, BYTE month, BYTE second, WORD year);
	void						DeleteQuickTeleportBySlot(CHARACTERID charId, BYTE bySlot);

};

#define GetQuickSlotRepository()		CQuickSlotRepository::GetInstance()
#define g_pQuickSlotRepository			GetQuickSlotRepository()
