#include "stdafx.h"
#include "DynamicFieldSystem.h"
#include "QueryServer.h"
#include "Repository/DynamicFieldRepository.h"


CDynamicFieldSystem::CDynamicFieldSystem()
{
	Init();
}

CDynamicFieldSystem::~CDynamicFieldSystem()
{
	Destroy();
}



void CDynamicFieldSystem::Init()
{
	smart_ptr<QueryResult> item = g_pDynamicFieldRepository->GetCount();
	if (item)
	{
		Field* i = item->Fetch();
		if (i)
		{
			m_dwCurCount = i->GetDWORD();
		}
	}
}

void CDynamicFieldSystem::Destroy()
{
}



