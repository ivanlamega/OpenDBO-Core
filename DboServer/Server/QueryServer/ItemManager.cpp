#include "stdafx.h"
#include "ItemManager.h"
#include "QueryServer.h"
#include "Repository/ItemRepository.h"


CItemManager::CItemManager()
{
	Init();
}

CItemManager::~CItemManager()
{

}


void CItemManager::Init()
{
	m_lastItemID = 0;

	smart_ptr<QueryResult> result = g_pItemRepository->GetMaxId();
	if (result)
	{
		Field* f = result->Fetch();

		m_lastItemID = f[0].GetUInt64();
	}

	ERR_LOG(LOG_GENERAL, "Last ITEMID %I64u", m_lastItemID);
}


ITEMID CItemManager::CreateItem(sITEM_DATA& rItemData)
{
	++m_lastItemID;

	g_pItemRepository->Insert(m_lastItemID, rItemData, rItemData.charId);

	return m_lastItemID;
}

ITEMID CItemManager::CreateItem(sSHOP_BUY_INVEN & rData, CHARACTERID charID)
{
	++m_lastItemID;

	g_pItemRepository->Insert(m_lastItemID, rData, charID);

	return m_lastItemID;
}


ITEMID CItemManager::CreateBank(CHARACTERID charID, TBLIDX itemTblidx, BYTE byPlace, BYTE byPos, BYTE byRank, BYTE byDurType, DBOTIME nUseStartTime, DBOTIME nUseEndTime)
{
	++m_lastItemID;

	g_pItemRepository->InsertBank(m_lastItemID, charID, itemTblidx, byPlace, byPos, byRank, byDurType, nUseStartTime, nUseEndTime);

	return m_lastItemID;
}

ITEMID CItemManager::CreateGuildItem(sITEM_DATA & rItemData, GUILDID guildId)
{
	++m_lastItemID;

	g_pItemRepository->InsertGuildItem(m_lastItemID, rItemData, guildId);

	return m_lastItemID;
}

ITEMID CItemManager::CreateSharedBankItem(sITEM_DATA & rItemData, ACCOUNTID accountId)
{
	++m_lastItemID;

	g_pItemRepository->InsertSharedBankItem(m_lastItemID, rItemData, accountId);

	return m_lastItemID;
}
