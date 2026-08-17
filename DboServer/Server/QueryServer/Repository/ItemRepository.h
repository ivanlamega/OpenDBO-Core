#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "ItemTable.h"
#include "NtlSharedType.h"

class CPlayerCache;
class CAccountCache;

class CItemRepository : public CNtlSingleton<CItemRepository>
{

public:

	CItemRepository() {}
	virtual ~CItemRepository() {}

public:

	smart_ptr<QueryResult>		GetMaxId();
	smart_ptr<QueryResult>		GetById(ITEMID itemId);
	smart_ptr<QueryResult>		GetByIdSingle(ITEMID itemId);
	smart_ptr<QueryResult>		GetItemsByGuildId(GUILDID guildId);

	void						Insert(ITEMID id, sITEM_DATA& rItemData, CHARACTERID charId);
	void						Insert(ITEMID id, sSHOP_BUY_INVEN& rData, CHARACTERID charID);
	void						InsertBank(ITEMID id, CHARACTERID charID, TBLIDX itemTblidx, BYTE byPlace, BYTE byPos, BYTE byRank, BYTE byDurType, DBOTIME nUseStartTime, DBOTIME nUseEndTime);
	void						InsertGuildItem(ITEMID id, sITEM_DATA& rItemData, GUILDID guildId);
	void						InsertSharedBankItem(ITEMID id, sITEM_DATA& rItemData, ACCOUNTID accountId);

	void						DeleteItemsByOwner(CHARACTERID charId);
	void						DeleteItemsCdByChar(CHARACTERID charId);
	void						ClearItemCoolTimes(CHARACTERID charId);
	void						InsertItemCd(CHARACTERID charId, BYTE byGroupIndex, DWORD dwInitial, DWORD dwRemaining);
	void						UpdateBattleAttribute(ITEMID itemId, BYTE byBattleAttribute);
	void						UpdateUseEndTime32(ITEMID itemId, DWORD useEndTime);
	void						UpdateUseEndTime64(ITEMID itemId, DBOTIME useEndTime);
	void						UpdateBeadOptions(WORD option7Idx, INT option7Val, WORD option8Idx, INT option8Val, DBOTIME useStartTime, DBOTIME useEndTime, BYTE byRestrictState, BYTE byDurationType, ITEMID itemId);
	void						ClearBeadOptions(WORD invalidIdx1, WORD invalidIdx2, BYTE byRestrictState, BYTE byDurationType, ITEMID itemId);
	void						UpdateUseTimeRange(ITEMID itemId, DBOTIME startTime, DBOTIME endTime);
	void						InsertSharedBankItemMinimal(ITEMID id, TBLIDX itemNo, BYTE byPlace, BYTE byPos, BYTE byRank, DBOTIME useStartTime, DBOTIME useEndTime, BYTE byDurationType, ACCOUNTID accountId);
	void						UpdateOptionSet(TBLIDX opt1, TBLIDX opt2, WORD randId1, INT randVal1, WORD randId2, INT randVal2, WORD randId3, INT randVal3, WORD randId4, INT randVal4, ITEMID itemId);
	void						UpdateGrade(ITEMID itemId, BYTE byGrade);
	void						UpdateRestrictState(ITEMID itemId, BYTE byRestrictState);
	void						UpdateGradeAndDuration(BYTE byGrade, DWORD nUseStartTime, DWORD nUseEndTime, BYTE byRestrictState, BYTE byDurationType, ITEMID itemId);
	void						ResetGradeAndDuration(BYTE byRestrictState, ITEMID itemId);
	void						DeleteById(ITEMID itemId);
	void						UpdateCount(ITEMID itemId, BYTE byCount);
	void						ClearOwner(ITEMID itemId);

	void						UpdatePlace(ITEMID itemId, BYTE byPlace, BYTE byPos);
	void						UpdatePlaceWithRestrict(ITEMID itemId, BYTE byPlace, BYTE byPos, BYTE byRestrictState);
	void						MoveToPersonalBank(ITEMID itemId, CHARACTERID ownerId, BYTE byPlace, BYTE byPos);
	void						MoveToSharedBank(ITEMID itemId, BYTE byPlace, BYTE byPos, ACCOUNTID accountId);
	void						UpdateOwnerPlace(ITEMID itemId, CHARACTERID ownerId, BYTE byPlace, BYTE byPos);
	void						UpdateOwnerPlacePos(ITEMID itemId, CHARACTERID ownerId, BYTE byPlace, BYTE byPos);
	void						MoveToGuildBank(ITEMID itemId, BYTE byPlace, BYTE byPos, GUILDID guildId);
	void						MoveFromGuildBankToOwner(ITEMID itemId, CHARACTERID ownerId, BYTE byPlace, BYTE byPos);
	void						UpdateRankGrade(ITEMID itemId, BYTE byRank, BYTE byGrade);
	void						UpdateDurability(ITEMID itemId, BYTE byDurability);
	void						UpdateDurabilityByOwnerPlacePos(BYTE byDurability, CHARACTERID charId, BYTE byPlace, BYTE byPos);
	void						ClearNeedToIdentify(ITEMID itemId);
	void						InsertSplitFromSource(ITEMID newItemId, BYTE byPlace, BYTE byPos, BYTE byCount, ITEMID sourceItemId);

	void						LoadBankDataAsync(CPlayerCache* pCache, CAccountCache* pAccount, bool bAccountBankAlreadyLoaded, HOBJECT handle, HOBJECT npcHandle, CHARACTERID charId, ACCOUNTID accountId);

};

#define GetItemRepository()		CItemRepository::GetInstance()
#define g_pItemRepository			GetItemRepository()
