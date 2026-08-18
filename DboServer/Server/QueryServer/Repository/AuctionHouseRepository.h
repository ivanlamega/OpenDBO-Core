#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "NtlItem.h"

class CAuctionHouseRepository : public CNtlSingleton<CAuctionHouseRepository>
{

public:

	CAuctionHouseRepository() {}
	virtual ~CAuctionHouseRepository() {}

public:

	smart_ptr<QueryResult>			LoadAuctionHouse();

	void							DeleteListing(ITEMID itemId);
	void							DeleteListingsByChar(CHARACTERID charId);

	void							InsertListing(ITEMID id, CHARACTERID charId, BYTE byTabType, WCHAR* wszItemName, WCHAR* wszSeller, DWORD dwPrice, ITEMID itemId, DBOTIME nStartSellTime, DBOTIME nEndSellTime, BYTE byItemLevel, DWORD dwNeedClass, BYTE byItemType);
	void							InsertLog(CHARACTERID sellerCharId, CHARACTERID buyerCharId, DWORD dwPrice, TBLIDX itemTblidx, ITEMID itemId);
	void							UpdateSellerName(WCHAR* wszName, CHARACTERID charId);

};

#define GetAuctionHouseRepository()		CAuctionHouseRepository::GetInstance()
#define g_pAuctionHouseRepository			GetAuctionHouseRepository()
