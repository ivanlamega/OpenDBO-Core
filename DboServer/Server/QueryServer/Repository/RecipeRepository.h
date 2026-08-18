#pragma once

#include "NtlSingleton.h"
#include "DatabaseEnv.h"
#include "NtlSharedType.h"

class CRecipeRepository : public CNtlSingleton<CRecipeRepository>
{

public:

	CRecipeRepository() {}
	virtual ~CRecipeRepository() {}

public:

	void						DeleteRecipes(CHARACTERID charId);
	void						InsertRecipe(CHARACTERID charId, TBLIDX recipeTblidx, BYTE byRecipeType);

};

#define GetRecipeRepository()		CRecipeRepository::GetInstance()
#define g_pRecipeRepository		GetRecipeRepository()
