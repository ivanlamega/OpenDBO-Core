#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "NtlBudokai.h"

class CBudokaiRepository : public CNtlSingleton<CBudokaiRepository>
{

public:

	CBudokaiRepository() {}
	virtual ~CBudokaiRepository() {}

public:

	smart_ptr<QueryResult>			LoadBudokaiState();

	void							InsertWinnerIndividual(WORD wSeasonCount, BYTE byBudokaiType, BYTE byMatchType, CHARACTERID winnerCharId);
	void							InsertWinnerTeam(WORD wSeasonCount, BYTE byBudokaiType, BYTE byMatchType, CHARACTERID winnerCharId1, CHARACTERID winnerCharId2, CHARACTERID winnerCharId3, CHARACTERID winnerCharId4, CHARACTERID winnerCharId5);

	void							UpdateSeasonCount(WORD wSeasonCount);

};

#define GetBudokaiRepository()			CBudokaiRepository::GetInstance()
#define g_pBudokaiRepository			GetBudokaiRepository()
