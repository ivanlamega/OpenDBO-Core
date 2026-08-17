#include "stdafx.h"
#include "CashShopRepository.h"
#include "../QueryServer.h"


smart_ptr<QueryResult> CCashShopRepository::GetMaxProductId()
{
	return GetAccDB.Query("SELECT MAX(ProductId) FROM cashshop_storage");
}

void CCashShopRepository::InsertStorageItem(QWORD productId, ACCOUNTID accountId, TBLIDX hlsItemTblidx, BYTE stackCount, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second, WORD millisecond, ACCOUNTID buyer, DWORD price)
{
	GetAccDB.Execute("INSERT INTO cashshop_storage (ProductId,AccountID,HLSitemTblidx,StackCount,year,month,day,hour,minute,second,millisecond,Buyer,price)VALUES(%I64u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)",
		productId, accountId, hlsItemTblidx, stackCount, year, month, day, hour, minute, second, millisecond, buyer, price);
}

void CCashShopRepository::UpdateStorageItemMoved(ITEMID itemId, QWORD productId)
{
	GetAccDB.Execute("UPDATE cashshop_storage SET isMoved=1, ItemID=%I64u WHERE ProductId=%I64u", itemId, productId);
}

void CCashShopRepository::UpdateStorageItemMovedOnly(QWORD productId)
{
	GetAccDB.Execute("UPDATE cashshop_storage SET isMoved=1 WHERE ProductId=%I64u", productId);
}

void CCashShopRepository::InsertGiftStorageItem(QWORD productId, ACCOUNTID accountId, TBLIDX hlsItemTblidx, BYTE stackCount, CHARACTERID giftCharId, WCHAR* wszSenderName, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second, WORD millisecond, ACCOUNTID buyer, DWORD price)
{
	GetAccDB.Execute("INSERT INTO cashshop_storage (ProductId,AccountID,HLSitemTblidx,StackCount,giftCharId,SenderName,year,month,day,hour,minute,second,millisecond,Buyer,price)VALUES(%I64u,%u,%u,%u,%u,\"%ls\",%u,%u,%u,%u,%u,%u,%u,%u,%u)",
		productId, accountId, hlsItemTblidx, stackCount, giftCharId, wszSenderName, year, month, day, hour, minute, second, millisecond, buyer, price);
}
