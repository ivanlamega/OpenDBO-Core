#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "NtlSharedType.h"

class CAuditLogRepository : public CNtlSingleton<CAuditLogRepository>
{

public:

	CAuditLogRepository() {}
	virtual ~CAuditLogRepository() {}

public:

	void						InsertCharacterDeleteLog(ACCOUNTID accountId, CHARACTERID charId);

	void						InsertAccountBanLog(ACCOUNTID gmAccountId, ACCOUNTID targetAccountId, const char* reason, DWORD byDuration);
	void						InsertCharNameChangeLog(CHARACTERID charId, WCHAR* wszOldName, WCHAR* wszNewName);
	void						InsertItemUpgradeLog(CHARACTERID charId, int bIsSuccessful, ITEMID itemId, TBLIDX itemTblidx, BYTE byCurGrade, BYTE byNewGrade, ITEMID stoneId, TBLIDX stoneTblidx, int bCoreItemUse, ITEMID coreId, TBLIDX coreItemIdx);
	void						InsertGmLog(CHARACTERID charId, BYTE byLogType, const char* message);

	void						InsertMailDeletedLog(DWORD mailId, CHARACTERID charId, BYTE bySenderType, BYTE byMailType, BYTE byTextSize, const char* text, DWORD zenny, ITEMID itemId, WCHAR* wszFromName,
									int bIsAccept, int bIsLock, int bIsRead, DBOTIME endTime, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second);

	void						InsertMuteLog(CHARACTERID charId, ACCOUNTID gmAccountId, DWORD durationInMinutes, WCHAR* wszReason, DBOTIME muteUntil);
	void						DeleteMuteLog(CHARACTERID charId);

	void						InsertSlotMachineLog(ACCOUNTID accountId, CHARACTERID charId, BYTE extractCount, BYTE machineType, WORD coin, DWORD currentPoints, DWORD newPoints, QWORD productId1, QWORD productId2, QWORD productId3, QWORD productId4, QWORD productId5, QWORD productId6, QWORD productId7, QWORD productId8, QWORD productId9, QWORD productId10);

	void						InsertTradeLog(CHARACTERID charId, CHARACTERID targetCharId, DWORD zeni, BYTE itemCount,
									ITEMID itemId1, TBLIDX itemTblidx1, ITEMID itemId2, TBLIDX itemTblidx2, ITEMID itemId3, TBLIDX itemTblidx3,
									ITEMID itemId4, TBLIDX itemTblidx4, ITEMID itemId5, TBLIDX itemTblidx5, ITEMID itemId6, TBLIDX itemTblidx6,
									ITEMID itemId7, TBLIDX itemTblidx7, ITEMID itemId8, TBLIDX itemTblidx8, ITEMID itemId9, TBLIDX itemTblidx9,
									ITEMID itemId10, TBLIDX itemTblidx10, ITEMID itemId11, TBLIDX itemTblidx11, ITEMID itemId12, TBLIDX itemTblidx12);

	void						InsertPrivateShopLog(CHARACTERID sellerCharId, CHARACTERID buyerCharId, DWORD zeni, BYTE itemCount,
									ITEMID itemId1, TBLIDX itemTblidx1, ITEMID itemId2, TBLIDX itemTblidx2, ITEMID itemId3, TBLIDX itemTblidx3,
									ITEMID itemId4, TBLIDX itemTblidx4, ITEMID itemId5, TBLIDX itemTblidx5, ITEMID itemId6, TBLIDX itemTblidx6,
									ITEMID itemId7, TBLIDX itemTblidx7, ITEMID itemId8, TBLIDX itemTblidx8, ITEMID itemId9, TBLIDX itemTblidx9,
									ITEMID itemId10, TBLIDX itemTblidx10, ITEMID itemId11, TBLIDX itemTblidx11, ITEMID itemId12, TBLIDX itemTblidx12,
									BOOL bHasIssues, const char* issueReason);

};

#define GetAuditLogRepository()		CAuditLogRepository::GetInstance()
#define g_pAuditLogRepository			GetAuditLogRepository()
