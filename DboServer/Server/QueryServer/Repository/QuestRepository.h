#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "NtlSharedType.h"

class CQuestRepository : public CNtlSingleton<CQuestRepository>
{

public:

	CQuestRepository() {}
	virtual ~CQuestRepository() {}

public:

	void						DeleteQuestItems(CHARACTERID charId);
	void						DeleteQuests(CHARACTERID charId);

	void						UpdateQuestItemAmount(BYTE byCurCount, CHARACTERID charId, BYTE byPos);
	void						InsertQuestItem(CHARACTERID charId, TBLIDX itemTblidx, BYTE byCurCount, BYTE byPos);
	void						DeleteQuestItem(CHARACTERID charId, BYTE byPos);
	void						UpdateQuestItemPos(BYTE byDestPos, CHARACTERID charId, BYTE bySrcPos);
	void						UpdateQuestItemPosByTblidx(BYTE byDestPos, CHARACTERID charId, TBLIDX tblidx, BYTE bySrcPos);
	void						DeleteQuestProgress(CHARACTERID charId, DWORD questId);

	void						UpsertQuestProgress(CHARACTERID charId, DWORD questId, DWORD tcQuestInfo, DWORD taQuestInfo, DWORD tgExcCGroup, DWORD tcPreId, DWORD tcCurId, DWORD tcId, DWORD taId,
									DWORD userData0, DWORD userData1, DWORD userData2, DWORD userData3, DWORD exceptTcId, DWORD exceptTaId, DWORD remainTime, DWORD qState);

};

#define GetQuestRepository()		CQuestRepository::GetInstance()
#define g_pQuestRepository			GetQuestRepository()
