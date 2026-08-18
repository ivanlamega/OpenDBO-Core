#include "stdafx.h"
#include "RecipeRepository.h"
#include "../QueryServer.h"


void CRecipeRepository::DeleteRecipes(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM hoipoi_recipe WHERE char_id=%u", charId);
}

void CRecipeRepository::InsertRecipe(CHARACTERID charId, TBLIDX recipeTblidx, BYTE byRecipeType)
{
	GetCharDB.Execute("INSERT INTO hoipoi_recipe (char_id,recipe_tblidx,recipe_type) VALUES (%u,%u,%u)", charId, recipeTblidx, byRecipeType);
}
