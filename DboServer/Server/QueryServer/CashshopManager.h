#ifndef __CASHSHOP_MANAGER_H__
#define __CASHSHOP_MANAGER_H__

#include "NtlSingleton.h"
#include "HLSItemTable.h"
#include "NtlMutex.h"

class CCashshopManager : public CNtlSingleton<CCashshopManager>
{

public:

	CCashshopManager();
	virtual ~CCashshopManager();


private:

	void						Init();

public:

	// called from multiple QueryServer IOCP worker threads (cash shop
	// purchases, mail, slot-machine draws); ++m_qwLastProductId is not
	// atomic on its own, so concurrent callers can hand out the same id,
	// which then collides on the cashshop_storage primary key
	QWORD						AcquireProductId();

private:

	CNtlMutex					m_mutex;

	QWORD						m_qwLastProductId;

};

#define GetCashshopManager()	CCashshopManager::GetInstance()
#define g_pCashshopManager		GetCashshopManager()

#endif