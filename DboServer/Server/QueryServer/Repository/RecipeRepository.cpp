#include "stdafx.h"
#include "RecipeRepository.h"
#include "QueryServer.h"


void CRecipeRepository::DeleteRecipes(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM hoipoi_recipe WHERE CharID=%u", charId);
}

void CRecipeRepository::InsertRecipe(CHARACTERID charId, TBLIDX recipeTblidx, BYTE byRecipeType)
{
	GetCharDB.Execute("INSERT INTO hoipoi_recipe (CharID,RecipeTblidx,RecipeType) VALUES (%u,%u,%u)", charId, recipeTblidx, byRecipeType);
}
