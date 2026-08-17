#include "stdafx.h"
#include "AuditLogRepository.h"
#include "QueryServer.h"


void CAuditLogRepository::InsertCharacterDeleteLog(ACCOUNTID accountId, CHARACTERID charId)
{
	GetLogDB.Execute("INSERT INTO character_delete_log (AccountID, CharID) VALUES (%u, %u)", accountId, charId);
}

void CAuditLogRepository::InsertMuteLog(CHARACTERID charId, ACCOUNTID gmAccountId, DWORD durationInMinutes, WCHAR* wszReason, DBOTIME muteUntil)
{
	GetLogDB.Execute("INSERT INTO mute_log (CharID,GmAccountID,DurationInMinutes,Reason,muteUntil) values (%u, %u, %u, \"%ls\", %I64u)",
		charId, gmAccountId, durationInMinutes, wszReason, muteUntil);
}

void CAuditLogRepository::DeleteMuteLog(CHARACTERID charId)
{
	GetLogDB.Execute("DELETE FROM mute_log WHERE CharID=%u", charId);
}

void CAuditLogRepository::InsertSlotMachineLog(ACCOUNTID accountId, CHARACTERID charId, BYTE extractCount, BYTE machineType, WORD coin, DWORD currentPoints, DWORD newPoints, QWORD productId1, QWORD productId2, QWORD productId3, QWORD productId4, QWORD productId5, QWORD productId6, QWORD productId7, QWORD productId8, QWORD productId9, QWORD productId10)
{
	GetLogDB.Execute("INSERT INTO slot_machine_log (accountid,charid,extractCount,type,coin,currentPoints,newPoints,ProductId1,ProductId2,ProductId3,ProductId4,ProductId5,ProductId6,ProductId7,ProductId8,ProductId9,ProductId10)VALUES(%u,%u,%u,%u,%u,%u,%u,%I64u,%I64u,%I64u,%I64u,%I64u,%I64u,%I64u,%I64u,%I64u,%I64u)",
		accountId, charId, extractCount, machineType, coin, currentPoints, newPoints, productId1, productId2, productId3, productId4, productId5, productId6, productId7, productId8, productId9, productId10);
}

void CAuditLogRepository::InsertAccountBanLog(ACCOUNTID gmAccountId, ACCOUNTID targetAccountId, const char* reason, DWORD byDuration)
{
	GetAccDB.Execute("INSERT INTO accounts_banned(GM_AccId, Banned_AccId, Reason, Duration) VALUES (%u, %u, \"%s\", %u)", gmAccountId, targetAccountId, reason, byDuration);
}

void CAuditLogRepository::InsertCharNameChangeLog(CHARACTERID charId, WCHAR* wszOldName, WCHAR* wszNewName)
{
	GetLogDB.Execute("INSERT INTO change_char_name (CharID, Name, newName) VALUES(%u, \"%ls\", \"%ls\")", charId, wszOldName, wszNewName);
}

void CAuditLogRepository::InsertItemUpgradeLog(CHARACTERID charId, int bIsSuccessful, ITEMID itemId, TBLIDX itemTblidx, BYTE byCurGrade, BYTE byNewGrade, ITEMID stoneId, TBLIDX stoneTblidx, int bCoreItemUse, ITEMID coreId, TBLIDX coreItemIdx)
{
	GetLogDB.Execute("INSERT INTO item_upgrade_log (charId, IsSuccess, itemId, itemTblidx, grade, newGrade, StoneItemId, StoneItemTblidx, CoreItemUse, coreItemId, coreItemTblidx) VALUES (%u, %i, %I64u, %u, %u, %u, %I64u, %u, %i, %I64u, %u)",
		charId, bIsSuccessful, itemId, itemTblidx, byCurGrade, byNewGrade, stoneId, stoneTblidx, bCoreItemUse, coreId, coreItemIdx);
}

void CAuditLogRepository::InsertGmLog(CHARACTERID charId, BYTE byLogType, const char* message)
{
	GetLogDB.Execute("INSERT INTO gm_log (CharID, LogType, String) VALUES (%u,%u,'%s')", charId, byLogType, message);
}

void CAuditLogRepository::InsertMailDeletedLog(DWORD mailId, CHARACTERID charId, BYTE bySenderType, BYTE byMailType, BYTE byTextSize, const char* text, DWORD zenny, ITEMID itemId, WCHAR* wszFromName,
	int bIsAccept, int bIsLock, int bIsRead, DBOTIME endTime, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second)
{
	GetLogDB.Execute("INSERT INTO mail_deleted (id,CharID,SenderType,MailType,TextSize,Text,Zenny,itemId,FromName,IsAccept,IsLock,IsRead,EndTime, year, month, day, hour, minute, second)"
		"VALUES(%u,%u,%u,%u,%u,\"%s\",%u, %I64u,\"%ls\",%i,%i,%i,%I64u,%u,%u,%u,%u,%u,%u)",
		mailId, charId, bySenderType, byMailType, byTextSize, text, zenny, itemId, wszFromName, bIsAccept, bIsLock, bIsRead, endTime
		, year, month, day, hour, minute, second);
}

void CAuditLogRepository::InsertTradeLog(CHARACTERID charId, CHARACTERID targetCharId, DWORD zeni, BYTE itemCount,
	ITEMID itemId1, TBLIDX itemTblidx1, ITEMID itemId2, TBLIDX itemTblidx2, ITEMID itemId3, TBLIDX itemTblidx3,
	ITEMID itemId4, TBLIDX itemTblidx4, ITEMID itemId5, TBLIDX itemTblidx5, ITEMID itemId6, TBLIDX itemTblidx6,
	ITEMID itemId7, TBLIDX itemTblidx7, ITEMID itemId8, TBLIDX itemTblidx8, ITEMID itemId9, TBLIDX itemTblidx9,
	ITEMID itemId10, TBLIDX itemTblidx10, ITEMID itemId11, TBLIDX itemTblidx11, ITEMID itemId12, TBLIDX itemTblidx12)
{
	GetLogDB.Execute("INSERT INTO TradeLogs (CharID, TargetCharID, Zeni, ItemCount, ItemID_1,ItemTblidx_1, ItemID_2,ItemTblidx_2, ItemID_3,ItemTblidx_3, ItemID_4,ItemTblidx_4, ItemID_5,ItemTblidx_5, ItemID_6,ItemTblidx_6"
		", ItemID_7,ItemTblidx_7, ItemID_8,ItemTblidx_8, ItemID_9,ItemTblidx_9, ItemID_10,ItemTblidx_10, ItemID_11, ItemTblidx_11, ItemID_12,ItemTblidx_12)"
		"VALUES(%u,%u,%u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u)",
		charId, targetCharId, zeni, itemCount,
		itemId1, itemTblidx1, itemId2, itemTblidx2, itemId3, itemTblidx3,
		itemId4, itemTblidx4, itemId5, itemTblidx5, itemId6, itemTblidx6,
		itemId7, itemTblidx7, itemId8, itemTblidx8, itemId9, itemTblidx9,
		itemId10, itemTblidx10, itemId11, itemTblidx11, itemId12, itemTblidx12
	);
}

void CAuditLogRepository::InsertPrivateShopLog(CHARACTERID sellerCharId, CHARACTERID buyerCharId, DWORD zeni, BYTE itemCount,
	ITEMID itemId1, TBLIDX itemTblidx1, ITEMID itemId2, TBLIDX itemTblidx2, ITEMID itemId3, TBLIDX itemTblidx3,
	ITEMID itemId4, TBLIDX itemTblidx4, ITEMID itemId5, TBLIDX itemTblidx5, ITEMID itemId6, TBLIDX itemTblidx6,
	ITEMID itemId7, TBLIDX itemTblidx7, ITEMID itemId8, TBLIDX itemTblidx8, ITEMID itemId9, TBLIDX itemTblidx9,
	ITEMID itemId10, TBLIDX itemTblidx10, ITEMID itemId11, TBLIDX itemTblidx11, ITEMID itemId12, TBLIDX itemTblidx12,
	BOOL bHasIssues, const char* issueReason)
{
	GetLogDB.Execute("INSERT INTO PrivateShopLogs (SellerCharID, BuyerCharID, Zeni, ItemCount, ItemID_1,ItemTblidx_1, ItemID_2,ItemTblidx_2, ItemID_3,ItemTblidx_3, ItemID_4,ItemTblidx_4, ItemID_5,ItemTblidx_5, ItemID_6,ItemTblidx_6"
		", ItemID_7,ItemTblidx_7, ItemID_8,ItemTblidx_8, ItemID_9,ItemTblidx_9, ItemID_10,ItemTblidx_10, ItemID_11, ItemTblidx_11, ItemID_12,ItemTblidx_12, HasIssues,IssueReason)"
		"VALUES(%u,%u,%u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%u,'%s')",
		sellerCharId, buyerCharId, zeni, itemCount,
		itemId1, itemTblidx1, itemId2, itemTblidx2, itemId3, itemTblidx3,
		itemId4, itemTblidx4, itemId5, itemTblidx5, itemId6, itemTblidx6,
		itemId7, itemTblidx7, itemId8, itemTblidx8, itemId9, itemTblidx9,
		itemId10, itemTblidx10, itemId11, itemTblidx11, itemId12, itemTblidx12,
		bHasIssues, issueReason);
}
