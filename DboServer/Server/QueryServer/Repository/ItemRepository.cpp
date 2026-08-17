#include "stdafx.h"
#include "ItemRepository.h"
#include "QueryServer.h"
#include "PlayerCache.h"


smart_ptr<QueryResult> CItemRepository::GetMaxId()
{
	return GetCharDB.Query("SELECT MAX(id) FROM items");
}

smart_ptr<QueryResult> CItemRepository::GetById(ITEMID itemId)
{
	return GetCharDB.Query("SELECT * FROM items WHERE id=%I64u", itemId);
}

smart_ptr<QueryResult> CItemRepository::GetByIdSingle(ITEMID itemId)
{
	return GetCharDB.Query("SELECT * FROM items WHERE id=%I64u LIMIT 1", itemId);
}

smart_ptr<QueryResult> CItemRepository::GetItemsByGuildId(GUILDID guildId)
{
	return GetCharDB.Query("SELECT * FROM items WHERE GuildID=%u ORDER BY place ASC LIMIT 96", guildId); //NTL_MAX_COUNT_GUILD_HAVE_TOTAL_ITEM
}

void CItemRepository::Insert(ITEMID id, sITEM_DATA& rItemData, CHARACTERID charId)
{
	GetCharDB.Execute("INSERT INTO items (id,tblidx,owner_id,place,pos,count,`rank`,durability,grade,NeedToIdentify,BattleAttribute,Maker,OptionTblidx,OptionTblidx2,OptionRandomId,OptionRandomVal,OptionRandomId2,OptionRandomVal2,OptionRandomId3,OptionRandomVal3,OptionRandomId4,OptionRandomVal4,OptionRandomId5,OptionRandomVal5,OptionRandomId6,OptionRandomVal6,OptionRandomId7,OptionRandomVal7,OptionRandomId8,OptionRandomVal8,UseStartTime,UseEndTime,RestrictState,DurationType)"
		"VALUES(%I64u, %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%ls',%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)",
		id, rItemData.itemNo, charId, rItemData.byPlace, rItemData.byPosition, rItemData.byStackcount, rItemData.byRank, rItemData.byCurrentDurability, rItemData.byGrade, rItemData.bNeedToIdentify, rItemData.byBattleAttribute, rItemData.awchMaker,
		rItemData.sOptionSet.aOptionTblidx[0], rItemData.sOptionSet.aOptionTblidx[1],
		rItemData.sOptionSet.aRandomOption[0].wOptionIndex, rItemData.sOptionSet.aRandomOption[0].optionValue,
		rItemData.sOptionSet.aRandomOption[1].wOptionIndex, rItemData.sOptionSet.aRandomOption[1].optionValue,
		rItemData.sOptionSet.aRandomOption[2].wOptionIndex, rItemData.sOptionSet.aRandomOption[2].optionValue,
		rItemData.sOptionSet.aRandomOption[3].wOptionIndex, rItemData.sOptionSet.aRandomOption[3].optionValue,
		rItemData.sOptionSet.aRandomOption[4].wOptionIndex, rItemData.sOptionSet.aRandomOption[4].optionValue,
		rItemData.sOptionSet.aRandomOption[5].wOptionIndex, rItemData.sOptionSet.aRandomOption[5].optionValue,
		rItemData.sOptionSet.aRandomOption[6].wOptionIndex, rItemData.sOptionSet.aRandomOption[6].optionValue,
		rItemData.sOptionSet.aRandomOption[7].wOptionIndex, rItemData.sOptionSet.aRandomOption[7].optionValue,
		rItemData.nUseStartTime, rItemData.nUseEndTime, rItemData.byRestrictState, rItemData.byDurationType);
}

void CItemRepository::Insert(ITEMID id, sSHOP_BUY_INVEN& rData, CHARACTERID charID)
{
	GetCharDB.Execute("INSERT INTO items (id,tblidx,owner_id,place,pos,count,`rank`,durability,grade,Maker,OptionTblidx,OptionTblidx2,OptionRandomId,OptionRandomVal,OptionRandomId2,OptionRandomVal2,OptionRandomId3,OptionRandomVal3,OptionRandomId4,OptionRandomVal4,OptionRandomId5,OptionRandomVal5,OptionRandomId6,OptionRandomVal6,OptionRandomId7,OptionRandomVal7,OptionRandomId8,OptionRandomVal8,UseStartTime,UseEndTime,RestrictState,DurationType)"
		"VALUES(%I64u, %u,%u,%u,%u,%u,%u,%u,%u,'%ls',%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)",
		id, rData.tblItem, charID, rData.byPlace, rData.byPos, rData.byStack, rData.byRank, rData.byCurrentDurability, rData.byGrade, rData.awchMaker,
		rData.sOptionSet.aOptionTblidx[0], rData.sOptionSet.aOptionTblidx[1],
		rData.sOptionSet.aRandomOption[0].wOptionIndex, rData.sOptionSet.aRandomOption[0].optionValue,
		rData.sOptionSet.aRandomOption[1].wOptionIndex, rData.sOptionSet.aRandomOption[1].optionValue,
		rData.sOptionSet.aRandomOption[2].wOptionIndex, rData.sOptionSet.aRandomOption[2].optionValue,
		rData.sOptionSet.aRandomOption[3].wOptionIndex, rData.sOptionSet.aRandomOption[3].optionValue,
		rData.sOptionSet.aRandomOption[4].wOptionIndex, rData.sOptionSet.aRandomOption[4].optionValue,
		rData.sOptionSet.aRandomOption[5].wOptionIndex, rData.sOptionSet.aRandomOption[5].optionValue,
		rData.sOptionSet.aRandomOption[6].wOptionIndex, rData.sOptionSet.aRandomOption[6].optionValue,
		rData.sOptionSet.aRandomOption[7].wOptionIndex, rData.sOptionSet.aRandomOption[7].optionValue,
		rData.nUseStartTime, rData.nUseEndTime, rData.byRestrictState, rData.byDurationType);
}

void CItemRepository::InsertBank(ITEMID id, CHARACTERID charID, TBLIDX itemTblidx, BYTE byPlace, BYTE byPos, BYTE byRank, BYTE byDurType, DBOTIME nUseStartTime, DBOTIME nUseEndTime)
{
	GetCharDB.Execute("INSERT INTO items (id,tblidx,owner_id,place,pos,`rank`,UseStartTime,UseEndTime,DurationType) VALUES (%I64u, %u,%u,%u,%u,%u,%u,%u,%u)", id, itemTblidx, charID, byPlace, byPos, byRank, nUseStartTime, nUseEndTime, byDurType);
}

void CItemRepository::InsertGuildItem(ITEMID id, sITEM_DATA& rItemData, GUILDID guildId)
{
	GetCharDB.Execute("INSERT INTO items (id,tblidx,place,pos,count,`rank`,durability,grade,NeedToIdentify,BattleAttribute,Maker,OptionTblidx,OptionTblidx2,OptionRandomId,OptionRandomVal,OptionRandomId2,OptionRandomVal2,OptionRandomId3,OptionRandomVal3,OptionRandomId4,OptionRandomVal4,OptionRandomId5,OptionRandomVal5,OptionRandomId6,OptionRandomVal6,OptionRandomId7,OptionRandomVal7,OptionRandomId8,OptionRandomVal8,UseStartTime,UseEndTime,RestrictState,DurationType,GuildID)"
		"VALUES(%I64u, %u,%u,%u,%u,%u,%u,%u,%u,%u,'%ls',%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)",
		id, rItemData.itemNo, rItemData.byPlace, rItemData.byPosition, rItemData.byStackcount, rItemData.byRank, rItemData.byCurrentDurability, rItemData.byGrade, rItemData.bNeedToIdentify, rItemData.byBattleAttribute, rItemData.awchMaker,
		rItemData.sOptionSet.aOptionTblidx[0], rItemData.sOptionSet.aOptionTblidx[1],
		rItemData.sOptionSet.aRandomOption[0].wOptionIndex, rItemData.sOptionSet.aRandomOption[0].optionValue,
		rItemData.sOptionSet.aRandomOption[1].wOptionIndex, rItemData.sOptionSet.aRandomOption[1].optionValue,
		rItemData.sOptionSet.aRandomOption[2].wOptionIndex, rItemData.sOptionSet.aRandomOption[2].optionValue,
		rItemData.sOptionSet.aRandomOption[3].wOptionIndex, rItemData.sOptionSet.aRandomOption[3].optionValue,
		rItemData.sOptionSet.aRandomOption[4].wOptionIndex, rItemData.sOptionSet.aRandomOption[4].optionValue,
		rItemData.sOptionSet.aRandomOption[5].wOptionIndex, rItemData.sOptionSet.aRandomOption[5].optionValue,
		rItemData.sOptionSet.aRandomOption[6].wOptionIndex, rItemData.sOptionSet.aRandomOption[6].optionValue,
		rItemData.sOptionSet.aRandomOption[7].wOptionIndex, rItemData.sOptionSet.aRandomOption[7].optionValue,
		rItemData.nUseStartTime, rItemData.nUseEndTime, rItemData.byRestrictState, rItemData.byDurationType, guildId);
}

void CItemRepository::InsertSharedBankItem(ITEMID id, sITEM_DATA& rItemData, ACCOUNTID accountId)
{
	GetCharDB.Execute("INSERT INTO items (id,tblidx,place,pos,count,`rank`,durability,grade,NeedToIdentify,BattleAttribute,Maker,OptionTblidx,OptionTblidx2,OptionRandomId,OptionRandomVal,OptionRandomId2,OptionRandomVal2,OptionRandomId3,OptionRandomVal3,OptionRandomId4,OptionRandomVal4,OptionRandomId5,OptionRandomVal5,OptionRandomId6,OptionRandomVal6,OptionRandomId7,OptionRandomVal7,OptionRandomId8,OptionRandomVal8,UseStartTime,UseEndTime,RestrictState,DurationType,AccountID)"
		"VALUES(%I64u, %u,%u,%u,%u,%u,%u,%u,%u,%u,'%ls',%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)",
		id, rItemData.itemNo, rItemData.byPlace, rItemData.byPosition, rItemData.byStackcount, rItemData.byRank, rItemData.byCurrentDurability, rItemData.byGrade, rItemData.bNeedToIdentify, rItemData.byBattleAttribute, rItemData.awchMaker,
		rItemData.sOptionSet.aOptionTblidx[0], rItemData.sOptionSet.aOptionTblidx[1],
		rItemData.sOptionSet.aRandomOption[0].wOptionIndex, rItemData.sOptionSet.aRandomOption[0].optionValue,
		rItemData.sOptionSet.aRandomOption[1].wOptionIndex, rItemData.sOptionSet.aRandomOption[1].optionValue,
		rItemData.sOptionSet.aRandomOption[2].wOptionIndex, rItemData.sOptionSet.aRandomOption[2].optionValue,
		rItemData.sOptionSet.aRandomOption[3].wOptionIndex, rItemData.sOptionSet.aRandomOption[3].optionValue,
		rItemData.sOptionSet.aRandomOption[4].wOptionIndex, rItemData.sOptionSet.aRandomOption[4].optionValue,
		rItemData.sOptionSet.aRandomOption[5].wOptionIndex, rItemData.sOptionSet.aRandomOption[5].optionValue,
		rItemData.sOptionSet.aRandomOption[6].wOptionIndex, rItemData.sOptionSet.aRandomOption[6].optionValue,
		rItemData.sOptionSet.aRandomOption[7].wOptionIndex, rItemData.sOptionSet.aRandomOption[7].optionValue,
		rItemData.nUseStartTime, rItemData.nUseEndTime, rItemData.byRestrictState, rItemData.byDurationType, accountId);
}

void CItemRepository::DeleteItemsByOwner(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM items WHERE owner_id=%u", charId);
}

void CItemRepository::DeleteItemsCdByChar(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM items_cd WHERE CharID=%u", charId);
}

void CItemRepository::ClearItemCoolTimes(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM items_cd WHERE CharID = %u", charId);
}

void CItemRepository::InsertItemCd(CHARACTERID charId, BYTE byGroupIndex, DWORD dwInitial, DWORD dwRemaining)
{
	GetCharDB.Execute("INSERT INTO items_cd VALUES(%u, %u, %u, %u)", charId, byGroupIndex, dwInitial, dwRemaining);
}

void CItemRepository::UpdateBattleAttribute(ITEMID itemId, BYTE byBattleAttribute)
{
	GetCharDB.Execute("UPDATE items SET BattleAttribute=%u WHERE id=%I64u", byBattleAttribute, itemId);
}

void CItemRepository::UpdateUseEndTime32(ITEMID itemId, DWORD useEndTime)
{
	GetCharDB.Execute("UPDATE items SET UseEndTime=%u WHERE id=%I64u", useEndTime, itemId);
}

void CItemRepository::UpdateUseEndTime64(ITEMID itemId, DBOTIME useEndTime)
{
	GetCharDB.Execute("UPDATE items SET UseEndTime=%I64u WHERE id=%I64u", useEndTime, itemId);
}

void CItemRepository::UpdateBeadOptions(WORD option7Idx, INT option7Val, WORD option8Idx, INT option8Val, DBOTIME useStartTime, DBOTIME useEndTime, BYTE byRestrictState, BYTE byDurationType, ITEMID itemId)
{
	GetCharDB.Execute("UPDATE items SET OptionRandomId7=%u, OptionRandomVal7=%i, OptionRandomId8=%u, OptionRandomVal8=%i, UseStartTime=%I64u, UseEndTime=%I64u, RestrictState=%u, DurationType=%u WHERE id=%I64u",
		option7Idx, option7Val, option8Idx, option8Val,
		useStartTime, useEndTime, byRestrictState, byDurationType, itemId);
}

void CItemRepository::ClearBeadOptions(WORD invalidIdx1, WORD invalidIdx2, BYTE byRestrictState, BYTE byDurationType, ITEMID itemId)
{
	GetCharDB.Execute("UPDATE items SET OptionRandomId7=%u, OptionRandomVal7=0, OptionRandomId8=%u, OptionRandomVal8=0, UseStartTime=0, UseEndTime=0, RestrictState=%u, DurationType=%u WHERE id=%I64u",
		invalidIdx1, invalidIdx2, byRestrictState, byDurationType, itemId);
}

void CItemRepository::UpdateUseTimeRange(ITEMID itemId, DBOTIME startTime, DBOTIME endTime)
{
	GetCharDB.Execute("UPDATE items SET UseStartTime=%I64u,UseEndTime=%I64u WHERE id=%I64u", startTime, endTime, itemId);
}

void CItemRepository::InsertSharedBankItemMinimal(ITEMID id, TBLIDX itemNo, BYTE byPlace, BYTE byPos, BYTE byRank, DBOTIME useStartTime, DBOTIME useEndTime, BYTE byDurationType, ACCOUNTID accountId)
{
	GetCharDB.Execute("INSERT INTO items (id,tblidx,place,pos,`rank`,UseStartTime,UseEndTime,DurationType,AccountID)"
		"VALUES(%I64u, %u, %u, %u, %u, %I64u, %I64u, %u, %u)",
		id, itemNo, byPlace, byPos, byRank, useStartTime, useEndTime, byDurationType, accountId);
}

void CItemRepository::UpdateOptionSet(TBLIDX opt1, TBLIDX opt2, WORD randId1, INT randVal1, WORD randId2, INT randVal2, WORD randId3, INT randVal3, WORD randId4, INT randVal4, ITEMID itemId)
{
	GetCharDB.Execute("UPDATE items SET OptionTblidx=%u,OptionTblidx2=%u,OptionRandomId=%u,OptionRandomVal=%u,OptionRandomId2=%u,OptionRandomVal2=%u,OptionRandomId3=%u,OptionRandomVal3=%u,OptionRandomId4=%u,OptionRandomVal4=%u WHERE id=%I64u",
		opt1, opt2, randId1, randVal1, randId2, randVal2, randId3, randVal3, randId4, randVal4, itemId);
}

void CItemRepository::UpdateGrade(ITEMID itemId, BYTE byGrade)
{
	GetCharDB.Execute("UPDATE items SET grade=%u WHERE id=%I64u", byGrade, itemId);
}

void CItemRepository::UpdateRestrictState(ITEMID itemId, BYTE byRestrictState)
{
	GetCharDB.Execute("UPDATE items SET RestrictState=%u WHERE id=%I64u", byRestrictState, itemId);
}

void CItemRepository::UpdateGradeAndDuration(BYTE byGrade, DWORD nUseStartTime, DWORD nUseEndTime, BYTE byRestrictState, BYTE byDurationType, ITEMID itemId)
{
	GetCharDB.Execute("UPDATE items SET grade=%u, UseStartTime=%u, UseEndTime=%u, RestrictState=%u, DurationType=%u WHERE id=%I64u", byGrade, nUseStartTime, nUseEndTime, byRestrictState, byDurationType, itemId);
}

void CItemRepository::ResetGradeAndDuration(BYTE byRestrictState, ITEMID itemId)
{
	GetCharDB.Execute("UPDATE items SET grade=0, UseStartTime=0, UseEndTime=0, RestrictState=%u, DurationType=0 WHERE id=%I64u", byRestrictState, itemId);
}

void CItemRepository::DeleteById(ITEMID itemId)
{
	GetCharDB.Execute("DELETE FROM items WHERE id=%I64u", itemId);
}

void CItemRepository::UpdateCount(ITEMID itemId, BYTE byCount)
{
	GetCharDB.Execute("UPDATE items SET count=%u WHERE id=%I64u", byCount, itemId);
}

void CItemRepository::ClearOwner(ITEMID itemId)
{
	GetCharDB.Execute("UPDATE items SET owner_id=0 WHERE id=%I64u", itemId);
}

void CItemRepository::UpdatePlace(ITEMID itemId, BYTE byPlace, BYTE byPos)
{
	GetCharDB.Execute("UPDATE items SET place=%u, pos=%u WHERE id=%I64u", byPlace, byPos, itemId);
}

void CItemRepository::UpdatePlaceWithRestrict(ITEMID itemId, BYTE byPlace, BYTE byPos, BYTE byRestrictState)
{
	GetCharDB.Execute("UPDATE items SET place=%u, pos=%u, RestrictState=%u WHERE id=%I64u", byPlace, byPos, byRestrictState, itemId);
}

void CItemRepository::MoveToPersonalBank(ITEMID itemId, CHARACTERID ownerId, BYTE byPlace, BYTE byPos)
{
	GetCharDB.Execute("UPDATE items SET owner_id=%u, place=%u, pos=%u, AccountID=0 WHERE id=%I64u", ownerId, byPlace, byPos, itemId);
}

void CItemRepository::MoveToSharedBank(ITEMID itemId, BYTE byPlace, BYTE byPos, ACCOUNTID accountId)
{
	GetCharDB.Execute("UPDATE items SET owner_id=0, place=%u, pos=%u, AccountID=%u WHERE id=%I64u", byPlace, byPos, accountId, itemId);
}

void CItemRepository::UpdateOwnerPlace(ITEMID itemId, CHARACTERID ownerId, BYTE byPlace, BYTE byPos)
{
	GetCharDB.Execute("UPDATE items SET owner_id=%u, place=%u, pos=%u WHERE id=%I64u", ownerId, byPlace, byPos, itemId);
}

void CItemRepository::UpdateOwnerPlacePos(ITEMID itemId, CHARACTERID ownerId, BYTE byPlace, BYTE byPos)
{
	GetCharDB.Execute("UPDATE items SET owner_id=%u,place=%u,pos=%u WHERE id=%I64u", ownerId, byPlace, byPos, itemId);
}

void CItemRepository::MoveToGuildBank(ITEMID itemId, BYTE byPlace, BYTE byPos, GUILDID guildId)
{
	GetCharDB.Execute("UPDATE items SET owner_id=0, place=%u, pos=%u, GuildID=%u WHERE id=%I64u", byPlace, byPos, guildId, itemId);
}

void CItemRepository::MoveFromGuildBankToOwner(ITEMID itemId, CHARACTERID ownerId, BYTE byPlace, BYTE byPos)
{
	GetCharDB.Execute("UPDATE items SET owner_id=%u, place=%u, pos=%u, GuildID=0 WHERE id=%I64u", ownerId, byPlace, byPos, itemId);
}

void CItemRepository::UpdateRankGrade(ITEMID itemId, BYTE byRank, BYTE byGrade)
{
	GetCharDB.Execute("UPDATE items SET `rank`=%u, grade=%u WHERE id=%I64u", byRank, byGrade, itemId);
}

void CItemRepository::UpdateDurability(ITEMID itemId, BYTE byDurability)
{
	GetCharDB.Execute("UPDATE items SET durability=%u WHERE id=%I64u", byDurability, itemId);
}

void CItemRepository::UpdateDurabilityByOwnerPlacePos(BYTE byDurability, CHARACTERID charId, BYTE byPlace, BYTE byPos)
{
	GetCharDB.Execute("UPDATE items SET durability=%u WHERE owner_id=%u AND place=%u AND pos=%u", byDurability, charId, byPlace, byPos);
}

void CItemRepository::ClearNeedToIdentify(ITEMID itemId)
{
	GetCharDB.Execute("UPDATE items SET NeedToIdentify=false WHERE id=%I64u", itemId);
}

void CItemRepository::InsertSplitFromSource(ITEMID newItemId, BYTE byPlace, BYTE byPos, BYTE byCount, ITEMID sourceItemId)
{
	GetCharDB.Execute("INSERT INTO items (id,tblidx,owner_id,place,pos,count,`rank`,Maker,RestrictState) SELECT %I64u,tblidx,owner_id,%u,%u,%u,`rank`,Maker,RestrictState FROM items WHERE id=%I64u",
		newItemId, byPlace, byPos, byCount, sourceItemId);
}

void CItemRepository::LoadBankDataAsync(CPlayerCache* pCache, CAccountCache* pAccount, bool bAccountBankAlreadyLoaded, HOBJECT handle, HOBJECT npcHandle, CHARACTERID charId, ACCOUNTID accountId)
{
	SQLCallbackBase* pCallBack = new SQLClassCallbackP4<CPlayerCache, CAccountCache*, bool, HOBJECT, HOBJECT>(pCache, &CPlayerCache::OnLoadBank, pAccount, bAccountBankAlreadyLoaded, handle, npcHandle);
	AsyncQuery * q = new AsyncQuery(pCallBack);
	q->AddQuery("SELECT * FROM items WHERE owner_id=%u AND (place >= 7 AND place <=10) ORDER BY place ASC LIMIT 68", charId); // 7 = CONTAINER_TYPE_BANKSLOT
	if (!bAccountBankAlreadyLoaded)
		q->AddQuery("SELECT * FROM items WHERE AccountID=%u ORDER BY place ASC LIMIT 68", accountId);
	GetCharDB.QueueAsyncQuery(q);
}
