#include "stdafx.h"
#include "MailRepository.h"
#include "QueryServer.h"
#include "PlayerCache.h"


void CMailRepository::DeleteMailByChar(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM mail WHERE CharID=%u", charId);
}

void CMailRepository::InsertItemMail(CHARACTERID charId, BYTE bySenderType, BYTE byMailType, int textSize, WCHAR* wszText, ITEMID itemId, DBOTIME createTime, DBOTIME endTime, WORD remainDay, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second)
{
	GetCharDB.Execute("INSERT INTO mail (CharID, SenderType, MailType, TextSize, Text, itemId, FromName, CreateTime, EndTime, RemainDay,year,month,day,hour,minute,second) VALUES (%u,%u,%u,%u,\"%ls\",%I64u,'System',%I64u,%I64u,%u,%u,%u,%u,%u,%u,%u)",
		charId, bySenderType, byMailType, textSize, wszText, itemId, createTime, endTime, remainDay, year, month, day, hour, minute, second);
}

void CMailRepository::InsertZennyMail(CHARACTERID charId, BYTE bySenderType, BYTE byMailType, int textSize, WCHAR* wszText, DWORD dwZenny, DBOTIME createTime, DBOTIME endTime, WORD remainDay, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second)
{
	GetCharDB.Execute("INSERT INTO mail (CharID, SenderType, MailType, TextSize, Text, Zenny, FromName, CreateTime, EndTime, RemainDay,year,month,day,hour,minute,second) VALUES (%u,%u,%u,%u,\"%ls\",%u,'System',%I64u,%I64u,%u,%u,%u,%u,%u,%u,%u)",
		charId, bySenderType, byMailType, textSize, wszText, dwZenny, createTime, endTime, remainDay, year, month, day, hour, minute, second);
}

void CMailRepository::InsertBasicMail(CHARACTERID charId, BYTE bySenderType, BYTE byMailType, int textSize, WCHAR* wszText, WCHAR* wszFromName, DBOTIME createTime, DBOTIME endTime, WORD remainDay, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second)
{
	GetCharDB.Execute("INSERT INTO mail (CharID, SenderType, MailType, TextSize, Text, FromName, CreateTime, EndTime, RemainDay,year,month,day,hour,minute,second) VALUES (%u,%u,%u,%u, \"%ls\", \"%ls\", %I64u,%I64u,%u,%u,%u,%u,%u,%u,%u)",
		charId, bySenderType, byMailType, textSize, wszText, wszFromName, createTime, endTime, remainDay, year, month, day, hour, minute, second);
}

void CMailRepository::LoadMailboxAsync(CPlayerCache* pCache, CHARACTERID charId, HOBJECT handle, HOBJECT hObject)
{
	SQLCallbackBase* pCallBack = new SQLClassCallbackP2<CPlayerCache, HOBJECT, HOBJECT>(pCache, &CPlayerCache::StartMailResult, handle, hObject);
	AsyncQuery * q = new AsyncQuery(pCallBack);
	q->AddQuery("SELECT * FROM mail WHERE CharID=%u LIMIT 30", charId); // LIMIT NTL_MAX_MAIL_SLOT_COUNT
	GetCharDB.QueueAsyncQuery(q);
}

void CMailRepository::LoadMailDetailAsync(CPlayerCache* pCache, CHARACTERID charId, HOBJECT handle, HOBJECT hObject)
{
	SQLCallbackBase* pCallBack = new SQLClassCallbackP2<CPlayerCache, HOBJECT, HOBJECT>(pCache, &CPlayerCache::LoadMailResult, handle, hObject);
	AsyncQuery * q = new AsyncQuery(pCallBack);
	q->AddQuery("SELECT * FROM mail WHERE CharID=%u LIMIT 30", charId); // LIMIT NTL_MAX_MAIL_SLOT_COUNT
	GetCharDB.QueueAsyncQuery(q);
}

void CMailRepository::ReloadMailboxAsync(CPlayerCache* pCache, CHARACTERID charId, HOBJECT handle, bool bIsSchedule)
{
	SQLCallbackBase* pCallBack = new SQLClassCallbackP2<CPlayerCache, HOBJECT, bool>(pCache, &CPlayerCache::ScheduleReloadMailsResult, handle, bIsSchedule);
	AsyncQuery * q = new AsyncQuery(pCallBack);
	q->AddQuery("SELECT id,MailType,Zenny,IsAccept,IsLock,IsRead,SenderType,EndTime FROM mail WHERE CharID=%u LIMIT 30", charId); // LIMIT NTL_MAX_MAIL_SLOT_COUNT
	GetCharDB.QueueAsyncQuery(q);
}

void CMailRepository::InsertTargetedMail(CHARACTERID targetCharId, BYTE bySenderType, BYTE byMailType, BYTE byTextSize, const char* text, DWORD zenny, ITEMID itemId, const char* targetNameUtf8, WCHAR* wszFromName, BYTE remainDay, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second)
{
	GetCharDB.Execute("INSERT INTO mail (CharID,SenderType,MailType,TextSize,Text,Zenny,itemId,TargetName,FromName,RemainDay,year,month,day,hour,minute,second)"
		"VALUES(%u,%u,%u,%u,\"%s\",%u, %I64u,\"%s\",\"%ls\",%u,%u,%u,%u,%u,%u,%u)",
		targetCharId, bySenderType, byMailType, byTextSize, text, zenny, itemId, targetNameUtf8, wszFromName, remainDay, year, month, day, hour, minute, second);
}

void CMailRepository::UpdateReadFlag(DWORD mailId)
{
	GetCharDB.Execute("UPDATE mail SET IsRead=1 WHERE id=%u", mailId);
}

void CMailRepository::UpdateAcceptFlag(DWORD mailId)
{
	GetCharDB.Execute("UPDATE mail SET IsAccept=1 WHERE id=%u", mailId);
}

void CMailRepository::UpdateLockFlag(BOOL bIsLock, DWORD mailId)
{
	GetCharDB.Execute("UPDATE mail SET IsLock=%i WHERE id=%u", bIsLock, mailId);
}

void CMailRepository::DeleteMailById(DWORD mailId)
{
	GetCharDB.Execute("DELETE FROM mail WHERE id=%u", mailId);
}

smart_ptr<QueryResult> CMailRepository::GetFromNameById(DWORD mailId)
{
	return GetCharDB.Query("SELECT FromName FROM mail WHERE id=%u LIMIT 1", mailId); //we have to do this, in case the sender changed his char name
}

void CMailRepository::UpdateReturnToSender(CHARACTERID newCharId, const char* fromNameUtf8, WCHAR* wszNewFromName, DWORD mailId)
{
	GetCharDB.Execute("UPDATE mail SET CharID=%u,SenderType=2,TargetName=\"%s\",FromName=\"%ls\",IsLock=0,IsRead=0 WHERE id=%u  LIMIT 1", newCharId, fromNameUtf8, wszNewFromName, mailId);
}

void CMailRepository::UpdateFromNameByOldName(WCHAR* wszNewName, WCHAR* wszOldName)
{
	GetCharDB.Execute("UPDATE mail SET FromName=\"%ls\" WHERE FromName=\"%ls\"", wszNewName, wszOldName);
}

void CMailRepository::UpdateTargetNameByOldName(WCHAR* wszNewName, WCHAR* wszOldName)
{
	GetCharDB.Execute("UPDATE mail SET TargetName=\"%ls\" WHERE TargetName=\"%ls\"", wszNewName, wszOldName);
}
