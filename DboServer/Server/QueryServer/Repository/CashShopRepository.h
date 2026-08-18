#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "HLSItemTable.h"
#include "NtlSharedType.h"

class CCashShopRepository : public CNtlSingleton<CCashShopRepository>
{

public:

	CCashShopRepository() {}
	virtual ~CCashShopRepository() {}

public:

	smart_ptr<QueryResult>		GetMaxProductId();

	void						InsertStorageItem(QWORD productId, ACCOUNTID accountId, TBLIDX hlsItemTblidx, BYTE stackCount, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second, WORD millisecond, ACCOUNTID buyer, DWORD price);
	void						UpdateStorageItemMoved(ITEMID itemId, QWORD productId);
	void						UpdateStorageItemMovedOnly(QWORD productId);
	void						InsertGiftStorageItem(QWORD productId, ACCOUNTID accountId, TBLIDX hlsItemTblidx, BYTE stackCount, CHARACTERID giftCharId, WCHAR* wszSenderName, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second, WORD millisecond, ACCOUNTID buyer, DWORD price);

};

#define GetCashShopRepository()		CCashShopRepository::GetInstance()
#define g_pCashShopRepository			GetCashShopRepository()
