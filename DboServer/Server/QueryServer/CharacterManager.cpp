#include "stdafx.h"
#include "CharacterManager.h"
#include "QueryServer.h"
#include "Repository/CharacterRepository.h"


CCharacterManager::CCharacterManager()
{
	Init();
}

CCharacterManager::~CCharacterManager()
{
}


void CCharacterManager::Init()
{
	m_uiLastCharacterId = 0;

	smart_ptr<QueryResult> result = g_pCharacterRepository->GetMaxCharId();
	if (result)
	{
		Field* f = result->Fetch();

		m_uiLastCharacterId = f[0].GetUInt32();
	}

	ERR_LOG(LOG_GENERAL, "Last Character-ID %u", m_uiLastCharacterId);
}


void CCharacterManager::CreateCharacter(ACCOUNTID accountId, sPC_SUMMARY& sSum, SERVERFARMID serverFarmId)
{
	UNREFERENCED_PARAMETER(serverFarmId);

	g_pCharacterRepository->InsertCharacter(sSum, accountId);
}