#include "stdafx.h"
#include "CashShopRepository.h"
#include "../QueryServer.h"


smart_ptr<QueryResult> CCashShopRepository::GetMaxProductId()
{
	return GetAccDB.Query("SELECT MAX(id) FROM cashshop_storage");
}

void CCashShopRepository::InsertStorageItem(QWORD productId, ACCOUNTID accountId, TBLIDX hlsItemTblidx, BYTE stackCount, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second, WORD millisecond, ACCOUNTID buyer, DWORD price)
{
	GetAccDB.Execute("INSERT INTO cashshop_storage (id,account_id,hls_item_tblidx,stack_count,year,month,day,hour,minute,second,millisecond,buyer_account_id,price)VALUES(%I64u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)",
		productId, accountId, hlsItemTblidx, stackCount, year, month, day, hour, minute, second, millisecond, buyer, price);
}

void CCashShopRepository::UpdateStorageItemMoved(ITEMID itemId, QWORD productId)
{
	GetAccDB.Execute("UPDATE cashshop_storage SET is_moved=1, item_id=%I64u WHERE id=%I64u", itemId, productId);
}

void CCashShopRepository::UpdateStorageItemMovedOnly(QWORD productId)
{
	GetAccDB.Execute("UPDATE cashshop_storage SET is_moved=1 WHERE id=%I64u", productId);
}

void CCashShopRepository::InsertGiftStorageItem(QWORD productId, ACCOUNTID accountId, TBLIDX hlsItemTblidx, BYTE stackCount, CHARACTERID giftCharId, WCHAR* wszSenderName, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second, WORD millisecond, ACCOUNTID buyer, DWORD price)
{
	GetAccDB.Execute("INSERT INTO cashshop_storage (id,account_id,hls_item_tblidx,stack_count,gift_char_id,sender_name,year,month,day,hour,minute,second,millisecond,buyer_account_id,price)VALUES(%I64u,%u,%u,%u,%u,\"%ls\",%u,%u,%u,%u,%u,%u,%u,%u,%u)",
		productId, accountId, hlsItemTblidx, stackCount, giftCharId, wszSenderName, year, month, day, hour, minute, second, millisecond, buyer, price);
}
