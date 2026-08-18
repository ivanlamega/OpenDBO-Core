#include "stdafx.h"
#include "CashshopManager.h"
#include "QueryServer.h"
#include "Repository/CashShopRepository.h"


CCashshopManager::CCashshopManager()
{
	Init();
}

CCashshopManager::~CCashshopManager()
{
}


void CCashshopManager::Init()
{
	m_qwLastProductId = 0;

	smart_ptr<QueryResult> result = g_pCashShopRepository->GetMaxProductId();
	if (result)
	{
		Field* f = result->Fetch();

		m_qwLastProductId = f[0].GetUInt64();
	}

	ERR_LOG(LOG_GENERAL, "Last Product-ID %I64u", m_qwLastProductId);
}


QWORD CCashshopManager::AcquireProductId()
{
	return ++m_qwLastProductId;
}