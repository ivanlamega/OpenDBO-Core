#include "stdafx.h"
#include "HlsSlotMachine.h"
#include "TableContainerManager.h"
#include "HLSItemTable.h"
#include "SlotMachineTable.h"
#include "NtlRandom.h"
#include "Player.h"
#include <time.h>
#include "NtlPacketTU.h"


CHlsSlotMachine::CHlsSlotMachine()
{
	Init();
}

CHlsSlotMachine::~CHlsSlotMachine()
{
}


void CHlsSlotMachine::Init()
{
	//delete items
	for (SLOTMACHINEGROUP::iterator it = m_slotMachineGroup.begin(); it != m_slotMachineGroup.end();)
	{
		delete it->second;
		it = m_slotMachineGroup.erase(it);
	}

	//delete slot machine
	for (std::map<TBLIDX, sSLOT_MACHINE*>::iterator it = m_mapSlotMachine.begin(); it != m_mapSlotMachine.end();)
	{
		delete it->second;
		it = m_mapSlotMachine.erase(it);
	}


	CHlsSlotMachineItemTable* pItemTable = g_pTableContainer->GetSlotMachineItemTable();
	for (CTable::TABLEIT it = pItemTable->Begin(); it != pItemTable->End(); it++)
	{
		sHLS_SLOT_MACHINE_ITEM_TBLDAT* pHlsSlotItem = (sHLS_SLOT_MACHINE_ITEM_TBLDAT*)it->second;

		if (pHlsSlotItem->bActive == false)
			continue;

		sHLS_ITEM_TBLDAT* pHlsItem = (sHLS_ITEM_TBLDAT*)g_pTableContainer->GetHLSItemTable()->FindData(pHlsSlotItem->cashItemTblidx);
		if(pHlsItem)
		{
			sHLS_SLOT_ITEM* pSlot = new sHLS_SLOT_ITEM;

			pSlot->wCountLeft = (pHlsSlotItem->byStackCount > 0) ? pHlsSlotItem->byStackCount : INVALID_WORD;
			pSlot->byRank = 0;
			pSlot->fPercent = (float)pHlsSlotItem->byPercent;
			pSlot->pHlsItem = pHlsItem;
			pSlot->pSlotItem = pHlsSlotItem;

			m_slotMachineGroup.insert(SLOTMACHINEGROUP_VAL(pHlsSlotItem->slotMachineTblidx, pSlot));
		}
	}

	//loop the slots to get the top 10 items
	CSlotMachineTable* pSlotTable = g_pTableContainer->GetSlotMachineTable();
	for (CTable::TABLEIT it = pSlotTable->Begin(); it != pSlotTable->End(); it++)
	{
		sHLS_SLOT_MACHINE_TBLDAT* pHlsMachineTbldat = (sHLS_SLOT_MACHINE_TBLDAT*)it->second;

		if (pHlsMachineTbldat->bOnOff == false)
			continue;

		//insert slot machine
		sSLOT_MACHINE* pSlotMachine = new sSLOT_MACHINE;
		pSlotMachine->pTbldat = pHlsMachineTbldat;

		for (BYTE i = 0; i < DBO_MAX_HLS_SLOT_MACHINES_MAX_ITEMS; i++)
		{
			if (pHlsMachineTbldat->aItemTblidx[i] == INVALID_TBLIDX)
				break;

			sHLS_ITEM_TBLDAT* pHlsItem = (sHLS_ITEM_TBLDAT*)g_pTableContainer->GetHLSItemTable()->FindData(pHlsMachineTbldat->aItemTblidx[i]);
			if (pHlsItem)
			{
				sHLS_SLOT_ITEM* pSlot = new sHLS_SLOT_ITEM;

				pSlot->wCountLeft = pHlsMachineTbldat->wQuantity[i];
				pSlot->byRank = i + 1;
				pSlot->fPercent = 0.0f;
				pSlot->pHlsItem = pHlsItem;
				pSlot->pSlotItem = NULL;

				m_slotMachineGroup.insert(SLOTMACHINEGROUP_VAL(pHlsMachineTbldat->tblidx, pSlot));
			}
			else printf("pHlsMachineTbldat->aItemTblidx[i] %u not found in table_hls_item_data\n", pHlsMachineTbldat->aItemTblidx[i]);
		}

		// capsule totals are the sum of every item actually assigned to this machine
		// (both the type-0 percent pool and the rank items just inserted above)
		SLOTMACHINEGROUP_IT itLow = m_slotMachineGroup.lower_bound(pHlsMachineTbldat->tblidx);
		SLOTMACHINEGROUP_IT itUp = m_slotMachineGroup.upper_bound(pHlsMachineTbldat->tblidx);
		int nCapsuleTotal = 0;
		while (itLow != itUp)
		{
			nCapsuleTotal += itLow->second->wCountLeft;
			++itLow;
		}

		pSlotMachine->wMaxCapsule = (WORD)nCapsuleTotal;
		pSlotMachine->wCurrentCapsule = (WORD)nCapsuleTotal;
		m_mapSlotMachine.insert({ pHlsMachineTbldat->tblidx, pSlotMachine });
	}
}


void CHlsSlotMachine::GetSlotItems(TBLIDX slotIdx, std::vector<sHLS_SLOT_ITEM*>* pVec)
{
	sSLOT_MACHINE* pSlot = GetSlotMachine(slotIdx);
	if (pSlot == NULL)
		return;

	std::multimap<TBLIDX, sHLS_SLOT_ITEM*>::iterator itLow = m_slotMachineGroup.lower_bound(slotIdx);
	std::multimap<TBLIDX, sHLS_SLOT_ITEM*>::iterator itUp = m_slotMachineGroup.upper_bound(slotIdx);

	while (itLow != itUp)
	{
		sHLS_SLOT_ITEM* pTbldat = itLow->second;

		if (pTbldat->wCountLeft > 0)
		{
			pTbldat->fPercent = (float)pTbldat->wCountLeft / (float)pSlot->wCurrentCapsule;
			pVec->push_back(pTbldat);
		}

		++itLow;
	}
}

void CHlsSlotMachine::SetWaguItemCount(TBLIDX slotIdx, BYTE Count, TBLIDX tblidx)
{
	std::multimap<TBLIDX, sHLS_SLOT_ITEM*>::iterator itLow = m_slotMachineGroup.lower_bound(slotIdx);
	std::multimap<TBLIDX, sHLS_SLOT_ITEM*>::iterator itUp = m_slotMachineGroup.upper_bound(slotIdx);

	while (itLow != itUp)
	{
		sHLS_SLOT_ITEM* pTbldat = itLow->second;
		if (pTbldat->pHlsItem->tblidx == tblidx)
		{
			pTbldat->wCountLeft -= Count;
		}

		++itLow;
	}
}

void CHlsSlotMachine::AddWinner(TBLIDX slotId, TBLIDX itemTblidx, CPlayer * pPlayer)
{
	QWORD& winnerIndex = m_mapWinnerIndex[slotId];
	winnerIndex += 1;

	sHLS_SLOT_WINNER_INFO* pWinner = new sHLS_SLOT_WINNER_INFO;
	pWinner->nExtractTime = time(NULL);
	NTL_SAFE_WCSCPY(pWinner->wszPlayer, pPlayer->GetCharName());
	pWinner->wWinCount = pPlayer->GetSlotMachineCount();
	pWinner->winnerIndex = winnerIndex;

	std::list<sHLS_SLOT_WINNER_INFO*>& winnerList = m_mapSlotWinnerInfo[slotId];
	winnerList.push_back(pWinner);

	if (winnerList.size() > 3)
	{
		sHLS_SLOT_WINNER_INFO* pInfo = winnerList.front();
		SAFE_DELETE(pInfo);
		winnerList.pop_front();
	}
}

void CHlsSlotMachine::GetWinnerInfo(TBLIDX wSlot, CPlayer * pPlayer)
{
	CNtlPacket packet(sizeof(sTU_HLS_SLOT_MACHINE_WINNER_INFO_RES));
	sTU_HLS_SLOT_MACHINE_WINNER_INFO_RES* res = (sTU_HLS_SLOT_MACHINE_WINNER_INFO_RES*)packet.GetPacketData();
	res->wOpCode = TU_HLS_SLOT_MACHINE_WINNER_INFO_RES;
	res->wResultCode = CHAT_SUCCESS;
	res->wMachineIndex = wSlot;
	res->byInfoCount = 0;

	std::map<TBLIDX, std::list<sHLS_SLOT_WINNER_INFO*>>::iterator itWinnerList = m_mapSlotWinnerInfo.find(wSlot);
	if (itWinnerList != m_mapSlotWinnerInfo.end())
	{
		for (std::list<sHLS_SLOT_WINNER_INFO*>::iterator it = itWinnerList->second.begin(); it != itWinnerList->second.end(); it++)
		{
			sHLS_SLOT_WINNER_INFO* pInfo = *it;

			NTL_SAFE_WCSCPY(res->wszPlayer[res->byInfoCount], pInfo->wszPlayer);
			res->wWinCount[res->byInfoCount] = pInfo->wWinCount;
			res->nExtractTime[res->byInfoCount] = pInfo->nExtractTime;
			res->nWinnerIndex[res->byInfoCount] = (WORD)pInfo->winnerIndex;

			if (++res->byInfoCount == DBO_MAX_HLS_SLOT_MACHINES_MAX_WINNERS)
				break;
		}
	}

	packet.SetPacketLen(sizeof(sTU_HLS_SLOT_MACHINE_WINNER_INFO_RES));
	pPlayer->SendPacket(&packet);
}

void CHlsSlotMachine::LoadSlotMachines(CPlayer* pPlayer, BYTE byType)
{
	BYTE i = 0;

	CNtlPacket packet(sizeof(sTU_HLS_SLOT_MACHINE_INFO_RES));
	sTU_HLS_SLOT_MACHINE_INFO_RES* res = (sTU_HLS_SLOT_MACHINE_INFO_RES*)packet.GetPacketData();
	res->wOpCode = TU_HLS_SLOT_MACHINE_INFO_RES;
	res->wResultCode = CHAT_SUCCESS;
	res->byType = byType;

	for (std::map<TBLIDX, sSLOT_MACHINE*>::iterator it = m_mapSlotMachine.begin(); it != m_mapSlotMachine.end(); it++)
	{
		sSLOT_MACHINE* pSlotMachine = (sSLOT_MACHINE*)it->second;

		if (pSlotMachine->pTbldat->byType == byType)
		{
			res->wMachineIndex[i] = pSlotMachine->pTbldat->tblidx;
			res->byCoin[i] = (BYTE)pSlotMachine->pTbldat->byCoin;
			res->bOnOff[i] = pSlotMachine->pTbldat->bOnOff;
			memcpy(res->ItemTblidx[i], pSlotMachine->pTbldat->aItemTblidx, sizeof(pSlotMachine->pTbldat->aItemTblidx));
			res->wWaitingTime[i] = 0;
			res->wCurrentCapsule[i] = pSlotMachine->wCurrentCapsule;
			res->wMaxCapsule[i] = pSlotMachine->wMaxCapsule;

			i++;
		}
	}

	res->byMachineCount = i;
	packet.SetPacketLen(sizeof(sTU_HLS_SLOT_MACHINE_INFO_RES));
	pPlayer->SendPacket(&packet);
}

sSLOT_MACHINE * CHlsSlotMachine::GetSlotMachine(TBLIDX tblidx)
{
	std::map<TBLIDX, sSLOT_MACHINE*>::iterator it = m_mapSlotMachine.find(tblidx);
	if (it != m_mapSlotMachine.end())
		return it->second;

	return nullptr;
}

void CHlsSlotMachine::DebugDumpSlotMachines(TBLIDX requestedIdx)
{
	printf("[HlsSlotMachine] machine index %u not found. Loaded machines (tblidx/type/bOnOff):", requestedIdx);
	for (std::map<TBLIDX, sSLOT_MACHINE*>::iterator it = m_mapSlotMachine.begin(); it != m_mapSlotMachine.end(); it++)
	{
		sSLOT_MACHINE* pSlotMachine = it->second;
		printf(" [%u/%u/%u]", it->first, pSlotMachine->pTbldat->byType, pSlotMachine->pTbldat->bOnOff);
	}
	printf("\n");
}
