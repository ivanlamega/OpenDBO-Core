#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"

class CDynamicFieldRepository : public CNtlSingleton<CDynamicFieldRepository>
{

public:

	CDynamicFieldRepository() {}
	virtual ~CDynamicFieldRepository() {}

public:

	smart_ptr<QueryResult>		GetCount();
	void						ResetCount(DWORD serverFarmId);
	void						UpdateCount(DWORD totalCount, DWORD serverFarmId);

};

#define GetDynamicFieldRepository()		CDynamicFieldRepository::GetInstance()
#define g_pDynamicFieldRepository			GetDynamicFieldRepository()
