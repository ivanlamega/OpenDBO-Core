#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "NtlItem.h"

class CPlayerCache;

class CMailRepository : public CNtlSingleton<CMailRepository>
{

public:

	CMailRepository() {}
	virtual ~CMailRepository() {}

public:

	void						DeleteMailByChar(CHARACTERID charId);

	void						InsertItemMail(CHARACTERID charId, BYTE bySenderType, BYTE byMailType, int textSize, WCHAR* wszText, ITEMID itemId, DBOTIME createTime, DBOTIME endTime, WORD remainDay, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second);
	void						InsertZennyMail(CHARACTERID charId, BYTE bySenderType, BYTE byMailType, int textSize, WCHAR* wszText, DWORD dwZenny, DBOTIME createTime, DBOTIME endTime, WORD remainDay, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second);
	void						InsertBasicMail(CHARACTERID charId, BYTE bySenderType, BYTE byMailType, int textSize, WCHAR* wszText, WCHAR* wszFromName, DBOTIME createTime, DBOTIME endTime, WORD remainDay, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second);

	void						LoadMailboxAsync(CPlayerCache* pCache, CHARACTERID charId, HOBJECT handle, HOBJECT hObject);
	void						LoadMailDetailAsync(CPlayerCache* pCache, CHARACTERID charId, HOBJECT handle, HOBJECT hObject);
	void						ReloadMailboxAsync(CPlayerCache* pCache, CHARACTERID charId, HOBJECT handle, bool bIsSchedule);

	void						InsertTargetedMail(CHARACTERID targetCharId, BYTE bySenderType, BYTE byMailType, BYTE byTextSize, const char* text, DWORD zenny, ITEMID itemId, const char* targetNameUtf8, WCHAR* wszFromName, BYTE remainDay, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second);

	void						UpdateReadFlag(DWORD mailId);
	void						UpdateAcceptFlag(DWORD mailId);
	void						UpdateLockFlag(BOOL bIsLock, DWORD mailId);
	void						DeleteMailById(DWORD mailId);

	smart_ptr<QueryResult>		GetFromNameById(DWORD mailId);
	void						UpdateReturnToSender(CHARACTERID newCharId, const char* fromNameUtf8, WCHAR* wszNewFromName, DWORD mailId);

	void						UpdateFromNameByOldName(WCHAR* wszNewName, WCHAR* wszOldName);
	void						UpdateTargetNameByOldName(WCHAR* wszNewName, WCHAR* wszOldName);

};

#define GetMailRepository()		CMailRepository::GetInstance()
#define g_pMailRepository			GetMailRepository()
