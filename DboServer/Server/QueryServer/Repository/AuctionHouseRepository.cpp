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
	GetCharDB.Execute("DELETE FROM auctionhouse WHERE CharID=%u", charId);
}

void CAuctionHouseRepository::InsertListing(ITEMID id, CHARACTERID charId, BYTE byTabType, WCHAR* wszItemName, WCHAR* wszSeller, DWORD dwPrice, ITEMID itemId, DBOTIME nStartSellTime, DBOTIME nEndSellTime, BYTE byItemLevel, DWORD dwNeedClass, BYTE byItemType)
{
	GetCharDB.Execute("INSERT INTO auctionhouse (id,CharID,TabType,ItemName,Seller,Price,ItemID,TimeStart,TimeEnd,ItemLevel,NeedClass,ItemType) VALUES (%I64u, %u, %u,\"%ls\",\"%ls\", %u, %I64u, %I64u, %u, %u, %u, %u)",
		id, charId, byTabType, wszItemName, wszSeller, dwPrice, itemId, nStartSellTime, nEndSellTime, byItemLevel, dwNeedClass, byItemType);
}

void CAuctionHouseRepository::InsertLog(CHARACTERID sellerCharId, CHARACTERID buyerCharId, DWORD dwPrice, TBLIDX itemTblidx, ITEMID itemId)
{
	GetLogDB.Execute("INSERT INTO auctionhouse_log (Seller,Buyer,Price,ItemTblidx,ItemID) VALUES (%u,%u,%u,%u,%I64u)", sellerCharId, buyerCharId, dwPrice, itemTblidx, itemId);
}

void CAuctionHouseRepository::UpdateSellerName(WCHAR* wszName, CHARACTERID charId)
{
	GetCharDB.Execute("UPDATE auctionhouse SET Seller=\"%ls\" WHERE CharID=%u", wszName, charId);
}
