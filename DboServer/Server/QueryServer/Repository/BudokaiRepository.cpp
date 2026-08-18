#include "stdafx.h"
#include "BudokaiRepository.h"
#include "../QueryServer.h"


smart_ptr<QueryResult> CBudokaiRepository::LoadBudokaiState()
{
	return GetLogDB.Query("SELECT * FROM budokai");
}

void CBudokaiRepository::InsertWinnerIndividual(WORD wSeasonCount, BYTE byBudokaiType, BYTE byMatchType, CHARACTERID winnerCharId)
{
	GetLogDB.Execute("INSERT INTO budokai_winners (BudokaiNumber,Type,MatchType,WinnerCharID1) VALUES (%u, %u, %u, %u)", wSeasonCount, byBudokaiType, byMatchType, winnerCharId);
}

void CBudokaiRepository::InsertWinnerTeam(WORD wSeasonCount, BYTE byBudokaiType, BYTE byMatchType, CHARACTERID winnerCharId1, CHARACTERID winnerCharId2, CHARACTERID winnerCharId3, CHARACTERID winnerCharId4, CHARACTERID winnerCharId5)
{
	GetLogDB.Execute("INSERT INTO budokai_winners (BudokaiNumber,Type,MatchType,WinnerCharID1,WinnerCharID2,WinnerCharID3,WinnerCharID4,WinnerCharID5) VALUES (%u, %u, %u, %u, %u, %u, %u, %u)",
		wSeasonCount, byBudokaiType, byMatchType, winnerCharId1, winnerCharId2, winnerCharId3, winnerCharId4, winnerCharId5);
}

void CBudokaiRepository::UpdateSeasonCount(WORD wSeasonCount)
{
	GetLogDB.Execute("UPDATE budokai SET SeasonCount=%u", wSeasonCount);
}
