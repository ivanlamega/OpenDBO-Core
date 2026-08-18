#include "stdafx.h"
#include "AuctionHouseRepository.h"
#include "../QueryServer.h"


smart_ptr<QueryResult> CAuctionHouseRepository::LoadAuctionHouse()
{
	return GetCharDB.Query("SELECT * FROM auctionhouse");
}

void CAuctionHouseRepository::DeleteListing(ITEMID itemId)
{
	GetCharDB.Execute("DELETE FROM auctionhouse WHERE id=%I64u", itemId);
}

void CAuctionHouseRepository::DeleteListingsByChar(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM auctionhouse WHERE char_id=%u", charId);
}

void CAuctionHouseRepository::InsertListing(ITEMID id, CHARACTERID charId, BYTE byTabType, WCHAR* wszItemName, WCHAR* wszSeller, DWORD dwPrice, ITEMID itemId, DBOTIME nStartSellTime, DBOTIME nEndSellTime, BYTE byItemLevel, DWORD dwNeedClass, BYTE byItemType)
{
	GetCharDB.Execute("INSERT INTO auctionhouse (id,char_id,tab_type,item_name,seller_name,price,item_id,time_start,time_end,item_level,need_class,item_type) VALUES (%I64u, %u, %u,\"%ls\",\"%ls\", %u, %I64u, %I64u, %u, %u, %u, %u)",
		id, charId, byTabType, wszItemName, wszSeller, dwPrice, itemId, nStartSellTime, nEndSellTime, byItemLevel, dwNeedClass, byItemType);
}

void CAuctionHouseRepository::InsertLog(CHARACTERID sellerCharId, CHARACTERID buyerCharId, DWORD dwPrice, TBLIDX itemTblidx, ITEMID itemId)
{
	GetLogDB.Execute("INSERT INTO auctionhouse_log (seller_char_id,buyer_char_id,price,item_tblidx,item_id) VALUES (%u,%u,%u,%u,%I64u)", sellerCharId, buyerCharId, dwPrice, itemTblidx, itemId);
}

void CAuctionHouseRepository::UpdateSellerName(WCHAR* wszName, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE auctionhouse SET seller_name=\"%ls\" WHERE char_id=%u", wszName, charId);
}
