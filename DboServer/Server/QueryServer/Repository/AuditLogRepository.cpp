#include "stdafx.h"
#include "AuditLogRepository.h"
#include "../QueryServer.h"


void CAuditLogRepository::InsertCharacterDeleteLog(ACCOUNTID accountId, CHARACTERID charId)
{
	GetLogDB.Execute("INSERT INTO character_delete_log (account_id, char_id) VALUES (%u, %u)", accountId, charId);
}

void CAuditLogRepository::InsertMuteLog(CHARACTERID charId, ACCOUNTID gmAccountId, DWORD durationInMinutes, WCHAR* wszReason, DBOTIME muteUntil)
{
	GetLogDB.Execute("INSERT INTO mute_log (char_id,gm_account_id,duration_in_minutes,reason,mute_until) values (%u, %u, %u, \"%ls\", %I64u)",
		charId, gmAccountId, durationInMinutes, wszReason, muteUntil);
}

void CAuditLogRepository::DeleteMuteLog(CHARACTERID charId)
{
	GetLogDB.Execute("DELETE FROM mute_log WHERE char_id=%u", charId);
}

void CAuditLogRepository::InsertSlotMachineLog(ACCOUNTID accountId, CHARACTERID charId, BYTE extractCount, BYTE machineType, WORD coin, DWORD currentPoints, DWORD newPoints, QWORD productId1, QWORD productId2, QWORD productId3, QWORD productId4, QWORD productId5, QWORD productId6, QWORD productId7, QWORD productId8, QWORD productId9, QWORD productId10)
{
	GetLogDB.Execute("INSERT INTO slot_machine_log (account_id,char_id,extract_count,type,coin,current_points,new_points,product_id_1,product_id_2,product_id_3,product_id_4,product_id_5,product_id_6,product_id_7,product_id_8,product_id_9,product_id_10)VALUES(%u,%u,%u,%u,%u,%u,%u,%I64u,%I64u,%I64u,%I64u,%I64u,%I64u,%I64u,%I64u,%I64u,%I64u)",
		accountId, charId, extractCount, machineType, coin, currentPoints, newPoints, productId1, productId2, productId3, productId4, productId5, productId6, productId7, productId8, productId9, productId10);
}

void CAuditLogRepository::InsertAccountBanLog(ACCOUNTID gmAccountId, ACCOUNTID targetAccountId, const char* reason, DWORD byDuration)
{
	GetAccDB.Execute("INSERT INTO accounts_banned(gm_account_id, banned_account_id, reason, duration) VALUES (%u, %u, \"%s\", %u)", gmAccountId, targetAccountId, reason, byDuration);
}

void CAuditLogRepository::InsertCharNameChangeLog(CHARACTERID charId, WCHAR* wszOldName, WCHAR* wszNewName)
{
	GetLogDB.Execute("INSERT INTO change_char_name (char_id, name, new_name) VALUES(%u, \"%ls\", \"%ls\")", charId, wszOldName, wszNewName);
}

void CAuditLogRepository::InsertItemUpgradeLog(CHARACTERID charId, int bIsSuccessful, ITEMID itemId, TBLIDX itemTblidx, BYTE byCurGrade, BYTE byNewGrade, ITEMID stoneId, TBLIDX stoneTblidx, int bCoreItemUse, ITEMID coreId, TBLIDX coreItemIdx)
{
	GetLogDB.Execute("INSERT INTO item_upgrade_log (char_id, is_success, item_id, item_tblidx, grade, new_grade, stone_item_id, stone_item_tblidx, core_item_use, core_item_id, core_item_tblidx) VALUES (%u, %i, %I64u, %u, %u, %u, %I64u, %u, %i, %I64u, %u)",
		charId, bIsSuccessful, itemId, itemTblidx, byCurGrade, byNewGrade, stoneId, stoneTblidx, bCoreItemUse, coreId, coreItemIdx);
}

void CAuditLogRepository::InsertGmLog(CHARACTERID charId, BYTE byLogType, const char* message)
{
	GetLogDB.Execute("INSERT INTO gm_log (char_id, log_type, string) VALUES (%u,%u,'%s')", charId, byLogType, message);
}

void CAuditLogRepository::InsertMailDeletedLog(DWORD mailId, CHARACTERID charId, BYTE bySenderType, BYTE byMailType, BYTE byTextSize, const char* text, DWORD zenny, ITEMID itemId, WCHAR* wszFromName,
	int bIsAccept, int bIsLock, int bIsRead, DBOTIME endTime, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second)
{
	GetLogDB.Execute("INSERT INTO mail_deleted (id,char_id,sender_type,mail_type,text_size,text,zenny,item_id,from_name,is_accept,is_lock,is_read,end_time, year, month, day, hour, minute, second)"
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
	GetLogDB.Execute("INSERT INTO TradeLogs (char_id, target_char_id, zeni, item_count, item_id_1,item_tblidx_1, item_id_2,item_tblidx_2, item_id_3,item_tblidx_3, item_id_4,item_tblidx_4, item_id_5,item_tblidx_5, item_id_6,item_tblidx_6"
		", item_id_7,item_tblidx_7, item_id_8,item_tblidx_8, item_id_9,item_tblidx_9, item_id_10,item_tblidx_10, item_id_11, item_tblidx_11, item_id_12,item_tblidx_12)"
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
	GetLogDB.Execute("INSERT INTO PrivateShopLogs (seller_char_id, buyer_char_id, zeni, item_count, item_id_1,item_tblidx_1, item_id_2,item_tblidx_2, item_id_3,item_tblidx_3, item_id_4,item_tblidx_4, item_id_5,item_tblidx_5, item_id_6,item_tblidx_6"
		", item_id_7,item_tblidx_7, item_id_8,item_tblidx_8, item_id_9,item_tblidx_9, item_id_10,item_tblidx_10, item_id_11, item_tblidx_11, item_id_12,item_tblidx_12, has_issues,issue_reason)"
		"VALUES(%u,%u,%u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%I64u,%u,%u,'%s')",
		sellerCharId, buyerCharId, zeni, itemCount,
		itemId1, itemTblidx1, itemId2, itemTblidx2, itemId3, itemTblidx3,
		itemId4, itemTblidx4, itemId5, itemTblidx5, itemId6, itemTblidx6,
		itemId7, itemTblidx7, itemId8, itemTblidx8, itemId9, itemTblidx9,
		itemId10, itemTblidx10, itemId11, itemTblidx11, itemId12, itemTblidx12,
		bHasIssues, issueReason);
}
