#include "stdafx.h"
#include "DynamicFieldRepository.h"
#include "../QueryServer.h"


smart_ptr<QueryResult> CDynamicFieldRepository::GetCount()
{
	return GetLogDB.Query("SELECT count FROM dynamic_field_count WHERE serverIndex=0");
}

void CDynamicFieldRepository::ResetCount(DWORD serverFarmId)
{
	GetLogDB.Execute("UPDATE dynamic_field_count SET count=0 WHERE serverIndex=%u", serverFarmId);
}

void CDynamicFieldRepository::UpdateCount(DWORD totalCount, DWORD serverFarmId)
{
	GetLogDB.Execute("UPDATE dynamic_field_count SET count=%u WHERE serverIndex=%u", totalCount, serverFarmId);
}
