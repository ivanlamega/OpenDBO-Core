#include "pch.h"
#include "XmlExport.h"
#include "TableSchema.h"
#include "Util.h"
#include "ItemTable.h"
#include "NewbieTable.h"
#include "NpcSpeechTable.h"
#include "MascotTable.h"
#include "MascotGradeTable.h"
#include "MascotStatusTable.h"
#include "CharTitleTable.h"
#include "ChattingFilterTable.h"
#include "ItemEnchantTable.h"
#include "MerchantTable.h"
#include "FormulaTable.h"
#include "ItemMixExpTable.h"
#include "ChatCommandTable.h"
#include "HTBSetTable.h"
#include "DirectionLinkTable.h"
#include "DojoTable.h"
#include "VehicleTable.h"
#include "DungeonTable.h"
#include "MobMovePatternTable.h"
#include "DragonBallRewardTable.h"
#include "ExpTable.h"
#include "LandMarkTable.h"
#include "CharmTable.h"
#include "ActionTable.h"
#include "AirCostumeTable.h"
#include "QuestTextDataTable.h"
#include "HelpTable.h"
#include "GuideHintTable.h"
#include "TimeQuestTable.h"
#include "BudokaiTable.h"
#include "RankBattleTable.h"
#include "ScriptLinkTable.h"
#include "QuestNarrationTable.h"
#include "DynamicObjectTable.h"
#include "UseItemTable.h"
#include "SetItemTable.h"
#include "QuestItemTable.h"
#include "ItemDisassembleTable.h"
#include "PortalTable.h"
#include "ItemRecipeTable.h"
#include "ItemMixMachineTable.h"
#include "DragonBallTable.h"
#include "QuestRewardTable.h"
#include "QuestRewardSelectTable.h"
#include "WorldZoneTable.h"
#include "CommonConfigTable.h"
#include "DwcTable.h"
#include "NpcServerTable.h"
#include "WorldTable.h"
#include "WorldMapTable.h"
#include "SystemEffectTable.h"
#include "ItemOptionTable.h"
#include "SkillTable.h"
#include "PCTable.h"
#include "MobTable.h"
#include "NPCTable.h"
#include "HLSItemTable.h"
#include "StatusTransformTable.h"
#include "DwcMissionTable.h"
#include "QuestDropTable.h"
#include "QuestProbabilityTable.h"
#include "WorldPlayTable.h"
#include "SlotMachineTable.h"
#include "HlsSlotMachineItemTable.h"
#include "ItemUpgradeRateNewTable.h"
#include "ItemBagListTable.h"
#include "ItemGroupListTable.h"
#include "MobServerTable.h"
#include "DragonBallReturnPointTable.h"
#include "EventSystemTable.h"
#include "DynamicFieldSystemTable.h"
#include "TextAllTable.h"

#include <vector>
#include <map>
#include <functional>

namespace {

// ---- low-level UTF-16LE file writer -------------------------------------
//
// CTable::LoadFromXml loads through MSXML's IXMLDOMDocument::load(), which
// auto-detects encoding from the byte-order mark. Writing UTF-16LE + BOM
// sidesteps any ANSI-codepage ambiguity for non-ASCII text (Korean names,
// etc.) -- every WCHAR maps straight to Unicode, no codepage guessing.

bool WriteUtf16File(const CString& strPath, const CStringW& strContent)
{
	CFile file;
	if (!file.Open(strPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
	{
		return false;
	}

	WORD wBom = 0xFEFF;
	file.Write(&wBom, sizeof(wBom));
	file.Write((LPCWSTR)strContent, strContent.GetLength() * sizeof(WCHAR));
	file.Close();

	return true;
}

CStringW XmlEscape(const CStringW& s)
{
	CStringW out;
	int n = s.GetLength();
	for (int i = 0; i < n; ++i)
	{
		WCHAR ch = s[i];
		if (ch == L'&')
		{
			out += L"&amp;";
		}
		else if (ch == L'<')
		{
			out += L"&lt;";
		}
		else if (ch == L'>')
		{
			out += L"&gt;";
		}
		else
		{
			out += ch;
		}
	}
	return out;
}

// char[] fields are decoded with the same codepage CTableContainer was
// created with (GetACP(), see Util.cpp::CreateTableContainer), matching
// how CTable::READ_STRING re-encodes them on load.
CStringW AnsiToXml(const char* psz)
{
	if (!psz || !*psz)
	{
		return CStringW();
	}

	int nLen = ::MultiByteToWideChar(CP_ACP, 0, psz, -1, nullptr, 0);
	if (nLen <= 1)
	{
		return CStringW();
	}

	CStringW wide;
	::MultiByteToWideChar(CP_ACP, 0, psz, -1, wide.GetBuffer(nLen), nLen);
	wide.ReleaseBuffer();

	return XmlEscape(wide);
}

CStringW WideToXml(const WCHAR* psz)
{
	return XmlEscape(CStringW(psz ? psz : L""));
}

CStringW Dec(DWORD v)	{ CStringW s; s.Format(L"%u", v); return s; }
CStringW DecB(BYTE v)	{ CStringW s; s.Format(L"%u", (unsigned)v); return s; }
CStringW DecW(WORD v)	{ CStringW s; s.Format(L"%u", (unsigned)v); return s; }
CStringW Hex(DWORD v)	{ CStringW s; s.Format(L"%X", v); return s; }
CStringW Flt(float v)	{ CStringW s; s.Format(L"%g", v); return s; }
CStringW Dbl(double v)	{ CStringW s; s.Format(L"%g", v); return s; }
CStringW Bl(bool b)		{ return b ? L"1" : L"0"; }

// Cross-checks the value an extractor produced against the *declared*
// type from the schema JSON. This doesn't change what gets written --
// the extractor already decided the format -- it exists to catch the
// case where the schema was hand-edited (or drifted) to claim a type
// that doesn't match what the C++ extractor for that field name actually
// produces, which would otherwise be a silent, hard-to-notice bug.
bool ValidateFieldValue(const CStringW& strType, const CStringW& strValue, CString& strError)
{
	if (strType == L"bool")
	{
		if (strValue != L"0" && strValue != L"1")
		{
			strError.Format(_T("expected \"0\" or \"1\" for a bool field, got \"%s\""), (LPCTSTR)CString(strValue));
			return false;
		}
	}
	else if (strType == L"byte" || strType == L"word" || strType == L"dword")
	{
		for (int i = 0; i < strValue.GetLength(); ++i)
		{
			if (!iswdigit(strValue[i]))
			{
				strError.Format(_T("expected a decimal number, got \"%s\""), (LPCTSTR)CString(strValue));
				return false;
			}
		}
	}
	else if (strType == L"hex")
	{
		for (int i = 0; i < strValue.GetLength(); ++i)
		{
			if (!iswxdigit(strValue[i]))
			{
				strError.Format(_T("expected a hex number, got \"%s\""), (LPCTSTR)CString(strValue));
				return false;
			}
		}
	}
	// "float", "ansi", "wide" -- no useful cross-check beyond "it parsed",
	// which the extractor already guarantees by construction.

	return true;
}

// A row's cell tag names (<F1>, <F2>, ...) aren't actually read by name --
// CTable::InitializeFromXmlDoc walks cells by position (get_item(i, ...)).
// Only the header row's cell *text* supplies the field names the reader
// matches on; every other row just needs to line up positionally with it.
void AppendRow(CStringW& out, const CStringW& strSheet, const std::vector<CStringW>& cells)
{
	out += L"<";
	out += strSheet;
	out += L">";

	for (size_t i = 0; i < cells.size(); ++i)
	{
		out.AppendFormat(L"<F%u>%s</F%u>", (unsigned)(i + 1), (LPCWSTR)cells[i], (unsigned)(i + 1));
	}

	out += L"</";
	out += strSheet;
	out += L">\r\n";
}

// ---- per-table field extractors -------------------------------------------
//
// The schema JSON drives *which* fields get exported and in what order.
// It can't drive *how* to pull a value off a raw C++ struct pointer --
// these structs have no reflection, so that part is unavoidably C++. Each
// map below is the one place that coupling lives, keyed by the same field
// names used in schema/<table>.schema.json.

typedef std::function<CStringW(sTBLDAT*)> FieldExtractor;
typedef std::map<CStringW, FieldExtractor> FieldExtractorMap;

#define ITEM_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sITEM_TBLDAT* p = (sITEM_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetItemExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		ITEM_FIELD(L"Tblidx", Dec(p->tblidx));
		ITEM_FIELD(L"Validity_Able", Bl(p->bValidity_Able));
		ITEM_FIELD(L"Name", Dec(p->Name));
		ITEM_FIELD(L"Name_Text", WideToXml(p->wszNameText));
		ITEM_FIELD(L"Icon_Name", AnsiToXml(p->szIcon_Name));
		ITEM_FIELD(L"Model_Type", DecB(p->byModel_Type));
		ITEM_FIELD(L"Model", AnsiToXml(p->szModel));
		ITEM_FIELD(L"Sub_Weapon_Act_Model", AnsiToXml(p->szSub_Weapon_Act_Model));
		ITEM_FIELD(L"Item_Type", DecB(p->byItem_Type));
		ITEM_FIELD(L"Equip_Type", DecB(p->byEquip_Type));
		ITEM_FIELD(L"Equip_Slot_Type_Bit_Flag", Hex(p->dwEquip_Slot_Type_Bit_Flag));
		ITEM_FIELD(L"Function_Bit_Flag", Hex(p->wFunction_Bit_Flag));
		ITEM_FIELD(L"Max_Stack", DecB(p->byMax_Stack));
		ITEM_FIELD(L"Rank", DecB(p->byRank));
		ITEM_FIELD(L"Weight", Dec(p->dwWeight));
		ITEM_FIELD(L"Cost", Dec(p->dwCost));
		ITEM_FIELD(L"Sell_Price", Dec(p->dwSell_Price));
		ITEM_FIELD(L"Durability", DecB(p->byDurability));
		ITEM_FIELD(L"Durability_Count", DecB(p->byDurability_Count));
		ITEM_FIELD(L"Battle_Attribute", DecB(p->byBattle_Attribute));
		ITEM_FIELD(L"Physical_Offence", DecW(p->wPhysical_Offence));
		ITEM_FIELD(L"Energy_Offence", DecW(p->wEnergy_Offence));
		ITEM_FIELD(L"Physical_Defence", DecW(p->wPhysical_Defence));
		ITEM_FIELD(L"Energy_Defence", DecW(p->wEnergy_Defence));
		ITEM_FIELD(L"Attack_Range_Bonus", Flt(p->fAttack_Range_Bonus));
		ITEM_FIELD(L"Attack_Speed_Rate", DecW(p->wAttack_Speed_Rate));
		ITEM_FIELD(L"Need_Level", DecB(p->byNeed_Min_Level));
		ITEM_FIELD(L"Need_Max_Level", DecB(p->byNeed_Max_Level));
		ITEM_FIELD(L"Need_Class_Bit_Flag", Hex(p->dwNeed_Class_Bit_Flag));
		ITEM_FIELD(L"Need_Gender_Bit_Flag", Hex(p->dwNeed_Gender_Bit_Flag));
		ITEM_FIELD(L"Class_Special", DecB(p->byClass_Special));
		ITEM_FIELD(L"Race_Special", DecB(p->byRace_Special));
		ITEM_FIELD(L"Need_Str", DecW(p->wNeed_Str));
		ITEM_FIELD(L"Need_Con", DecW(p->wNeed_Con));
		ITEM_FIELD(L"Need_Foc", DecW(p->wNeed_Foc));
		ITEM_FIELD(L"Need_Dex", DecW(p->wNeed_Dex));
		ITEM_FIELD(L"Need_Sol", DecW(p->wNeed_Sol));
		ITEM_FIELD(L"Need_Eng", DecW(p->wNeed_Eng));
		ITEM_FIELD(L"Set_Item_Tblidx", Dec(p->set_Item_Tblidx));
		ITEM_FIELD(L"Note", Dec(p->Note));
		ITEM_FIELD(L"Bag_Size", DecB(p->byBag_Size));
		ITEM_FIELD(L"Scouter_Watt", DecW(p->wScouter_Watt));
		ITEM_FIELD(L"Scouter_MaxPower", Dec(p->dwScouter_MaxPower));
		ITEM_FIELD(L"Use_Item_Tblidx", Dec(p->Use_Item_Tblidx));
		ITEM_FIELD(L"bCan_Have_Option", Bl(p->bIsCanHaveOption));
		ITEM_FIELD(L"Item_Option_Tblidx", Dec(p->Item_Option_Tblidx));
		ITEM_FIELD(L"Item_Group", DecB(p->byItemGroup));
		ITEM_FIELD(L"Charm_Tblidx", Dec(p->Charm_Tblidx));
		ITEM_FIELD(L"Costume_Hide_Bit_Flag", Hex(p->wCostumeHideBitFlag));
		ITEM_FIELD(L"Need_Item_Tblidx", Dec(p->NeedItemTblidx));
		ITEM_FIELD(L"Common_Point", Dec(p->CommonPoint));
		ITEM_FIELD(L"Common_Point_Type", DecB(p->byCommonPointType));
		ITEM_FIELD(L"Need_Function", DecB(p->byNeedFunction));
		ITEM_FIELD(L"Use_Duration_Max", Dec(p->dwUseDurationMax));
		ITEM_FIELD(L"Duration_Type", DecB(p->byDurationType));
		ITEM_FIELD(L"Contents_Tblidx", Dec(p->contentsTblidx));
		ITEM_FIELD(L"Duration_Group", Dec(p->dwDurationGroup));
		ITEM_FIELD(L"Drop_Level", DecB(p->byDropLevel));
		ITEM_FIELD(L"Create_Enchant_Rate_Tblidx", Dec(p->enchantRateTblidx));
		ITEM_FIELD(L"Excellent_Tblidx", Dec(p->excellentTblidx));
		ITEM_FIELD(L"Rare_Tblidx", Dec(p->rareTblidx));
		ITEM_FIELD(L"Legendary_Tblidx", Dec(p->legendaryTblidx));
		ITEM_FIELD(L"Create_Superior_Able", Bl(p->bCreateSuperiorAble));
		ITEM_FIELD(L"Create_Excellent_Able", Bl(p->bCreateExcellentAble));
		ITEM_FIELD(L"Create_Rare_Able", Bl(p->bCreateRareAble));
		ITEM_FIELD(L"Create_Legendary_Able", Bl(p->bCreateLegendaryAble));
		ITEM_FIELD(L"Restrict_Type", DecB(p->byRestrictType));
		ITEM_FIELD(L"fAtk_Phy", Flt(p->fAttack_Physical_Revision));
		ITEM_FIELD(L"fAtk_Eng", Flt(p->fAttack_Energy_Revision));
		ITEM_FIELD(L"fDef_Phy", Flt(p->fDefence_Physical_Revision));
		ITEM_FIELD(L"fDef_Eng", Flt(p->fDefence_Energy_Revision));
		ITEM_FIELD(L"TMP_Category_Type", DecB(p->byTmpTabType));
		ITEM_FIELD(L"Can_Renewal", Bl(p->bIsCanRenewal));
		ITEM_FIELD(L"Disassamble_Bit_Flag", Hex(p->wDisassemble_Bit_Flag));
		ITEM_FIELD(L"Normal_Min", DecB(p->byDisassembleNormalMin));
		ITEM_FIELD(L"Normal_Max", DecB(p->byDisassembleNormalMax));
		ITEM_FIELD(L"Rank_Up_Min", DecB(p->byDisassembleUpperMin));
		ITEM_FIELD(L"Rank_Up_Max", DecB(p->byDisassembleUpperMax));
		ITEM_FIELD(L"Drop_Visual", DecB(p->byDropVisual));
	}
	return m;
}
#undef ITEM_FIELD

#define NEWBIE_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sNEWBIE_TBLDAT* p = (sNEWBIE_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetNewbieExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		NEWBIE_FIELD(L"Tblidx", Dec(p->tblidx));
		NEWBIE_FIELD(L"Race", DecB(p->byRace));
		NEWBIE_FIELD(L"Class", DecB(p->byClass));
		NEWBIE_FIELD(L"World_Id", Dec(p->world_Id));
		NEWBIE_FIELD(L"Tutorial_World_Tblidx", Dec(p->tutorialWorld));
		NEWBIE_FIELD(L"Spawn_Loc_X", Flt(p->vSpawn_Loc.x));
		NEWBIE_FIELD(L"Spawn_Loc_Y", Flt(p->vSpawn_Loc.y));
		NEWBIE_FIELD(L"Spawn_Loc_Z", Flt(p->vSpawn_Loc.z));
		NEWBIE_FIELD(L"Spawn_Dir_X", Flt(p->vSpawn_Dir.x));
		NEWBIE_FIELD(L"Spawn_Dir_Z", Flt(p->vSpawn_Dir.z));
		NEWBIE_FIELD(L"Bind_Loc_X", Flt(p->vBind_Loc.x));
		NEWBIE_FIELD(L"Bind_Loc_Y", Flt(p->vBind_Loc.y));
		NEWBIE_FIELD(L"Bind_Loc_Z", Flt(p->vBind_Loc.z));
		NEWBIE_FIELD(L"Bind_Dir_X", Flt(p->vBind_Dir.x));
		NEWBIE_FIELD(L"Bind_Dir_Z", Flt(p->vBind_Dir.z));
		NEWBIE_FIELD(L"Map_Name_Tblidx", Dec(p->mapNameTblidx));
		NEWBIE_FIELD(L"QItem_Tblidx_1", Dec(p->qItemTblidx1));
		NEWBIE_FIELD(L"QPosition_1", DecB(p->byQPosition1));
		NEWBIE_FIELD(L"QStack_Quantity_1", DecB(p->byQStackQuantity1));

		for (int i = 0; i < NTL_MAX_NEWBIE_ITEM; ++i)
		{
			CStringW key; key.Format(L"Item_Tblidx_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sNEWBIE_TBLDAT* p = (sNEWBIE_TBLDAT*)pRow; return Dec(p->aitem_Tblidx[i]); };
		}
		for (int i = 0; i < NTL_MAX_NEWBIE_ITEM; ++i)
		{
			CStringW key; key.Format(L"Position_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sNEWBIE_TBLDAT* p = (sNEWBIE_TBLDAT*)pRow; return DecB(p->abyPos[i]); };
		}
		for (int i = 0; i < NTL_MAX_NEWBIE_ITEM; ++i)
		{
			CStringW key; key.Format(L"Stack_Quantity_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sNEWBIE_TBLDAT* p = (sNEWBIE_TBLDAT*)pRow; return DecB(p->abyStack_Quantity[i]); };
		}
		for (int i = 0; i < NTL_MAX_NEWBIE_SKILL; ++i)
		{
			CStringW key; key.Format(L"Skill_Tblidx_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sNEWBIE_TBLDAT* p = (sNEWBIE_TBLDAT*)pRow; return Dec(p->aSkillTblidx[i]); };
		}
		for (int i = 0; i < NTL_MAX_NEWBIE_QUICKSLOT_COUNT; ++i)
		{
			CStringW key; key.Format(L"Quick_Tblidx%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sNEWBIE_TBLDAT* p = (sNEWBIE_TBLDAT*)pRow; return Dec(p->asQuickData[i].tbilidx); };
		}
		for (int i = 0; i < NTL_MAX_NEWBIE_QUICKSLOT_COUNT; ++i)
		{
			CStringW key; key.Format(L"Quick_Type%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sNEWBIE_TBLDAT* p = (sNEWBIE_TBLDAT*)pRow; return DecB(p->asQuickData[i].byType); };
		}
		for (int i = 0; i < NTL_MAX_NEWBIE_QUICKSLOT_COUNT; ++i)
		{
			CStringW key; key.Format(L"Quick_Position%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sNEWBIE_TBLDAT* p = (sNEWBIE_TBLDAT*)pRow; return DecB(p->asQuickData[i].byQuickSlot); };
		}
	}
	return m;
}
#undef NEWBIE_FIELD


// MascotGrade/MascotStatus/Mascot/ItemEnchant: verified against the real
// SetTableData for each of these -- every one of them only recognizes a
// "Tblidx" column and errors out ("Unknown field name found!") on
// anything else. Only Tblidx is exposed here on purpose; adding more
// fields to these extractor maps without the engine's readers also
// gaining matching branches would produce an XML file the engine can't
// actually re-import.

#define MASCOT_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sMASCOT_TBLDAT* p = (sMASCOT_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetMascotExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		MASCOT_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef MASCOT_FIELD

#define MASCOT_GRADE_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sMASCOT_GRADE_TBLDAT* p = (sMASCOT_GRADE_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetMascotGradeExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		MASCOT_GRADE_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef MASCOT_GRADE_FIELD

#define MASCOT_STATUS_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sMASCOT_STATUS_TBLDAT* p = (sMASCOT_STATUS_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetMascotStatusExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		MASCOT_STATUS_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef MASCOT_STATUS_FIELD

#define ITEM_ENCHANT_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sITEM_ENCHANT_TBLDAT* p = (sITEM_ENCHANT_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetItemEnchantExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		ITEM_ENCHANT_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef ITEM_ENCHANT_FIELD

#define CHARTITLE_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sCHARTITLE_TBLDAT* p = (sCHARTITLE_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetCharTitleExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		CHARTITLE_FIELD(L"Tblidx", Dec(p->tblidx));
		CHARTITLE_FIELD(L"Name", Dec(p->tblNameIndex));
		CHARTITLE_FIELD(L"Contents_Type", DecB(p->byContentsType));
		CHARTITLE_FIELD(L"Direct_Type", DecB(p->byRepresentationType));
		CHARTITLE_FIELD(L"Bone_Name", WideToXml(p->wszBoneName));
		CHARTITLE_FIELD(L"Effect_Name", WideToXml(p->wszEffectName));
		CHARTITLE_FIELD(L"Effect_Sound", WideToXml(p->wszEffectSound));

		for (int i = 0; i < NTL_MAX_CHAR_TITLE_EFFECT; ++i)
		{
			CStringW key; key.Format(L"System_Effect_Tblidx%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sCHARTITLE_TBLDAT* p = (sCHARTITLE_TBLDAT*)pRow; return Dec(p->atblSystem_Effect_Index[i]); };
		}
		for (int i = 0; i < NTL_MAX_CHAR_TITLE_EFFECT; ++i)
		{
			CStringW key; key.Format(L"System_Effect_Type%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sCHARTITLE_TBLDAT* p = (sCHARTITLE_TBLDAT*)pRow; return DecB(p->abySystem_Effect_Type[i]); };
		}
		for (int i = 0; i < NTL_MAX_CHAR_TITLE_EFFECT; ++i)
		{
			CStringW key; key.Format(L"System_Effect_Value%d", i + 1);
			// abySystem_Effect_Value is a double in the struct, but the
			// engine's SetTableData reads this column with READ_BYTE (a
			// pre-existing bug, not ours to fix here) -- write it as a
			// plain truncated integer so it round-trips through that
			// actual parser instead of a format it doesn't expect.
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sCHARTITLE_TBLDAT* p = (sCHARTITLE_TBLDAT*)pRow; return DecB((BYTE)p->abySystem_Effect_Value[i]); };
		}
	}
	return m;
}
#undef CHARTITLE_FIELD

#define CHATFILTER_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sCHAT_FILTER_TBLDAT* p = (sCHAT_FILTER_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetChatFilterExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		// This table's Tblidx column is unusually named "Chatting_Filter_Tblidx",
		// not "Tblidx" like every other table -- verified against the real
		// SetTableData, not a guess.
		CHATFILTER_FIELD(L"Chatting_Filter_Tblidx", Dec(p->tblidx));
		CHATFILTER_FIELD(L"Slang_Text", WideToXml(p->wszSlangText));
		CHATFILTER_FIELD(L"Filtering_Text_Index", Dec(p->filteringTextIndex));
	}
	return m;
}
#undef CHATFILTER_FIELD

#define MERCHANT_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sMERCHANT_TBLDAT* p = (sMERCHANT_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetMerchantExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		MERCHANT_FIELD(L"Tblidx", Dec(p->tblidx));
		MERCHANT_FIELD(L"Name_Text", WideToXml(p->wszNameText));
		MERCHANT_FIELD(L"Sell_Type", DecB(p->bySell_Type));
		MERCHANT_FIELD(L"Tab_Name", Dec(p->Tab_Name));
		MERCHANT_FIELD(L"Need_Mileage", Dec(p->dwNeedMileage));

		for (int i = 0; i < NTL_MAX_MERCHANT_COUNT; ++i)
		{
			CStringW key; key.Format(L"Item_Tblidx_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sMERCHANT_TBLDAT* p = (sMERCHANT_TBLDAT*)pRow; return Dec(p->aitem_Tblidx[i]); };
		}
		for (int i = 0; i < NTL_MAX_MERCHANT_COUNT; ++i)
		{
			CStringW key; key.Format(L"Need_Item_Tindex_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sMERCHANT_TBLDAT* p = (sMERCHANT_TBLDAT*)pRow; return Dec(p->aNeedItemTblidx[i]); };
		}
		for (int i = 0; i < NTL_MAX_MERCHANT_COUNT; ++i)
		{
			CStringW key; key.Format(L"Need_Item_Stack_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sMERCHANT_TBLDAT* p = (sMERCHANT_TBLDAT*)pRow; return DecB(p->abyNeedItemStack[i]); };
		}
		for (int i = 0; i < NTL_MAX_MERCHANT_COUNT; ++i)
		{
			CStringW key; key.Format(L"Need_Zenny_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sMERCHANT_TBLDAT* p = (sMERCHANT_TBLDAT*)pRow; return Dec(p->adwNeedZenny[i]); };
		}
	}
	return m;
}
#undef MERCHANT_FIELD

#define FORMULA_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sFORMULA_TBLDAT* p = (sFORMULA_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetFormulaExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		// Tblidx's XML column is "Idx" here, not "Tblidx" -- verified
		// against the real SetTableData. bValidity_Able has no XML column
		// at all (see GetXmlExportCaveat).
		FORMULA_FIELD(L"Idx", Dec(p->tblidx));
		for (int i = 0; i < DBO_MAX_FORMULA_RATE_COUNT; ++i)
		{
			CStringW key; key.Format(L"Rate%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sFORMULA_TBLDAT* p = (sFORMULA_TBLDAT*)pRow; return Flt(p->afRate[i]); };
		}
	}
	return m;
}
#undef FORMULA_FIELD

#define ITEM_MIX_EXP_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sITEM_MIX_EXP_TBLDAT* p = (sITEM_MIX_EXP_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetItemMixExpExtractors()
{
	// Only Tblidx is recognized by this table's SetTableData -- see
	// GetXmlExportCaveat.
	static FieldExtractorMap m;
	if (m.empty())
	{
		ITEM_MIX_EXP_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef ITEM_MIX_EXP_FIELD

#define CHATCMD_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sCHAT_COMMAND_TBLDAT* p = (sCHAT_COMMAND_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetChatCommandExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		CHATCMD_FIELD(L"Tblidx", Dec(p->tblidx));
		CHATCMD_FIELD(L"Validity_Able", Bl(p->bValidity_Able));
		CHATCMD_FIELD(L"Action_Animation_Index", DecW(p->wAction_Animation_Index));

		for (int i = 0; i < NTL_MAX_CHAT_COMMAND; ++i)
		{
			CStringW key; key.Format(L"Chat_Command_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sCHAT_COMMAND_TBLDAT* p = (sCHAT_COMMAND_TBLDAT*)pRow; return Dec(p->aChat_Command[i]); };
		}
	}
	return m;
}
#undef CHATCMD_FIELD




#define VEHICLE_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sVEHICLE_TBLDAT* p = (sVEHICLE_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetVehicleExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		VEHICLE_FIELD(L"Tblidx", Dec(p->tblidx));
		VEHICLE_FIELD(L"Model_Name", AnsiToXml(p->szModelName));
		VEHICLE_FIELD(L"SRP_Type", DecB(p->bySRPType));
		VEHICLE_FIELD(L"Speed", DecB(p->bySpeed));
		VEHICLE_FIELD(L"Vehicle_Type", DecB(p->byVehicleType));
		VEHICLE_FIELD(L"Run_Height", DecW(p->wRunHeight));
		VEHICLE_FIELD(L"Personnel", DecB(p->byPersonnel));
	}
	return m;
}
#undef VEHICLE_FIELD

#define DUNGEON_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sDUNGEON_TBLDAT* p = (sDUNGEON_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetDungeonExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		DUNGEON_FIELD(L"Tblidx", Dec(p->tblidx));
		DUNGEON_FIELD(L"Dungeon_Type", DecB(p->byDungeonType));
		DUNGEON_FIELD(L"Max_Member", DecB(p->byMaxMember));
		DUNGEON_FIELD(L"Link_World", Dec(p->linkWorld));
		DUNGEON_FIELD(L"Min_Level", DecB(p->byMinLevel));
		DUNGEON_FIELD(L"Max_Level", DecB(p->byMaxLevel));
		DUNGEON_FIELD(L"Need_Item", Dec(p->needItemTblidx));
		DUNGEON_FIELD(L"Honor_Point", Dec(p->dwHonorPoint));
		DUNGEON_FIELD(L"Wps_Tblidx", Dec(p->wpsTblidx));
		DUNGEON_FIELD(L"Open_Cine", Dec(p->openCine));
		DUNGEON_FIELD(L"Group_Index", Dec(p->groupIdx));
	}
	return m;
}
#undef DUNGEON_FIELD


#define DBREWARD_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sDRAGONBALL_REWARD_TBLDAT* p = (sDRAGONBALL_REWARD_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetDragonBallRewardExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		DBREWARD_FIELD(L"Tblidx", Dec(p->tblidx));
		DBREWARD_FIELD(L"Ball_Type", DecB(p->byBallType));
		DBREWARD_FIELD(L"Reward_Category_Depth", DecB(p->byRewardCategoryDepth));
		// Genuinely lowercase in the real reader, unlike every neighboring
		// column -- verified against source, not a typo here.
		DBREWARD_FIELD(L"class_bit", Dec(p->dwClassBit));
		DBREWARD_FIELD(L"Reward_Category_Name", Dec(p->rewardCategoryName));
		DBREWARD_FIELD(L"Reward_Category_Dialog", Dec(p->rewardCategoryDialog));
		DBREWARD_FIELD(L"Reward_Type", DecB(p->byRewardType));
		DBREWARD_FIELD(L"Reward_Name", Dec(p->rewardName));
		DBREWARD_FIELD(L"Reward_Link_Tblidx", Dec(p->rewardLinkTblidx));
		DBREWARD_FIELD(L"Reward_Zenny", Dec(p->dwRewardZenny));
		DBREWARD_FIELD(L"Reward_Dialog_1", Dec(p->rewardDialog1));
		DBREWARD_FIELD(L"Reward_Dialog_2", Dec(p->rewardDialog2));
	}
	return m;
}
#undef DBREWARD_FIELD

#define EXP_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sEXP_TBLDAT* p = (sEXP_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetExpExtractors()
{
	// The 10 individual/team rank-point fields have no XML column at all
	// -- see GetXmlExportCaveat.
	static FieldExtractorMap m;
	if (m.empty())
	{
		EXP_FIELD(L"Level", Dec(p->tblidx));
		EXP_FIELD(L"EXP", Dec(p->dwExp));
		EXP_FIELD(L"Need_EXP", Dec(p->dwNeed_Exp));
		EXP_FIELD(L"Normal_Race", DecW(p->wNormal_Race));
		EXP_FIELD(L"Super_Race", DecW(p->wSuperRace));
		EXP_FIELD(L"Mob_Exp", Dec(p->dwMobExp));
		EXP_FIELD(L"Phy_Defence_Ref", Dec(p->dwPhyDefenceRef));
		EXP_FIELD(L"Eng_Defence_Ref", Dec(p->dwEngDefenceRef));
		EXP_FIELD(L"Mob_Zenny", Dec(p->dwMobZenny));
	}
	return m;
}
#undef EXP_FIELD


#define CHARM_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sCHARM_TBLDAT* p = (sCHARM_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetCharmExtractors()
{
	// dwKeep_Time_In_Millisecs is derived (wKeep_Time * 1000) inline in the
	// reader, no XML column of its own -- not exported separately.
	static FieldExtractorMap m;
	if (m.empty())
	{
		CHARM_FIELD(L"Tblidx", Dec(p->tblidx));
		CHARM_FIELD(L"Drop_Rate", DecW(p->wDrop_Rate));
		CHARM_FIELD(L"EXP", DecW(p->wEXP));
		CHARM_FIELD(L"RP_Sharing", DecW(p->wRP_Sharing));
		CHARM_FIELD(L"Cool_Time", DecW(p->wCool_Time));
		CHARM_FIELD(L"Keep_Time", DecW(p->wKeep_Time));
		CHARM_FIELD(L"Need_Zenny", Dec(p->dwNeed_Zenny));
		CHARM_FIELD(L"Dice_Min", DecB(p->byDice_Min));
		CHARM_FIELD(L"Dice_Max", DecB(p->byDice_Max));
		CHARM_FIELD(L"Charm_Type_Bit_Flag", DecB(p->byCharm_Type_Bit_Flag));
	}
	return m;
}
#undef CHARM_FIELD

#define ACTION_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sACTION_TBLDAT* p = (sACTION_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetActionExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		ACTION_FIELD(L"Tblidx", Dec(p->tblidx));
		ACTION_FIELD(L"Validity_Able", Bl(p->bValidity_Able));
		ACTION_FIELD(L"Action_Type", DecB(p->byAction_Type));
		ACTION_FIELD(L"Action_Name", Dec(p->Action_Name));
		ACTION_FIELD(L"Icon_Name", AnsiToXml(p->szIcon_Name));
		ACTION_FIELD(L"Note", Dec(p->Note));
		ACTION_FIELD(L"Chat_Command_Index", Dec(p->chat_Command_Index));
		ACTION_FIELD(L"ETC_Action_Type", DecB(p->byETC_Action_Type));
	}
	return m;
}
#undef ACTION_FIELD



#define HELP_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sHELP_TBLDAT* p = (sHELP_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetHelpExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		HELP_FIELD(L"Help_Tblidx", Dec(p->tblidx));
		HELP_FIELD(L"Category", DecB(p->byCategory));
		HELP_FIELD(L"Help_Title", Dec(p->dwHelpTitle));
		HELP_FIELD(L"Popo_Hint", Dec(p->dwPopoHint));
		HELP_FIELD(L"Help_HTML_Name", WideToXml(p->wszHelpHTMLName));
		HELP_FIELD(L"Condition_Check", DecB(p->byConditionCheck));
	}
	return m;
}
#undef HELP_FIELD

#define GUIDEHINT_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sGUIDE_HINT_TBLDAT* p = (sGUIDE_HINT_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetGuideHintExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		GUIDEHINT_FIELD(L"Tblidx", Dec(p->tblidx));
		GUIDEHINT_FIELD(L"Type", DecB(p->byType));
		GUIDEHINT_FIELD(L"WidthPosition", DecB(p->byWidthPosition));
		GUIDEHINT_FIELD(L"HeightPosition", DecB(p->byHeightPosition));
		GUIDEHINT_FIELD(L"WidthSize", DecW(p->wWidthSize));
		GUIDEHINT_FIELD(L"HeightSize", DecW(p->wHeightSize));
		GUIDEHINT_FIELD(L"Resource", AnsiToXml(p->szResource));
		GUIDEHINT_FIELD(L"Note", AnsiToXml(p->szNote));
		GUIDEHINT_FIELD(L"Auto_Show", Bl(p->bAutoShow));
	}
	return m;
}
#undef GUIDEHINT_FIELD






#define DYNOBJ_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sDYNAMIC_OBJECT_TBLDAT* p = (sDYNAMIC_OBJECT_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetDynamicObjectExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		DYNOBJ_FIELD(L"Tblidx", Dec(p->tblidx));
		DYNOBJ_FIELD(L"Validity_Able", Bl(p->bValidityAble));
		DYNOBJ_FIELD(L"Type", DecB(p->byType));
		DYNOBJ_FIELD(L"Model_Name", AnsiToXml(p->szModelName));
		DYNOBJ_FIELD(L"State_Type", DecB(p->byStateType));
		DYNOBJ_FIELD(L"Spawn_Animation", Dec(p->spawnAnimation));
		DYNOBJ_FIELD(L"Idle_Animation", Dec(p->idleAnimation));
		DYNOBJ_FIELD(L"Despawn_Animation", Dec(p->despawnAnimation));
		DYNOBJ_FIELD(L"State1_Animation", Dec(p->state1Animation));
		DYNOBJ_FIELD(L"State2_Animation", Dec(p->state2Animation));
		DYNOBJ_FIELD(L"Boundary_Distance", DecB(p->byBoundaryDistance));
		DYNOBJ_FIELD(L"Despawn_Distance", DecB(p->byDespawnDistance));
	}
	return m;
}
#undef DYNOBJ_FIELD

#define USEITEM_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sUSE_ITEM_TBLDAT* p = (sUSE_ITEM_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetUseItemExtractors()
{
	// dwCastingTimeInMilliSecs/dwCoolTimeInMilliSecs/dwKeepTimeInMilliSecs
	// and fUse_Range_Min/fUse_Range_Max are derived by SetTableData from a
	// sibling column (Casting_Time*1000, Cool_Time*1000, Keep_Time*1000,
	// (float)Use_Range_Min, (float)Use_Range_Max) -- they have no XML
	// column of their own, so no extractor entry for them.
	static FieldExtractorMap m;
	if (m.empty())
	{
		USEITEM_FIELD(L"Tblidx", Dec(p->tblidx));
		USEITEM_FIELD(L"Use_Item_Active_Type", DecB(p->byUse_Item_Active_Type));
		USEITEM_FIELD(L"Buff_Group", DecB(p->byBuff_Group));
		USEITEM_FIELD(L"Buff_Keep_Type", DecB(p->byBuffKeepType));
		USEITEM_FIELD(L"Cool_Time_Bit_Flag", Hex(p->dwCool_Time_Bit_Flag));
		USEITEM_FIELD(L"Function_Bit_Flag", Hex(p->wFunction_Bit_Flag));
		USEITEM_FIELD(L"Use_Restriction_Rule_Bit_Flag", Hex(p->dwUse_Restriction_Rule_Bit_Flag));
		USEITEM_FIELD(L"Use_Allow_Rule_Bit_Flag", Hex(p->dwUse_Allow_Rule_Bit_Flag));
		USEITEM_FIELD(L"Appoint_Target", DecB(p->byAppoint_Target));
		USEITEM_FIELD(L"Apply_Target", DecB(p->byApply_Target));
		USEITEM_FIELD(L"Apply_Target_Index", Dec(p->dwApply_Target_Index));
		USEITEM_FIELD(L"Apply_Target_Max", DecB(p->byApply_Target_Max));
		USEITEM_FIELD(L"Apply_Range", DecB(p->byApply_Range));
		USEITEM_FIELD(L"Apply_Area_Size_1", DecB(p->byApply_Area_Size_1));
		USEITEM_FIELD(L"Apply_Area_Size_2", DecB(p->byApply_Area_Size_2));
		USEITEM_FIELD(L"Need_State_Bit_Flag", Hex(p->wNeed_State_Bit_Flag));

		for (int i = 0; i < (int)NTL_MAX_EFFECT_IN_ITEM; ++i)
		{
			CStringW key;

			key.Format(L"System_Effect_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sUSE_ITEM_TBLDAT* p = (sUSE_ITEM_TBLDAT*)pRow; return Dec(p->aSystem_Effect[i]); };

			key.Format(L"System_Effect_Type_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sUSE_ITEM_TBLDAT* p = (sUSE_ITEM_TBLDAT*)pRow; return DecB(p->abySystem_Effect_Type[i]); };

			key.Format(L"System_Effect_Value_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sUSE_ITEM_TBLDAT* p = (sUSE_ITEM_TBLDAT*)pRow; return Dbl(p->aSystem_Effect_Value[i]); };
		}

		USEITEM_FIELD(L"Require_LP", Dec(p->dwRequire_LP));
		USEITEM_FIELD(L"Require_EP", DecW(p->wRequire_EP));
		USEITEM_FIELD(L"Require_RP_Ball", DecB(p->byRequire_RP_Ball));
		USEITEM_FIELD(L"Casting_Time", Flt(p->fCasting_Time));
		USEITEM_FIELD(L"Cool_Time", Dec(p->dwCool_Time));
		USEITEM_FIELD(L"Keep_Time", Dec(p->dwKeep_Time));
		USEITEM_FIELD(L"Keep_Effect", Bl(p->bKeep_Effect));
		USEITEM_FIELD(L"Use_Range_Min", DecB(p->byUse_Range_Min));
		USEITEM_FIELD(L"Use_Range_Max", DecB(p->byUse_Range_Max));
		USEITEM_FIELD(L"Use_Info_Text", Dec(p->Use_Info_Text));
		USEITEM_FIELD(L"Casting_Effect", AnsiToXml(p->szCasting_Effect));
		USEITEM_FIELD(L"Action_Effect", AnsiToXml(p->szAction_Effect));
		USEITEM_FIELD(L"Casting_Animation_Start", DecW(p->wCasting_Animation_Start));
		USEITEM_FIELD(L"Casting_Animation_Loop", DecW(p->wCasting_Animation_Loop));
		USEITEM_FIELD(L"Action_Animation_Index", DecW(p->wAction_Animation_Index));
		USEITEM_FIELD(L"Action_Loop_Animation_Index", DecW(p->wAction_Loop_Animation_Index));
		USEITEM_FIELD(L"Action_End_Animation_Index", DecW(p->wAction_End_Animation_Index));
		USEITEM_FIELD(L"Casting_Effect_Position", DecB(p->byCastingEffectPosition));
		USEITEM_FIELD(L"Action_Effect_Position", DecB(p->byActionEffectPosition));
		USEITEM_FIELD(L"UseWorld_Index", Dec(p->useWorldTblidx));
		USEITEM_FIELD(L"UseLoc_X", Flt(p->fUseLoc_X));
		USEITEM_FIELD(L"UseLoc_Z", Flt(p->fUseLoc_Z));
		USEITEM_FIELD(L"UseLoc_Radius", Flt(p->fUseLoc_Radius));
		USEITEM_FIELD(L"RequiredQuestID", DecW(p->RequiredQuestID));
	}
	return m;
}
#undef USEITEM_FIELD

#define SETITEM_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sSET_ITEM_TBLDAT* p = (sSET_ITEM_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetSetItemExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		SETITEM_FIELD(L"Tblidx", Dec(p->tblidx));
		SETITEM_FIELD(L"Validity_Able", Bl(p->bValidity_Able));
		SETITEM_FIELD(L"Semi_Set_Option", Dec(p->semiSetOption));
		SETITEM_FIELD(L"Full_Set_Option", Dec(p->fullSetOption));

		for (int i = 0; i < NTL_MAX_SET_ITEM_COUNT; ++i)
		{
			CStringW key; key.Format(L"Item_Tblidx_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sSET_ITEM_TBLDAT* p = (sSET_ITEM_TBLDAT*)pRow; return Dec(p->aItemTblidx[i]); };
		}
	}
	return m;
}
#undef SETITEM_FIELD







#define QUESTREWARD_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sQUEST_REWARD_TBLDAT* p = (sQUEST_REWARD_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetQuestRewardExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		QUESTREWARD_FIELD(L"Tblidx", Dec(p->tblidx));
		QUESTREWARD_FIELD(L"Def_Reward_EXP", Dec(p->dwDef_Reward_EXP));
		QUESTREWARD_FIELD(L"Def_Reward_Zeny", Dec(p->dwDef_Reward_Zeny));

		for (int i = 0; i < QUEST_REWARD_DEF_MAX_CNT; ++i)
		{
			CStringW key;

			key.Format(L"Def_Reward_Type_%d", i);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_REWARD_TBLDAT* p = (sQUEST_REWARD_TBLDAT*)pRow; return DecB(p->arsDefRwd[i].byRewardType); };

			key.Format(L"Def_Reward_Idx_%d", i);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_REWARD_TBLDAT* p = (sQUEST_REWARD_TBLDAT*)pRow; return Dec(p->arsDefRwd[i].dwRewardIdx); };

			key.Format(L"Def_Reward_Val_%d", i);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_REWARD_TBLDAT* p = (sQUEST_REWARD_TBLDAT*)pRow; return Dec(p->arsDefRwd[i].dwRewardVal); };
		}

		for (int i = 0; i < QUEST_REWARD_SEL_MAX_CNT; ++i)
		{
			CStringW key;

			key.Format(L"Sel_Reward_Type_%d", i);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_REWARD_TBLDAT* p = (sQUEST_REWARD_TBLDAT*)pRow; return DecB(p->arsSelRwd[i].byRewardType); };

			key.Format(L"Sel_Reward_Idx_%d", i);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_REWARD_TBLDAT* p = (sQUEST_REWARD_TBLDAT*)pRow; return Dec(p->arsSelRwd[i].dwRewardIdx); };

			key.Format(L"Sel_Reward_Val_%d", i);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_REWARD_TBLDAT* p = (sQUEST_REWARD_TBLDAT*)pRow; return Dec(p->arsSelRwd[i].dwRewardVal); };
		}
	}
	return m;
}
#undef QUESTREWARD_FIELD

#define QUESTREWARDSEL_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sQUEST_REWARD_SELECT_TBLDAT* p = (sQUEST_REWARD_SELECT_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetQuestRewardSelectExtractors()
{
	// Only Tblidx is recognized by this table's SetTableData -- see
	// GetXmlExportCaveat.
	static FieldExtractorMap m;
	if (m.empty())
	{
		QUESTREWARDSEL_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef QUESTREWARDSEL_FIELD




#define NPCSERVER_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sNPC_SERVER_TBLDAT* p = (sNPC_SERVER_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetNpcServerExtractors()
{
	// Only Tblidx is recognized by this table's SetTableData -- see
	// GetXmlExportCaveat.
	static FieldExtractorMap m;
	if (m.empty())
	{
		NPCSERVER_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef NPCSERVER_FIELD

#define WORLD_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sWORLD_TBLDAT* p = (sWORLD_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetWorldExtractors()
{
	// dwDynamicCreateCountShareGroup, vBattleStart2Loc, vBattleEnd2Loc, both
	// vWaitingPoint*, and dwProhibition_Bit_Flag have no XML column. Field
	// "Can_Fly" is recognized but its assignment is commented out (dead --
	// there's also no bCanFly struct member anymore). outWorldDir.z can't be
	// set independently: both "Out_Field_Dir_X" and "Out_Field_Dir_Z" write
	// to outWorldDir.x (copy/paste bug in the reader), so we only export
	// "Out_Field_Dir_X" and leave outWorldDir.z alone. See GetXmlExportCaveat.
	static FieldExtractorMap m;
	if (m.empty())
	{
		WORLD_FIELD(L"Tblidx", Dec(p->tblidx));
		WORLD_FIELD(L"Name", WideToXml(p->wszName));
		WORLD_FIELD(L"Dynamic_Able", Bl(p->bDynamic));
		WORLD_FIELD(L"Dynamic_Create_Count", Dec((DWORD)p->nCreateCount));
		WORLD_FIELD(L"Field_Door_Type", DecB(p->byDoorType));
		WORLD_FIELD(L"Field_Destory_Time", Dec(p->dwDestroyTimeInMilliSec / 1000));
		WORLD_FIELD(L"Mob_Spawn_Table_Name", WideToXml(p->wszMobSpawn_Table_Name));
		WORLD_FIELD(L"NPC_Spawn_Table_Name", WideToXml(p->wszNpcSpawn_Table_Name));
		WORLD_FIELD(L"Object_Spawn_Table_Name", WideToXml(p->wszObjSpawn_Table_Name));
		WORLD_FIELD(L"Field_Start_Loc_X", Flt(p->vStart.x));
		WORLD_FIELD(L"Field_Start_Loc_Z", Flt(p->vStart.z));
		WORLD_FIELD(L"Field_End_Loc_X", Flt(p->vEnd.x));
		WORLD_FIELD(L"Field_End_Loc_Z", Flt(p->vEnd.z));
		WORLD_FIELD(L"Battle_Start_X", Flt(p->vBattleStartLoc.x));
		WORLD_FIELD(L"Battle_Start_Z", Flt(p->vBattleStartLoc.z));
		WORLD_FIELD(L"Battle_End_X", Flt(p->vBattleEndLoc.x));
		WORLD_FIELD(L"Battle_End_Z", Flt(p->vBattleEndLoc.z));
		WORLD_FIELD(L"OutSide_Battle_Start_X", Flt(p->vOutSideBattleStartLoc.x));
		WORLD_FIELD(L"OutSide_Battle_Start_Z", Flt(p->vOutSideBattleStartLoc.z));
		WORLD_FIELD(L"OutSide_Battle_End_X", Flt(p->vOutSideBattleEndLoc.x));
		WORLD_FIELD(L"OutSide_Battle_End_Z", Flt(p->vOutSideBattleEndLoc.z));
		WORLD_FIELD(L"Spectator_Start_X", Flt(p->vSpectatorStartLoc.x));
		WORLD_FIELD(L"Spectator_Start_Z", Flt(p->vSpectatorStartLoc.z));
		WORLD_FIELD(L"Spectator_End_X", Flt(p->vSpectatorEndLoc.x));
		WORLD_FIELD(L"Spectator_End_Z", Flt(p->vSpectatorEndLoc.z));
		WORLD_FIELD(L"Default_Loc_X", Flt(p->vDefaultLoc.x));
		WORLD_FIELD(L"Default_Loc_Y", Flt(p->vDefaultLoc.y));
		WORLD_FIELD(L"Default_Loc_Z", Flt(p->vDefaultLoc.z));
		WORLD_FIELD(L"Default_Dir_X", Flt(p->vDefaultDir.x));
		WORLD_FIELD(L"Default_Dri_Y", Flt(p->vDefaultDir.y));
		WORLD_FIELD(L"Default_Dir_Z", Flt(p->vDefaultDir.z));
		WORLD_FIELD(L"Start1_Point_Loc_X", Flt(p->vStart1Loc.x));
		WORLD_FIELD(L"Start1_Point_Loc_Y", Flt(p->vStart1Loc.y));
		WORLD_FIELD(L"Start1_Point_Loc_Z", Flt(p->vStart1Loc.z));
		WORLD_FIELD(L"Start1_Point_Dir_X", Flt(p->vStart1Dir.x));
		WORLD_FIELD(L"Start1_Point_Dir_Z", Flt(p->vStart1Dir.z));
		WORLD_FIELD(L"Start2_Point_Loc_X", Flt(p->vStart2Loc.x));
		WORLD_FIELD(L"Start2_Point_Loc_Y", Flt(p->vStart2Loc.y));
		WORLD_FIELD(L"Start2_Point_Loc_Z", Flt(p->vStart2Loc.z));
		WORLD_FIELD(L"Start2_Point_Dir_X", Flt(p->vStart2Dir.x));
		WORLD_FIELD(L"Start2_Point_Dir_Z", Flt(p->vStart2Dir.z));
		WORLD_FIELD(L"Standard_Loc_X", Flt(p->vStandardLoc.x));
		WORLD_FIELD(L"Standard_Loc_Z", Flt(p->vStandardLoc.z));
		WORLD_FIELD(L"Split_Size", Flt(p->fSplitSize));
		WORLD_FIELD(L"Night_Able", Bl(p->bNight_Able));
		WORLD_FIELD(L"Static_Time", DecB(p->byStatic_Time));
		WORLD_FIELD(L"funcflag", Hex(p->wFuncFlag));
		WORLD_FIELD(L"World_Rule_Type", DecB(p->byWorldRuleType));
		WORLD_FIELD(L"World_Rule_Index", Dec(p->worldRuleTbldx));
		WORLD_FIELD(L"OutField_Tblidx", Dec(p->outWorldTblidx));
		WORLD_FIELD(L"Out_Field_Loc_X", Flt(p->outWorldLoc.x));
		WORLD_FIELD(L"Out_Field_Loc_Z", Flt(p->outWorldLoc.z));
		WORLD_FIELD(L"Out_Field_Dir_X", Flt(p->outWorldDir.x));
		WORLD_FIELD(L"ResourceFolder", WideToXml(p->wszResourceFolder));
		WORLD_FIELD(L"BGM_Rest_Time", Flt(p->fBGMRestTime));
		WORLD_FIELD(L"World_Resource_ID", Dec(p->dwWorldResourceID));
		WORLD_FIELD(L"FreeCamera_Height", Flt(p->fFreeCamera_Height));
		WORLD_FIELD(L"Enter_Resource_Flash", WideToXml(p->wszEnterResourceFlash));
		WORLD_FIELD(L"Leave_Resource_Flash", WideToXml(p->wszLeaveResourceFlash));
		WORLD_FIELD(L"WPS_Link_Index", Dec(p->wpsLinkIndex));
		WORLD_FIELD(L"Start_Point_Range", DecB(p->byStartPointRange));

		for (int i = 0; i < (int)DBO_MAX_WORLD_DRAGONBALLDROP; ++i)
		{
			CStringW key;

			key.Format(L"dragonball_have_rate_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sWORLD_TBLDAT* p = (sWORLD_TBLDAT*)pRow; return DecB(p->abyDragonBallHaveRate[i]); };

			key.Format(L"dragonball_drop_rate_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sWORLD_TBLDAT* p = (sWORLD_TBLDAT*)pRow; return DecB(p->abyDragonBallDropRate[i]); };
		}
	}
	return m;
}
#undef WORLD_FIELD


#define SYSEFFECT_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sSYSTEM_EFFECT_TBLDAT* p = (sSYSTEM_EFFECT_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetSystemEffectExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		SYSEFFECT_FIELD(L"Tblidx", Dec(p->tblidx));
		SYSEFFECT_FIELD(L"Name", WideToXml(p->wszName));
		SYSEFFECT_FIELD(L"Effect_Type", DecB(p->byEffect_Type));
		SYSEFFECT_FIELD(L"Active_Effect_Type", DecB(p->byActive_Effect_Type));
		SYSEFFECT_FIELD(L"Effect_Info_Text", Dec(p->Effect_Info_Text));
		SYSEFFECT_FIELD(L"Keep_Effect_Name", Dec(p->Keep_Effect_Name));
		SYSEFFECT_FIELD(L"Target_Effect_Position", DecB(p->byTarget_Effect_Position));
		SYSEFFECT_FIELD(L"Success_Effect_Name", AnsiToXml(p->szSuccess_Effect_Name));
		SYSEFFECT_FIELD(L"Success_Projectile_Type", DecB(p->bySuccess_Projectile_Type));
		SYSEFFECT_FIELD(L"Success_Effect_Position", DecB(p->bySuccess_Effect_Position));
		SYSEFFECT_FIELD(L"Success_End_Effect_Name", AnsiToXml(p->szSuccess_End_Effect_Name));
		SYSEFFECT_FIELD(L"End_Effect_Position", DecB(p->byEnd_Effect_Position));
		SYSEFFECT_FIELD(L"Keep_Animation_Index", DecW(p->wKeep_Animation_Index));
	}
	return m;
}
#undef SYSEFFECT_FIELD

#define ITEMOPTION_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sITEM_OPTION_TBLDAT* p = (sITEM_OPTION_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetItemOptionExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		ITEMOPTION_FIELD(L"Tblidx", Dec(p->tblidx));
		ITEMOPTION_FIELD(L"Option_Name", WideToXml(p->wszOption_Name));
		ITEMOPTION_FIELD(L"Validity_Able", Bl(p->bValidity_Able));
		ITEMOPTION_FIELD(L"Option_Rank", DecB(p->byOption_Rank));
		ITEMOPTION_FIELD(L"Item_Group", DecB(p->byItem_Group));
		ITEMOPTION_FIELD(L"Max_Quality", DecB(p->byMaxQuality));
		ITEMOPTION_FIELD(L"Quality", DecB(p->byQuality));
		ITEMOPTION_FIELD(L"Quality_Index", DecB(p->byQualityIndex));
		ITEMOPTION_FIELD(L"Cost", Dec(p->dwCost));
		ITEMOPTION_FIELD(L"Level", DecB(p->byLevel));

		for (int i = 0; i < NTL_MAX_SYSTEM_EFFECT_COUNT; ++i)
		{
			CStringW key;

			key.Format(L"System_Effect_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sITEM_OPTION_TBLDAT* p = (sITEM_OPTION_TBLDAT*)pRow; return Dec(p->system_Effect[i]); };

			key.Format(L"Type_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sITEM_OPTION_TBLDAT* p = (sITEM_OPTION_TBLDAT*)pRow; return Bl(p->bAppliedInPercent[i]); };

			key.Format(L"Value_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sITEM_OPTION_TBLDAT* p = (sITEM_OPTION_TBLDAT*)pRow; return Dec((DWORD)p->nValue[i]); };
		}

		ITEMOPTION_FIELD(L"Active_Effect", Dec(p->activeEffect));
		ITEMOPTION_FIELD(L"Active_Rate", Flt(p->fActiveRate));
		ITEMOPTION_FIELD(L"Note", AnsiToXml(p->szNote));

		for (int i = 0; i < NTL_MAX_SYSTEM_EFFECT_COUNT; ++i)
		{
			CStringW key; key.Format(L"Scouter_Info_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sITEM_OPTION_TBLDAT* p = (sITEM_OPTION_TBLDAT*)pRow; return DecB(p->byScouterInfo[i]); };
		}
	}
	return m;
}
#undef ITEMOPTION_FIELD

#define SKILL_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sSKILL_TBLDAT* p = (sSKILL_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetSkillExtractors()
{
	// byClass_Type, bySkill_Group, dwRequire_VP, and
	// dwUse_Restriction_Rule_Bit_Flag have no XML column. The
	// *InMilliSecs/fUse_Range_Min/fUse_Range_Max fields are derived from a
	// sibling column, not their own -- see GetXmlExportCaveat.
	static FieldExtractorMap m;
	if (m.empty())
	{
		SKILL_FIELD(L"Tblidx", Dec(p->tblidx));
		SKILL_FIELD(L"Skill_Name", Dec(p->Skill_Name));
		SKILL_FIELD(L"Name_Text", WideToXml(p->wszNameText));
		SKILL_FIELD(L"Validity_Able", Bl(p->bValidity_Able));
		SKILL_FIELD(L"PC_Class_Bit_Flag", Hex(p->dwPC_Class_Bit_Flag));
		SKILL_FIELD(L"Skill_Class", DecB(p->bySkill_Class));
		SKILL_FIELD(L"Skill_Type", DecB(p->bySkill_Type));
		SKILL_FIELD(L"Skill_Active_Type", DecB(p->bySkill_Active_Type));
		SKILL_FIELD(L"Buff_Group", DecB(p->byBuff_Group));
		SKILL_FIELD(L"Slot_Index", DecB(p->bySlot_Index));
		SKILL_FIELD(L"Skill_Grade", DecB(p->bySkill_Grade));
		SKILL_FIELD(L"Function_Bit_Flag", Hex(p->dwFunction_Bit_Flag));
		SKILL_FIELD(L"Appoint_Target", DecB(p->byAppoint_Target));
		SKILL_FIELD(L"Apply_Target", DecB(p->byApply_Target));
		SKILL_FIELD(L"Apply_Target_Max", DecB(p->byApply_Target_Max));
		SKILL_FIELD(L"Apply_Range", DecB(p->byApply_Range));
		SKILL_FIELD(L"Apply_Area_Size_1", DecB(p->byApply_Area_Size_1));
		SKILL_FIELD(L"Apply_Area_Size_2", DecB(p->byApply_Area_Size_2));

		for (int i = 0; i < (int)NTL_MAX_EFFECT_IN_SKILL; ++i)
		{
			CStringW key;

			key.Format(L"Skill_Effect_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sSKILL_TBLDAT* p = (sSKILL_TBLDAT*)pRow; return Dec(p->skill_Effect[i]); };

			key.Format(L"Skill_Effect_Type_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sSKILL_TBLDAT* p = (sSKILL_TBLDAT*)pRow; return DecB(p->bySkill_Effect_Type[i]); };

			key.Format(L"Skill_Effect_Value_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sSKILL_TBLDAT* p = (sSKILL_TBLDAT*)pRow; return Dbl(p->aSkill_Effect_Value[i]); };
		}

		for (int i = 0; i < (int)DBO_MAX_RP_BONUS_COUNT_PER_SKILL; ++i)
		{
			CStringW key;

			key.Format(L"RP_Effect_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sSKILL_TBLDAT* p = (sSKILL_TBLDAT*)pRow; return DecB(p->abyRpEffect[i]); };

			key.Format(L"RP_Effect_Value_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sSKILL_TBLDAT* p = (sSKILL_TBLDAT*)pRow; return Flt(p->afRpEffectValue[i]); };
		}

		SKILL_FIELD(L"Require_Train_Level", DecB(p->byRequire_Train_Level));
		SKILL_FIELD(L"Require_Zenny", Dec(p->dwRequire_Zenny));
		SKILL_FIELD(L"Next_Skill_Train_Exp", DecW(p->wNext_Skill_Train_Exp));
		SKILL_FIELD(L"Require_SP", DecW(p->wRequireSP));
		SKILL_FIELD(L"Self_Train", Bl(p->bSelfTrain));
		SKILL_FIELD(L"Require_Skill_Tblidx_Min_1", Dec(p->uiRequire_Skill_Tblidx_Min_1));
		SKILL_FIELD(L"Require_Skill_Tblidx_Max_1", Dec(p->uiRequire_Skill_Tblidx_Max_1));
		SKILL_FIELD(L"Require_Skill_Tblidx_Min_2", Dec(p->uiRequire_Skill_Tblidx_Min_2));
		SKILL_FIELD(L"Require_Skill_Tblidx_Max_2", Dec(p->uiRequire_Skill_Tblidx_Max_2));
		SKILL_FIELD(L"Root_Skill", Dec(p->Root_Skill));
		SKILL_FIELD(L"Require_Epuip_Slot_Type", DecB(p->byRequire_Epuip_Slot_Type));
		SKILL_FIELD(L"Require_Item_Type", DecB(p->byRequire_Item_Type));
		SKILL_FIELD(L"Icon_Name", AnsiToXml(p->szIcon_Name));
		SKILL_FIELD(L"Require_LP", Dec(p->dwRequire_LP));
		SKILL_FIELD(L"Require_EP", DecW(p->wRequire_EP));
		SKILL_FIELD(L"Require_RP_Ball", DecB(p->byRequire_RP_Ball));
		SKILL_FIELD(L"Casting_Time", Flt(p->fCasting_Time));
		SKILL_FIELD(L"Cool_Time", DecW(p->wCool_Time));
		SKILL_FIELD(L"Keep_Time", DecW(p->wKeep_Time));
		SKILL_FIELD(L"Keep_Effect", Bl(p->bKeep_Effect));
		SKILL_FIELD(L"Use_Range_Min", DecB(p->byUse_Range_Min));
		SKILL_FIELD(L"Use_Range_Max", DecB(p->byUse_Range_Max));
		SKILL_FIELD(L"Note", Dec(p->Note));
		SKILL_FIELD(L"Next_Skill_Tblidx", Dec(p->dwNextSkillTblidx));
		SKILL_FIELD(L"Default_Display_Off", Bl(p->bDefaultDisplayOff));
		SKILL_FIELD(L"Animation_Time", Dec(p->dwAnimation_Time));
		SKILL_FIELD(L"Casting_Animation_Start", DecW(p->wCasting_Animation_Start));
		SKILL_FIELD(L"Casting_Animation_Loop", DecW(p->wCasting_Animation_Loop));
		SKILL_FIELD(L"Action_Animation_Index", DecW(p->wAction_Animation_Index));
		SKILL_FIELD(L"Action_Loop_Animation_Index", DecW(p->wAction_Loop_Animation_Index));
		SKILL_FIELD(L"Action_End_Animation_Index", DecW(p->wAction_End_Animation_Index));
		SKILL_FIELD(L"Dash_Able", Bl(p->bDash_Able));
		SKILL_FIELD(L"Transform_Use_Info_Bit_Flag", Hex(p->dwTransform_Use_Info_Bit_Flag));
		SKILL_FIELD(L"Success_Rate", Flt(p->fSuccess_Rate));
		SKILL_FIELD(L"PC_Class_Change", DecB(p->byPC_Class_Change));
		SKILL_FIELD(L"Use_Type", DecB(p->byUse_Type));
	}
	return m;
}
#undef SKILL_FIELD




#define HLSITEM_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sHLS_ITEM_TBLDAT* p = (sHLS_ITEM_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetHlsItemExtractors()
{
	// byCategory has no XML column -- see GetXmlExportCaveat.
	static FieldExtractorMap m;
	if (m.empty())
	{
		HLSITEM_FIELD(L"Tblidx", Dec(p->tblidx));
		HLSITEM_FIELD(L"Item_Tblidx", Dec(p->itemTblidx));
		HLSITEM_FIELD(L"On_Sale", Bl(p->bOnSale));
		HLSITEM_FIELD(L"Cash", Dec(p->dwCash));
		HLSITEM_FIELD(L"Item_Stack_Count", DecB(p->byStackCount));
		HLSITEM_FIELD(L"Display_Bit_Flag", Hex(p->wDisplayBitFlag));
	}
	return m;
}
#undef HLSITEM_FIELD

#define STATUSTRANSFORM_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sSTATUS_TRANSFORM_TBLDAT* p = (sSTATUS_TRANSFORM_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetStatusTransformExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		STATUSTRANSFORM_FIELD(L"Tblidx", Dec(p->tblidx));
		STATUSTRANSFORM_FIELD(L"LP_Transform", Flt(p->fLP_Transform));
		STATUSTRANSFORM_FIELD(L"EP_Transform", Flt(p->fEP_Transform));
		STATUSTRANSFORM_FIELD(L"Physical_Offence_Transform", Flt(p->fPhysical_Offence_Transform));
		STATUSTRANSFORM_FIELD(L"Energy_Offence_Transform", Flt(p->fEnergy_Offence_Transform));
		STATUSTRANSFORM_FIELD(L"Physical_Defence_Transform", Flt(p->fPhysical_Defence_Transform));
		STATUSTRANSFORM_FIELD(L"Energy_Defence_Transform", Flt(p->fEnergy_Defence_Transform));
		STATUSTRANSFORM_FIELD(L"Run_Speed_Transform", Flt(p->fRun_Speed_Transform));
		STATUSTRANSFORM_FIELD(L"Attack_Speed_Transform", Flt(p->fAttack_Speed_Transform));
		STATUSTRANSFORM_FIELD(L"Attack_Rate_Transform", Flt(p->fAttack_Rate_Transform));
		STATUSTRANSFORM_FIELD(L"Dodge_Rate_Transform", Flt(p->fDodge_Rate_Transform));
		STATUSTRANSFORM_FIELD(L"Block_Rate_Transform", Flt(p->fBlock_Rate_Transform));
		STATUSTRANSFORM_FIELD(L"Curse_Success_Transform", Flt(p->fCurse_Success_Transform));
		STATUSTRANSFORM_FIELD(L"Curse_Tolerance_Transform", Flt(p->fCurse_Tolerance_Transform));
		STATUSTRANSFORM_FIELD(L"Attack_Range_Change", Flt(p->fAttack_Range_Change));
		STATUSTRANSFORM_FIELD(L"LP_Consume_Rate", Flt(p->fLP_Consume_Rate));
		STATUSTRANSFORM_FIELD(L"EP_Consume_Rate", Flt(p->fEP_Consume_Rate));
		// Duration is the only XML column for these two fields -- the reader
		// derives dwDurationInMilliSecs from it (* 1000) itself, same as
		// HTBSet's Cool_Time/dwCoolTimeInMilliSecs.
		STATUSTRANSFORM_FIELD(L"Duration", Dec(p->dwDuration));
	}
	return m;
}
#undef STATUSTRANSFORM_FIELD

// This table's XML reader (SetTableData) only recognizes Tblidx -- every
// other field errors out on import (verified against source). Only Tblidx
// is exposed here; see GetXmlExportCaveat.


#define QUESTPROBABILITY_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sQUEST_PROBABILITY_TBLDAT* p = (sQUEST_PROBABILITY_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetQuestProbabilityExtractors()
{
	// eUseType and byCount have no XML column at all -- see
	// GetXmlExportCaveat.
	static FieldExtractorMap m;
	if (m.empty())
	{
		QUESTPROBABILITY_FIELD(L"Tblidx", Dec(p->tblidx));
		QUESTPROBABILITY_FIELD(L"Name", WideToXml(p->wszName));
		QUESTPROBABILITY_FIELD(L"Note", WideToXml(p->wszNote));
		QUESTPROBABILITY_FIELD(L"Allow_Blank", Bl(p->bAllowBlank));
		QUESTPROBABILITY_FIELD(L"Probability_Type", DecB(p->byProbabilityType));

		for (int i = 0; i < NTL_QUEST_PROBABILITY_MAX_COUNT; ++i)
		{
			CStringW keyType; keyType.Format(L"Reward_Type%d", i + 1);
			m[keyType] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_PROBABILITY_TBLDAT* p = (sQUEST_PROBABILITY_TBLDAT*)pRow; return DecB(p->asProbabilityData[i].byType); };

			CStringW keyTblidx; keyTblidx.Format(L"Reward_Tblidx%d", i + 1);
			m[keyTblidx] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_PROBABILITY_TBLDAT* p = (sQUEST_PROBABILITY_TBLDAT*)pRow; return Dec(p->asProbabilityData[i].tblidx); };

			CStringW keyMin; keyMin.Format(L"Min_Value%d", i + 1);
			m[keyMin] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_PROBABILITY_TBLDAT* p = (sQUEST_PROBABILITY_TBLDAT*)pRow; return Dec(p->asProbabilityData[i].dwMinValue); };

			CStringW keyMax; keyMax.Format(L"Max_Value%d", i + 1);
			m[keyMax] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_PROBABILITY_TBLDAT* p = (sQUEST_PROBABILITY_TBLDAT*)pRow; return Dec(p->asProbabilityData[i].dwMaxValue); };

			CStringW keyRate; keyRate.Format(L"Drop_Rate%d", i + 1);
			m[keyRate] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_PROBABILITY_TBLDAT* p = (sQUEST_PROBABILITY_TBLDAT*)pRow; return Dec(p->asProbabilityData[i].dwRate); };
		}
	}
	return m;
}
#undef QUESTPROBABILITY_FIELD

#define WORLDPLAY_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sWORLDPLAY_TBLDAT* p = (sWORLDPLAY_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetWorldPlayExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		WORLDPLAY_FIELD(L"Tblidx", Dec(p->tblidx));
		WORLDPLAY_FIELD(L"Group", Dec(p->dwGroup));
		WORLDPLAY_FIELD(L"ExecuterType", DecB(p->byExecuterType));
		WORLDPLAY_FIELD(L"ShareType", DecB(p->byShareType));
		WORLDPLAY_FIELD(L"ShareLimitTime", Dec(p->dwShareLimitTime));
	}
	return m;
}
#undef WORLDPLAY_FIELD

// This table's XML reader (SetTableData) only recognizes Tblidx -- every
// other field errors out on import (verified against source). Only Tblidx
// is exposed here; see GetXmlExportCaveat.
#define SLOTMACHINE_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sHLS_SLOT_MACHINE_TBLDAT* p = (sHLS_SLOT_MACHINE_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetSlotMachineExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		SLOTMACHINE_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef SLOTMACHINE_FIELD

// This table's XML reader (SetTableData) only recognizes Tblidx -- see
// GetXmlExportCaveat.
#define SLOTMACHINEITEM_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sHLS_SLOT_MACHINE_ITEM_TBLDAT* p = (sHLS_SLOT_MACHINE_ITEM_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetSlotMachineItemExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		SLOTMACHINEITEM_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef SLOTMACHINEITEM_FIELD

// This table's XML reader (SetTableData) only recognizes Tblidx -- see
// GetXmlExportCaveat.
#define ITEMUPGRADERATENEW_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sITEM_UPGRADE_RATE_NEW_TBLDAT* p = (sITEM_UPGRADE_RATE_NEW_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetItemUpgradeRateNewExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		ITEMUPGRADERATENEW_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef ITEMUPGRADERATENEW_FIELD

// This table's XML reader (SetTableData) only recognizes Tblidx -- see
// GetXmlExportCaveat.
#define ITEMBAGLIST_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sITEM_BAG_LIST_TBLDAT* p = (sITEM_BAG_LIST_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetItemBagListExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		ITEMBAGLIST_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef ITEMBAGLIST_FIELD

// This table's XML reader (SetTableData) only recognizes Tblidx -- see
// GetXmlExportCaveat.
#define ITEMGROUPLIST_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sITEM_GROUP_LIST_TBLDAT* p = (sITEM_GROUP_LIST_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetItemGroupListExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		ITEMGROUPLIST_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef ITEMGROUPLIST_FIELD

// This table's XML reader (SetTableData) only recognizes Tblidx -- see
// GetXmlExportCaveat.
#define MOBSERVER_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sMOB_SERVER_TBLDAT* p = (sMOB_SERVER_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetMobServerExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		MOBSERVER_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef MOBSERVER_FIELD

// This table's XML reader (SetTableData) only recognizes Tblidx -- see
// GetXmlExportCaveat.
#define DBRETURNPOINT_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sDRAGONBALL_RETURN_POINT_TBLDAT* p = (sDRAGONBALL_RETURN_POINT_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetDragonBallReturnPointExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		DBRETURNPOINT_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef DBRETURNPOINT_FIELD

// This table's XML reader (SetTableData) only recognizes Tblidx -- see
// GetXmlExportCaveat.
#define EVENTSYSTEM_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sEVENT_SYSTEM_TBLDAT* p = (sEVENT_SYSTEM_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetEventSystemExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		EVENTSYSTEM_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef EVENTSYSTEM_FIELD

// This table's XML reader (SetTableData) only recognizes Tblidx -- see
// GetXmlExportCaveat.
#define DYNAMICFIELDSYSTEM_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sDYNAMIC_FIELD_SYSTEM_TBLDAT* p = (sDYNAMIC_FIELD_SYSTEM_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetDynamicFieldSystemExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		DYNAMICFIELDSYSTEM_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef DYNAMICFIELDSYSTEM_FIELD

#define SPEECH_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sNPC_SPEECH_TBLDAT* p = (sNPC_SPEECH_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetSpeechExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		SPEECH_FIELD(L"Tblidx", Dec(p->tblidx));
		SPEECH_FIELD(L"Dialog_Group", Dec(p->dwDialogGroup));
		SPEECH_FIELD(L"Dialog_Type", AnsiToXml(p->szDialogType));
		SPEECH_FIELD(L"Rate", DecB(p->byRate));
		SPEECH_FIELD(L"Text_Index", Dec(p->textIndex));
		SPEECH_FIELD(L"Ballon_Type", DecB(p->byBallonType));
		SPEECH_FIELD(L"Display_Time", Dec(p->dwDisplayTime));
		SPEECH_FIELD(L"Note", AnsiToXml(p->szNote));
	}
	return m;
}
#undef SPEECH_FIELD

#define HTBSET_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sHTB_SET_TBLDAT* p = (sHTB_SET_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetHtbSetExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		HTBSET_FIELD(L"Tblidx", Dec(p->tblidx));
		HTBSET_FIELD(L"Name_Text", WideToXml(p->wszNameText));
		HTBSET_FIELD(L"Validity_Able", Bl(p->bValidity_Able));
		HTBSET_FIELD(L"PC_Class_Bit_Flag", Hex(p->dwPC_Class_Bit_Flag));
		HTBSET_FIELD(L"Slot_Index", DecB(p->bySlot_Index));
		HTBSET_FIELD(L"Skill_Grade", DecB(p->bySkill_Grade));
		HTBSET_FIELD(L"HTB_Skill_Name", Dec(p->HTB_Skill_Name));
		HTBSET_FIELD(L"Icon_Name", AnsiToXml(p->szIcon_Name));
		HTBSET_FIELD(L"Need_EP", DecW(p->wNeed_EP));
		HTBSET_FIELD(L"Require_Train_Level", DecB(p->byRequire_Train_Level));
		HTBSET_FIELD(L"Require_Zenny", Dec(p->dwRequire_Zenny));
		HTBSET_FIELD(L"Next_Skill_Train_Exp", DecW(p->wNext_Skill_Train_Exp));
		HTBSET_FIELD(L"Cool_Time", DecW(p->wCool_Time));
		HTBSET_FIELD(L"Note", Dec(p->Note));
		HTBSET_FIELD(L"Set_Count", DecB(p->bySetCount));
		HTBSET_FIELD(L"Stop_Point", DecB(p->byStop_Point));
		HTBSET_FIELD(L"Require_SP", DecW(p->wRequireSP));

		for (int i = 0; i < (int)NTL_HTB_MAX_SKILL_COUNT_IN_SET; ++i)
		{
			CStringW key; key.Format(L"HTB_Type_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sHTB_SET_TBLDAT* p = (sHTB_SET_TBLDAT*)pRow; return DecB(p->aHTBAction[i].bySkillType); };
		}
		for (int i = 0; i < (int)NTL_HTB_MAX_SKILL_COUNT_IN_SET; ++i)
		{
			CStringW key; key.Format(L"Skill_Tblidx_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sHTB_SET_TBLDAT* p = (sHTB_SET_TBLDAT*)pRow; return Dec(p->aHTBAction[i].skillTblidx); };
		}
	}
	return m;
}
#undef HTBSET_FIELD

#define DIRLINK_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sDIRECTION_LINK_TBLDAT* p = (sDIRECTION_LINK_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetDirectionLinkExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		DIRLINK_FIELD(L"Tblidx", Dec(p->tblidx));
		DIRLINK_FIELD(L"Function_Name", AnsiToXml(p->szFunctionName));
		DIRLINK_FIELD(L"Note", AnsiToXml(p->szNote));
		DIRLINK_FIELD(L"Type", DecB(p->byType));
		DIRLINK_FIELD(L"Animation_ID", Dec(p->dwAnimationID));
		DIRLINK_FIELD(L"Direction_Func_Flag", Hex(p->byFuncFlag));
	}
	return m;
}
#undef DIRLINK_FIELD

#define DOJO_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sDOJO_TBLDAT* p = (sDOJO_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetDojoExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		DOJO_FIELD(L"Tblidx", Dec(p->tblidx));
		DOJO_FIELD(L"Zone_Tblidx", Dec(p->zoneTblidx));
		DOJO_FIELD(L"Map_Name", Dec(p->mapName));
		DOJO_FIELD(L"Receive_Hour", DecB(p->byReceiveHour));
		DOJO_FIELD(L"Receive_Minute", DecB(p->byReceiveMinute));
		DOJO_FIELD(L"Repeat_Type", DecB(p->byRepeatType));
		DOJO_FIELD(L"Repeat_Time", DecB(p->byRepeatTime));
		DOJO_FIELD(L"Week_Bit_Flag", Hex(p->wWeekBitFlag));
		DOJO_FIELD(L"Receive_Duration", DecB(p->byReceiveDuration));
		DOJO_FIELD(L"Reject_Duration", DecB(p->byRejectDuration));
		DOJO_FIELD(L"Standby_Duration", DecB(p->byStandbyDuration));
		DOJO_FIELD(L"Initial_Duration", DecB(p->byInitialDuration));
		DOJO_FIELD(L"Ready_Duration", DecB(p->byReadyDuration));
		DOJO_FIELD(L"Battle_Duration", DecB(p->byBattleDuration));
		DOJO_FIELD(L"Receive_Point", Dec(p->dwReceivePoint));
		DOJO_FIELD(L"Receive_Zeny", Dec(p->dwReceiveZenny));
		DOJO_FIELD(L"Controller_Tblidx", Dec(p->controllerTblidx));
		DOJO_FIELD(L"Battle_Point_Goal", Dec(p->dwBattlePointGoal));
		DOJO_FIELD(L"Battle_Point_Get", Dec(p->dwBattlePointGet));
		DOJO_FIELD(L"Battle_Point_Charge", Dec(p->dwBattlePointCharge));
		DOJO_FIELD(L"Charge_Point_Goal", Dec(p->dwChargePointGoal));
		DOJO_FIELD(L"Charge_Time", Dec(p->dwChargeTime));
		DOJO_FIELD(L"Charge_Time_Point", Dec(p->dwChageTimePoint));
		DOJO_FIELD(L"Rock_Tblidx", Dec(p->rockTblidx));

		for (int i = 0; i < (int)DOJO_MAX_UPGRADE_OBJECT_COUNT; ++i)
		{
			CStringW key; key.Format(L"Object_Tblidx_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sDOJO_TBLDAT* p = (sDOJO_TBLDAT*)pRow; return Dec(p->objectTblidx[i]); };
		}
		for (int i = 0; i < (int)DOJO_MAX_REWARD_TYPE_COUNT; ++i)
		{
			CStringW key; key.Format(L"Get_Point%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sDOJO_TBLDAT* p = (sDOJO_TBLDAT*)pRow; return Dec(p->asRawrd[i].dwGetPoint); };
		}
		for (int i = 0; i < (int)DOJO_MAX_REWARD_TYPE_COUNT; ++i)
		{
			CStringW key; key.Format(L"Get_Rock%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sDOJO_TBLDAT* p = (sDOJO_TBLDAT*)pRow; return DecB(p->asRawrd[i].byGetRock); };
		}
	}
	return m;
}
#undef DOJO_FIELD

#define MOBMOVEPATTERN_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sMOVE_PATTERN_TBLDAT* p = (sMOVE_PATTERN_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetMobMovePatternExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		MOBMOVEPATTERN_FIELD(L"Tblidx", Dec(p->tblidx));

		for (int i = 0; i < (int)DBO_MAX_COUNT_MOVE_PATTERN; ++i)
		{
			CStringW key; key.Format(L"Pattern_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sMOVE_PATTERN_TBLDAT* p = (sMOVE_PATTERN_TBLDAT*)pRow; return DecB(p->abyPattern[i]); };
		}
	}
	return m;
}
#undef MOBMOVEPATTERN_FIELD

#define LANDMARK_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sLAND_MARK_TBLDAT* p = (sLAND_MARK_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetLandMarkExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		LANDMARK_FIELD(L"Tblidx", Dec(p->tblidx));
		LANDMARK_FIELD(L"Name_Text", WideToXml(p->wszNameText));
		LANDMARK_FIELD(L"Landmark_Name", Dec(p->LandmarkName));
		LANDMARK_FIELD(L"Landmark_Type", DecB(p->byLandmarkType));
		LANDMARK_FIELD(L"Validity_Able", Bl(p->bValidityAble));
		LANDMARK_FIELD(L"Landmark_BitFlag", DecB(p->byLandmarkBitflag));
		LANDMARK_FIELD(L"Landmark_Display_BitFlag", DecB(p->byLandmarkDisplayBitFlag));
		LANDMARK_FIELD(L"Landmark_Loc_X", Flt(p->LandmarkLoc.x));
		LANDMARK_FIELD(L"Landmark_Loc_Z", Flt(p->LandmarkLoc.z));
		LANDMARK_FIELD(L"Link_Map_Idx", Dec(p->LinkMapIdx));
		LANDMARK_FIELD(L"Zone_Map_Idx", Dec(p->ZoneMapIdx));
		LANDMARK_FIELD(L"Link_Warfog_Idx", DecW(p->wLinkWarfogIdx));
		LANDMARK_FIELD(L"Icon_Name", WideToXml(p->wszIconName));
		LANDMARK_FIELD(L"Icon_Size", DecB(p->byIconSize));
		LANDMARK_FIELD(L"Note", Dec(p->Note));
	}
	return m;
}
#undef LANDMARK_FIELD

#define AIRCOSTUME_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sAIR_COSTUME_TBLDAT* p = (sAIR_COSTUME_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetAirCostumeExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		AIRCOSTUME_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef AIRCOSTUME_FIELD

#define QUESTTEXTDATA_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sQUEST_TEXT_DATA_TBLDAT* p = (sQUEST_TEXT_DATA_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetQuestTextDataExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		QUESTTEXTDATA_FIELD(L"Quest_Text_Index", Dec(p->tblidx));
		QUESTTEXTDATA_FIELD(L"Quest_Text", WideToXml(p->wstrText.c_str()));
	}
	return m;
}
#undef QUESTTEXTDATA_FIELD

#define TIMEQUEST_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetTimeQuestExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		TIMEQUEST_FIELD(L"Tblidx", Dec(p->tblidx));
		TIMEQUEST_FIELD(L"Type", DecB(p->byTimeQuestType));
		TIMEQUEST_FIELD(L"Difficultyflag", Hex(p->byDifficultyFlag));
		TIMEQUEST_FIELD(L"StartTime", Dec(p->dwStartTime));
		TIMEQUEST_FIELD(L"Start_Character_Direction", Dec(p->startCharacterDirection));
		TIMEQUEST_FIELD(L"Start_Object_Index", Dec(p->startObjectIndex));
		TIMEQUEST_FIELD(L"Start_Trigger_Id", Dec(p->startTriggerId));
		TIMEQUEST_FIELD(L"Leave_Trigger_Id", Dec(p->leaveTriggerId));
		TIMEQUEST_FIELD(L"Arrive_Character_Direction", Dec(p->arriveCharacterDirection));
		TIMEQUEST_FIELD(L"Leave_Character_Direction", Dec(p->leaveCharacterDirection));
		TIMEQUEST_FIELD(L"Arrive_Object_Index", Dec(p->arriveObjectIndex));
		TIMEQUEST_FIELD(L"Arrive_Trigger_Id", Dec(p->arriveTriggerId));
		TIMEQUEST_FIELD(L"Leave_Object_Index", Dec(p->leaveObjectIndex));
		TIMEQUEST_FIELD(L"DayRecord_MailIndex", Dec(p->dayRecordMailTblidx));
		TIMEQUEST_FIELD(L"BestRecord_MailIndex", Dec(p->bestRecordMailTblidx));
		TIMEQUEST_FIELD(L"Reset_Time", DecB(p->byResetTime));
		TIMEQUEST_FIELD(L"Prologue_Direction", WideToXml(p->wszPrologueDirection));
		TIMEQUEST_FIELD(L"Open_Cine", Dec(p->openCine));
		TIMEQUEST_FIELD(L"Note", Dec(p->Note));
		TIMEQUEST_FIELD(L"Stage_BGM1", WideToXml(p->wszStageBgm1));
		TIMEQUEST_FIELD(L"Stage_BGM2", WideToXml(p->wszStageBgm2));
		TIMEQUEST_FIELD(L"Last_BGM", WideToXml(p->wszLastBgm));

		for (int i = 0; i < 10; ++i)
		{
			CStringW key; key.Format(L"Start_Trigger_Direction_State%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return DecB(p->abyStartTriggerDirectionState[i]); };
		}

		// Name_Easy has no XML column -- see GetXmlExportCaveat.
		m[L"Name_Normal"] = [](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return Dec(p->sTimeQuestDataset[TIMEQUEST_DIFFICULTY_NORMAL].nameTblidx); };
		m[L"Name_Hard"] = [](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return Dec(p->sTimeQuestDataset[TIMEQUEST_DIFFICULTY_HARD].nameTblidx); };

		struct SDifficultySuffix { const WCHAR* pszSuffix; int nIndex; };
		static const SDifficultySuffix s_aDiff[3] =
		{
			{ L"Easy", TIMEQUEST_DIFFICULTY_EASY },
			{ L"Normal", TIMEQUEST_DIFFICULTY_NORMAL },
			{ L"Hard", TIMEQUEST_DIFFICULTY_HARD },
		};

		for (int i = 0; i < 3; ++i)
		{
			int nIndex = s_aDiff[i].nIndex;
			CStringW strSuffix = s_aDiff[i].pszSuffix;

			m[CStringW(L"QuestStringTblidx_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return Dec(p->sTimeQuestDataset[nIndex].questStringTblidx); };
			m[CStringW(L"WorldTblidx_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return Dec(p->sTimeQuestDataset[nIndex].worldTblidx); };
			m[CStringW(L"ScriptTblidx_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return Dec(p->sTimeQuestDataset[nIndex].scriptTblidx); };
			m[CStringW(L"LimitTime_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return Dec(p->sTimeQuestDataset[nIndex].dwLimitTime); };
			m[CStringW(L"MinMemberCount_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return DecB(p->sTimeQuestDataset[nIndex].byMinMemberCount); };
			m[CStringW(L"MaxMemberCount_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return DecB(p->sTimeQuestDataset[nIndex].byMaxMemberCount); };
			m[CStringW(L"MinMemberLevel_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return DecB(p->sTimeQuestDataset[nIndex].byMinMemberLevel); };
			m[CStringW(L"MaxMemberLevel_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return DecB(p->sTimeQuestDataset[nIndex].byMaxMemberLevel); };
			m[CStringW(L"NeedZenny_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return Dec(p->sTimeQuestDataset[nIndex].dwNeedZenny); };
			m[CStringW(L"NeedItem_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return Dec(p->sTimeQuestDataset[nIndex].needItemTblidx); };
			m[CStringW(L"NeedLimitCount_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return DecB(p->sTimeQuestDataset[nIndex].byNeedLimitCount); };
			m[CStringW(L"World_Count_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return DecB(p->sTimeQuestDataset[nIndex].byWorldCount); };
			m[CStringW(L"DayRecord_Reward_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return Dec(p->sTimeQuestDataset[nIndex].dayRecordRewardTblidx); };
			m[CStringW(L"BestRecord_Reward_") + strSuffix] = [nIndex](sTBLDAT* pRow) -> CStringW { sTIMEQUEST_TBLDAT* p = (sTIMEQUEST_TBLDAT*)pRow; return Dec(p->sTimeQuestDataset[nIndex].bestRecordRewardTblidx); };
		}
	}
	return m;
}
#undef TIMEQUEST_FIELD

#define BUDOKAI_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sBUDOKAI_TBLDAT* p = (sBUDOKAI_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetBudokaiExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		BUDOKAI_FIELD(L"Tblidx", Dec(p->tblidx));
		BUDOKAI_FIELD(L"Name", WideToXml(p->wstrName.c_str()));

		for (int i = 0; i < (int)BUDOKAI_MAX_TBLDAT_VALUE_COUNT; ++i)
		{
			CStringW key; key.Format(L"Value%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sBUDOKAI_TBLDAT* p = (sBUDOKAI_TBLDAT*)pRow; return WideToXml(p->wstrValue[i].c_str()); };
		}
	}
	return m;
}
#undef BUDOKAI_FIELD

CStringW RankBattleRuleTypeToXml(BYTE byRuleType)
{
	if (byRuleType == GAMERULE_RANKBATTLE)	return L"1";
	if (byRuleType == GAMERULE_MINORMATCH)	return L"100";
	if (byRuleType == GAMERULE_MAJORMATCH)	return L"101";
	if (byRuleType == GAMERULE_FINALMATCH)	return L"102";
	return DecB(byRuleType);
}

#define RANKBATTLE_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sRANKBATTLE_TBLDAT* p = (sRANKBATTLE_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetRankBattleExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		RANKBATTLE_FIELD(L"Tblidx", Dec(p->tblidx));
		RANKBATTLE_FIELD(L"Rule_Type", RankBattleRuleTypeToXml(p->byRuleType));
		RANKBATTLE_FIELD(L"Battle_Mode", DecB(p->byBattleMode));
		RANKBATTLE_FIELD(L"Name", WideToXml(p->wszName));
		RANKBATTLE_FIELD(L"Map_Index", Dec(p->worldTblidx));
		RANKBATTLE_FIELD(L"Need_Item", Dec(p->needItemTblidx));
		RANKBATTLE_FIELD(L"Need_Zenny", Dec(p->dwZenny));
		RANKBATTLE_FIELD(L"Min_Level", DecB(p->byMinLevel));
		RANKBATTLE_FIELD(L"Max_Level", DecB(p->byMaxLevel));
		RANKBATTLE_FIELD(L"Battle_Count", DecB(p->byBattleCount));
		RANKBATTLE_FIELD(L"WaitTime", Dec(p->dwWaitTime));
		RANKBATTLE_FIELD(L"DirectionTime", Dec(p->dwDirectionTime));
		RANKBATTLE_FIELD(L"MatchReadyTime", Dec(p->dwMatchReadyTime));
		RANKBATTLE_FIELD(L"StageReadyTime", Dec(p->dwStageReadyTime));
		RANKBATTLE_FIELD(L"StageRunTime", Dec(p->dwStageRunTime));
		RANKBATTLE_FIELD(L"StageFinishTime", Dec(p->dwStageFinishTime));
		RANKBATTLE_FIELD(L"MatchFinishTime", Dec(p->dwMatchFinishTime));
		RANKBATTLE_FIELD(L"BossDirection_Time", Dec(p->dwBossDirectionTime));
		RANKBATTLE_FIELD(L"BossKill_Time", Dec(p->dwBossKillTime));
		RANKBATTLE_FIELD(L"BossEndingTime", Dec(p->dwBossEndingTime));
		RANKBATTLE_FIELD(L"EndTime", Dec(p->dwEndTime));
		RANKBATTLE_FIELD(L"KO_Score", DecB((BYTE)p->chScoreKO));
		RANKBATTLE_FIELD(L"OutofArea_Score", DecB((BYTE)p->chScoreOutOfArea));
		RANKBATTLE_FIELD(L"Pointwin_Score", DecB((BYTE)p->chScorePointWin));
		RANKBATTLE_FIELD(L"Draw_Score", DecB((BYTE)p->chScoreDraw));
		RANKBATTLE_FIELD(L"Lost_Score", DecB((BYTE)p->chScoreLose));
		RANKBATTLE_FIELD(L"Excellent_Result", DecB((BYTE)p->chResultExcellent));
		RANKBATTLE_FIELD(L"Greate_Result", DecB((BYTE)p->chResultGreate));
		RANKBATTLE_FIELD(L"Good_Result", DecB((BYTE)p->chResultGood));
		RANKBATTLE_FIELD(L"Draw_Result", DecB((BYTE)p->chResultDraw));
		RANKBATTLE_FIELD(L"Lost_Result", DecB((BYTE)p->chResultLose));
		RANKBATTLE_FIELD(L"PerfectWinner_Score", DecB((BYTE)p->chBonusPerfectWinner));
		RANKBATTLE_FIELD(L"NormalWinner_Score", DecB((BYTE)p->chBonusNormalWinner));
		RANKBATTLE_FIELD(L"Day_Entry_Num", DecB(p->byDayEntryNum));
		RANKBATTLE_FIELD(L"OutSide_Able", Bl(p->bOutSizeAble));
		RANKBATTLE_FIELD(L"Info_Index", Dec(p->dwInfoIndex));
		RANKBATTLE_FIELD(L"StageMinClearTime", Dec(p->dwStateMinClearTime));
	}
	return m;
}
#undef RANKBATTLE_FIELD

#define SCRIPTLINK_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sSCRIPT_LINK_TBLDAT* p = (sSCRIPT_LINK_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetScriptLinkExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		SCRIPTLINK_FIELD(L"Tblidx", Dec(p->tblidx));
		SCRIPTLINK_FIELD(L"Value", WideToXml(p->wszValue));
		SCRIPTLINK_FIELD(L"Type", DecB(p->byType));
		SCRIPTLINK_FIELD(L"Action", DecB(p->byAction));
	}
	return m;
}
#undef SCRIPTLINK_FIELD

#define QUESTNARRATION_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sQUEST_NARRATION_TBLDAT* p = (sQUEST_NARRATION_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetQuestNarrationExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		QUESTNARRATION_FIELD(L"Tblidx", Dec(p->tblidx));
		QUESTNARRATION_FIELD(L"Type", Bl(p->bType));
		QUESTNARRATION_FIELD(L"Number", DecB(p->byNumber));
		QUESTNARRATION_FIELD(L"Time", DecB(p->byTime));

		for (int i = 0; i < (int)DBO_MAX_COUNT_OF_NARRATION; ++i)
		{
			CStringW key;
			key.Format(L"UIShowHideDirection_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_NARRATION_TBLDAT* p = (sQUEST_NARRATION_TBLDAT*)pRow; return DecB(p->asData[i].byUIShowHideDirection); };
			key.Format(L"OwnerType_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_NARRATION_TBLDAT* p = (sQUEST_NARRATION_TBLDAT*)pRow; return DecB(p->asData[i].byOwnerType); };
			key.Format(L"Owner_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_NARRATION_TBLDAT* p = (sQUEST_NARRATION_TBLDAT*)pRow; return Dec(p->asData[i].dwOwner); };
			key.Format(L"Condition_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_NARRATION_TBLDAT* p = (sQUEST_NARRATION_TBLDAT*)pRow; return DecB(p->asData[i].byCondition); };
			key.Format(L"Direction_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_NARRATION_TBLDAT* p = (sQUEST_NARRATION_TBLDAT*)pRow; return DecB(p->asData[i].byDirection); };
			key.Format(L"Dialog_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_NARRATION_TBLDAT* p = (sQUEST_NARRATION_TBLDAT*)pRow; return Dec(p->asData[i].dwDialog); };
			key.Format(L"UiType_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_NARRATION_TBLDAT* p = (sQUEST_NARRATION_TBLDAT*)pRow; return DecB(p->asData[i].byUiType); };
			key.Format(L"UIDirection_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_NARRATION_TBLDAT* p = (sQUEST_NARRATION_TBLDAT*)pRow; return DecB(p->asData[i].byUIDirection); };
		}
	}
	return m;
}
#undef QUESTNARRATION_FIELD

#define QUESTITEM_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sQUESTITEM_TBLDAT* p = (sQUESTITEM_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetQuestItemExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		QUESTITEM_FIELD(L"Item_Tblidx", Dec(p->tblidx));
		QUESTITEM_FIELD(L"Item_Name", Dec(p->ItemName));
		QUESTITEM_FIELD(L"Icon_Name", AnsiToXml(p->szIconName));
		QUESTITEM_FIELD(L"Note", Dec(p->Note));
		QUESTITEM_FIELD(L"Function_Bit_Flag", Hex(p->byFunctionBitFlag));
	}
	return m;
}
#undef QUESTITEM_FIELD

#define ITEMDISASSEMBLE_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sITEM_DISASSEMBLE_TBLDAT* p = (sITEM_DISASSEMBLE_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetItemDisassembleExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		ITEMDISASSEMBLE_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef ITEMDISASSEMBLE_FIELD

#define PORTAL_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sPORTAL_TBLDAT* p = (sPORTAL_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetPortalExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		PORTAL_FIELD(L"Tblidx", Dec(p->tblidx));
		PORTAL_FIELD(L"Point_Name", Dec(p->dwPointName));
		PORTAL_FIELD(L"Point_Name_Text", AnsiToXml(p->szPointNameText));
		PORTAL_FIELD(L"World", Dec(p->worldId));
		PORTAL_FIELD(L"Grade", DecB(p->byGrade));
		PORTAL_FIELD(L"Loc_X", Flt(p->vLoc.x));
		PORTAL_FIELD(L"Loc_Y", Flt(p->vLoc.y));
		PORTAL_FIELD(L"Loc_Z", Flt(p->vLoc.z));
		PORTAL_FIELD(L"Dir_X", Flt(p->vDir.x));
		PORTAL_FIELD(L"Dir_Z", Flt(p->vDir.z));
		PORTAL_FIELD(L"Map_X", Flt(p->vMap.x));
		PORTAL_FIELD(L"Map_Y", Flt(p->vMap.z));

		for (int i = 0; i < (int)DBO_MAX_POINT_PORTAL; ++i)
		{
			CStringW key; key.Format(L"Point_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sPORTAL_TBLDAT* p = (sPORTAL_TBLDAT*)pRow; return Dec(p->aPoint[i]); };
		}
		for (int i = 0; i < (int)DBO_MAX_POINT_PORTAL; ++i)
		{
			CStringW key; key.Format(L"Zeny_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sPORTAL_TBLDAT* p = (sPORTAL_TBLDAT*)pRow; return Dec(p->adwPointZenny[i]); };
		}
	}
	return m;
}
#undef PORTAL_FIELD

#define ITEMRECIPE_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sITEM_RECIPE_TBLDAT* p = (sITEM_RECIPE_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetItemRecipeExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		ITEMRECIPE_FIELD(L"Tblidx", Dec(p->tblidx));
		ITEMRECIPE_FIELD(L"Validity_Able", Bl(p->bValidityAble));
		ITEMRECIPE_FIELD(L"Name", Dec(p->dwName));
		ITEMRECIPE_FIELD(L"Recipe_Type", DecB(p->byRecipeType));
		ITEMRECIPE_FIELD(L"Need_Mix_Level", DecB(p->byNeedMixLevel));
		ITEMRECIPE_FIELD(L"Need_Mix_Zenny", Dec(p->dwNeedMixZenny));

		for (int i = 0; i < (int)DBO_MAX_COUNT_RECIPE_CREATE_ITEM; ++i)
		{
			CStringW key; key.Format(L"Create_Item_Tblidx_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sITEM_RECIPE_TBLDAT* p = (sITEM_RECIPE_TBLDAT*)pRow; return Dec(p->asCreateItemTblidx[i].itemTblidx); };
		}
		for (int i = 0; i < (int)DBO_MAX_COUNT_RECIPE_CREATE_ITEM; ++i)
		{
			CStringW key; key.Format(L"Create_Item_Rate_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sITEM_RECIPE_TBLDAT* p = (sITEM_RECIPE_TBLDAT*)pRow; return DecB(p->asCreateItemTblidx[i].itemRate); };
		}
		for (int i = 0; i < (int)DBO_MAX_COUNT_RECIPE_MATERIAL_ITEM; ++i)
		{
			CStringW key; key.Format(L"Material_Tblidx_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sITEM_RECIPE_TBLDAT* p = (sITEM_RECIPE_TBLDAT*)pRow; return Dec(p->asMaterial[i].materialTblidx); };
		}
		for (int i = 0; i < (int)DBO_MAX_COUNT_RECIPE_MATERIAL_ITEM; ++i)
		{
			CStringW key; key.Format(L"Material_Quantity_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sITEM_RECIPE_TBLDAT* p = (sITEM_RECIPE_TBLDAT*)pRow; return DecB(p->asMaterial[i].byMaterialCount); };
		}
	}
	return m;
}
#undef ITEMRECIPE_FIELD

#define MIXMACHINE_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sITEM_MIX_MACHINE_TBLDAT* p = (sITEM_MIX_MACHINE_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetMixMachineExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		MIXMACHINE_FIELD(L"Tblidx", Dec(p->tblidx));
		MIXMACHINE_FIELD(L"Validity_Able", Bl(p->bValidityAble));
		MIXMACHINE_FIELD(L"Name", Dec(p->name));
		MIXMACHINE_FIELD(L"Machine_Type", DecB(p->byMachineType));
		MIXMACHINE_FIELD(L"Function_Bit_Flag", Hex(p->wFunctionBitFlag));
		MIXMACHINE_FIELD(L"Mix_Zenny_Discount_Rate", DecB(p->byMixZennyDiscountRate));
		MIXMACHINE_FIELD(L"Dynamic_Object_Tblidx", Dec(p->dynamicObjectTblidx));

		for (int i = 0; i < (int)DBO_MAX_COUNT_BUILT_IN_RECIPE; ++i)
		{
			CStringW key; key.Format(L"Built_In_Recipe_Tblidx_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sITEM_MIX_MACHINE_TBLDAT* p = (sITEM_MIX_MACHINE_TBLDAT*)pRow; return Dec(p->aBuiltInRecipeTblidx[i]); };
		}
	}
	return m;
}
#undef MIXMACHINE_FIELD

#define DRAGONBALL_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sDRAGONBALL_TBLDAT* p = (sDRAGONBALL_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetDragonBallExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		DRAGONBALL_FIELD(L"Tblidx", Dec(p->tblidx));
		DRAGONBALL_FIELD(L"Altar_Group", Dec(p->dwAltarGroup));
		DRAGONBALL_FIELD(L"Ball_Type", DecB(p->byBallType));
		DRAGONBALL_FIELD(L"Ball_Drop_Tblidx", Dec(p->ballDropTblidx));
		DRAGONBALL_FIELD(L"Ball_Junk_Tblidx", Dec(p->ballJunkTblidx));
		DRAGONBALL_FIELD(L"Start_Dialog", Dec(p->startDialog));
		DRAGONBALL_FIELD(L"End_Dialog", Dec(p->endDialog));
		DRAGONBALL_FIELD(L"Timeover_End_Dialog", Dec(p->timeoverEndDialog));
		DRAGONBALL_FIELD(L"Hurry_Dialog", Dec(p->hurryDialog));
		DRAGONBALL_FIELD(L"Timeover_Dialog", Dec(p->timeoverDialog));
		DRAGONBALL_FIELD(L"No_Repeat_Dialog", Dec(p->noRepeatDialog));
		DRAGONBALL_FIELD(L"Inventory_Full_Dialog", Dec(p->inventoryFullDialog));
		DRAGONBALL_FIELD(L"Skill_Overlap_Dialog", Dec(p->skillOverlapDialog));
		DRAGONBALL_FIELD(L"Skill_Shortage_Of_LV_Dialog", Dec(p->skillShortageOfLVDialog));
		DRAGONBALL_FIELD(L"Dragon_NPC_Tblidx", Dec(p->dragonNPCTblidx));
		DRAGONBALL_FIELD(L"Default_Summon_Chat", Dec(p->defaultSummonChat));
		DRAGONBALL_FIELD(L"Appear_Dir_X", Flt(p->fDir.x));
		DRAGONBALL_FIELD(L"Appear_Dir_Z", Flt(p->fDir.z));

		for (int i = 0; i < (int)NTL_ITEM_MAX_DRAGONBALL; ++i)
		{
			CStringW key; key.Format(L"Ball_%d_Tblidx", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sDRAGONBALL_TBLDAT* p = (sDRAGONBALL_TBLDAT*)pRow; return Dec(p->aBallTblidx[i]); };
		}
	}
	return m;
}
#undef DRAGONBALL_FIELD

#define WORLDZONE_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sWORLD_ZONE_TBLDAT* p = (sWORLD_ZONE_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetWorldZoneExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		WORLDZONE_FIELD(L"Tblidx", Dec(p->tblidx));
		WORLDZONE_FIELD(L"Function_Bit_Flag", Hex(p->wFunctionBitFlag));
		WORLDZONE_FIELD(L"World", Dec(p->worldTblidx));
		WORLDZONE_FIELD(L"Name", Dec(p->nameTblidx));
		WORLDZONE_FIELD(L"Name_Text", WideToXml(p->wszName_Text));
		WORLDZONE_FIELD(L"Forbidden_Vehicle", Bl(p->bForbidden_Vehicle));
	}
	return m;
}
#undef WORLDZONE_FIELD

#define COMMONCONFIG_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sCOMMONCONFIG_TBLDAT* p = (sCOMMONCONFIG_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetCommonConfigExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		COMMONCONFIG_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef COMMONCONFIG_FIELD

#define DWC_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sDWC_TBLDAT* p = (sDWC_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetDwcExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		DWC_FIELD(L"Tblidx", Dec(p->tblidx));
		DWC_FIELD(L"Name", Dec(p->tblNameIndex));
		DWC_FIELD(L"Level_Min", DecB(p->byLevel_Min));
		DWC_FIELD(L"Level_Max", DecB(p->byLevel_Max));
		DWC_FIELD(L"Admission_Bit_Flag", Hex(p->wAdmission_Bit_Flag));
		DWC_FIELD(L"Admission_Num_Min", DecB(p->byAdmission_Num_Min));
		DWC_FIELD(L"Admission_Num_Max", DecB(p->byAdmission_Num_Max));
		DWC_FIELD(L"Prologue_Cinematic_Tblidx", Dec(p->prologueCinematicTblidx));
		DWC_FIELD(L"Prologue_Text", Dec(p->prologueTblidx));
		DWC_FIELD(L"World_Tblidx", Dec(p->worldTblidx));

		for (int i = 0; i < (int)MAX_DWC_ADMISSION_CONDITION_COUNT; ++i)
		{
			CStringW key; key.Format(L"Condition_Tblidx%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sDWC_TBLDAT* p = (sDWC_TBLDAT*)pRow; return Dec(p->aConditionTblidx[i]); };
		}
		for (int i = 0; i < (int)MAX_DWC_MISSION_COUNT_PER_SCENARIO; ++i)
		{
			CStringW key; key.Format(L"Mission_Tblidx%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sDWC_TBLDAT* p = (sDWC_TBLDAT*)pRow; return Dec(p->aMissionTblidx[i]); };
		}
	}
	return m;
}
#undef DWC_FIELD

#define WORLDMAP_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sWORLD_MAP_TBLDAT* p = (sWORLD_MAP_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetWorldMapExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		WORLDMAP_FIELD(L"Tblidx", Dec(p->tblidx));
		WORLDMAP_FIELD(L"World_Tblidx", Dec(p->World_Tblidx));
		WORLDMAP_FIELD(L"Zone_Tblidx", Dec(p->Zone_Tblidx));
		WORLDMAP_FIELD(L"Worldmap_Name", Dec(p->Worldmap_Name));
		WORLDMAP_FIELD(L"Name_Text", WideToXml(p->wszNameText));
		WORLDMAP_FIELD(L"Validity_Able", Bl(p->bValidityAble));
		WORLDMAP_FIELD(L"Map_Type", DecB(p->byMapType));
		WORLDMAP_FIELD(L"Standard_Loc_X", Flt(p->vStandardLoc.x));
		WORLDMAP_FIELD(L"Standard_Loc_Z", Flt(p->vStandardLoc.z));
		WORLDMAP_FIELD(L"Worldmap_Scale", Flt(p->fWorldmapScale));
		WORLDMAP_FIELD(L"Link_Map_Idx", Dec(p->dwLinkMapIdx));
		WORLDMAP_FIELD(L"Combobox_Type", Dec(p->dwComboBoxType));
		WORLDMAP_FIELD(L"Recomm_Min_Level", DecB(p->byRecomm_Min_Level));
		WORLDMAP_FIELD(L"Recomm_Max_Level", DecB(p->byRecomm_Max_Level));

		for (int i = 0; i < (int)DBO_WORLD_MAP_TABLE_COUNT_WORLD_WARFOG; ++i)
		{
			CStringW key; key.Format(L"Warfog_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sWORLD_MAP_TBLDAT* p = (sWORLD_MAP_TBLDAT*)pRow; return DecW(p->wWarfog[i]); };
		}
	}
	return m;
}
#undef WORLDMAP_FIELD

#define PC_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sPC_TBLDAT* p = (sPC_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetPcExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		PC_FIELD(L"Tblidx", Dec(p->tblidx));
		PC_FIELD(L"Race", DecB(p->byRace));
		PC_FIELD(L"Gender", DecB(p->byGender));
		PC_FIELD(L"Class", DecB(p->byClass));
		PC_FIELD(L"Prior_Class_Tblidx", Dec(p->prior_Class_Tblidx));
		PC_FIELD(L"Class_Bit_Flag", Hex(p->dwClass_Bit_Flag));
		PC_FIELD(L"Model_Child", AnsiToXml(p->szModel_Child));
		PC_FIELD(L"Model_Adult", AnsiToXml(p->szModel_Adult));
		PC_FIELD(L"Basic_LP", Dec((DWORD)p->dwBasic_LP));
		PC_FIELD(L"Basic_EP", DecW(p->wBasic_EP));
		PC_FIELD(L"Basic_RP", DecW(p->wBasic_RP));
		PC_FIELD(L"Basic_Physical_Offence", DecW(p->wBasic_Physical_Offence));
		PC_FIELD(L"Basic_Physical_Defence", DecW(p->wBasic_Physical_Defence));
		PC_FIELD(L"Basic_Energy_Offence", DecW(p->wBasic_Energy_Offence));
		PC_FIELD(L"Basic_Energy_Defence", DecW(p->wBasic_Energy_Defence));
		PC_FIELD(L"Basic_Str", DecW(p->wBasicStr));
		PC_FIELD(L"Basic_Con", DecW(p->wBasicCon));
		PC_FIELD(L"Basic_Foc", DecW(p->wBasicFoc));
		PC_FIELD(L"Basic_Dex", DecW(p->wBasicDex));
		PC_FIELD(L"Basic_Sol", DecW(p->wBasicSol));
		PC_FIELD(L"Basic_Eng", DecW(p->wBasicEng));
		PC_FIELD(L"Level_Up_LP", DecB(p->byLevel_Up_LP));
		PC_FIELD(L"Level_Up_EP", DecB(p->byLevel_Up_EP));
		PC_FIELD(L"Level_Up_RP", DecB(p->byLevel_Up_RP));
		PC_FIELD(L"Level_Up_Physical_Offence", DecB(p->byLevel_Up_Physical_Offence));
		PC_FIELD(L"Level_Up_Physical_Defence", DecB(p->byLevel_Up_Physical_Defence));
		PC_FIELD(L"Level_Up_Energy_Offence", DecB(p->byLevel_Up_Energy_Offence));
		PC_FIELD(L"Level_Up_Energy_Defence", DecB(p->byLevel_Up_Energy_Defence));
		PC_FIELD(L"Level_Up_Str", Flt(p->fLevel_Up_Str));
		PC_FIELD(L"Level_Up_Con", Flt(p->fLevel_Up_Con));
		PC_FIELD(L"Level_Up_Foc", Flt(p->fLevel_Up_Foc));
		PC_FIELD(L"Level_Up_Dex", Flt(p->fLevel_Up_Dex));
		PC_FIELD(L"Level_Up_Sol", Flt(p->fLevel_Up_Sol));
		PC_FIELD(L"Level_Up_Eng", Flt(p->fLevel_Up_Eng));
		PC_FIELD(L"Scale", Flt(p->fScale));
		PC_FIELD(L"Child_Run_Speed_Origin", Flt(p->fChild_Run_Speed_Origin));
		PC_FIELD(L"Child_Run_Speed", Flt(p->fChild_Run_Speed));
		PC_FIELD(L"Adult_Run_Speed_Origin", Flt(p->fAdult_Run_Speed_Origin));
		PC_FIELD(L"Adult_Run_Speed", Flt(p->fAdult_Run_Speed));
		PC_FIELD(L"Attack_Speed_Rate", DecW(p->wAttack_Speed_Rate));
		PC_FIELD(L"Attack_Type", DecB(p->byAttack_Type));
		PC_FIELD(L"Attack_Range", Flt(p->fAttack_Range));
		PC_FIELD(L"Basic_Attack_Rate", DecW(p->wAttack_Rate));
		PC_FIELD(L"Basic_Dodge_Rate", DecW(p->wDodge_Rate));
		PC_FIELD(L"Basic_Block_Rate", DecW(p->wBlock_Rate));
		PC_FIELD(L"Basic_Curse_Success_Rate", DecW(p->wCurse_Success_Rate));
		PC_FIELD(L"Basic_Curse_Tolerance_Rate", DecW(p->wCurse_Tolerance_Rate));
		PC_FIELD(L"Basic_Aggro_Point", DecW(p->wBasic_Aggro_Point));
	}
	return m;
}
#undef PC_FIELD

#define NPC_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sNPC_TBLDAT* p = (sNPC_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetNpcExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		NPC_FIELD(L"Tblidx", Dec(p->tblidx));
		NPC_FIELD(L"Validity_Able", Bl(p->bValidity_Able));
		NPC_FIELD(L"Name", Dec(p->Name));
		NPC_FIELD(L"Name_Text", WideToXml(p->wszNameText));
		NPC_FIELD(L"Model", AnsiToXml(p->szModel));
		NPC_FIELD(L"Level", DecB(p->byLevel));
		NPC_FIELD(L"Job", DecB(p->byJob));
		NPC_FIELD(L"Function_Bit_Flag", Hex(p->dwFunc_Bit_Flag));
		NPC_FIELD(L"Ai_Bit_Flag", Hex(p->dwAi_Bit_Flag));
		NPC_FIELD(L"Dialog_Script_Index", Dec(p->Dialog_Script_Index));
		NPC_FIELD(L"Battle_Attribute", DecB(p->byBattle_Attribute));
		NPC_FIELD(L"NPC_type", DecB(p->byNpcType));
		NPC_FIELD(L"Basic_LP", Dec((DWORD)p->dwBasic_LP));
		NPC_FIELD(L"LP_Regeneration", DecW(p->wLP_Regeneration));
		NPC_FIELD(L"Basic_EP", DecW(p->wBasic_EP));
		NPC_FIELD(L"EP_Regeneration", DecW(p->wEP_Regeneration));
		NPC_FIELD(L"Attack_Type", DecB(p->byAttack_Type));
		NPC_FIELD(L"Basic_Physical_Offence", DecW(p->wBasic_Physical_Offence));
		NPC_FIELD(L"Basic_Energy_Offence", DecW(p->wBasic_Energy_Offence));
		NPC_FIELD(L"Basic_Physical_Defence", DecW(p->wBasic_Physical_Defence));
		NPC_FIELD(L"Basic_Energy_Defence", DecW(p->wBasic_Energy_Defence));
		NPC_FIELD(L"Str", DecW(p->wBasicStr));
		NPC_FIELD(L"Con", DecW(p->wBasicCon));
		NPC_FIELD(L"Foc", DecW(p->wBasicFoc));
		NPC_FIELD(L"Dex", DecW(p->wBasicDex));
		NPC_FIELD(L"Sol", DecW(p->wBasicSol));
		NPC_FIELD(L"Eng", DecW(p->wBasicEng));
		NPC_FIELD(L"Scale", Flt(p->fScale));
		NPC_FIELD(L"Walk_Speed_Origin", Flt(p->fWalk_Speed_Origin));
		NPC_FIELD(L"Walk_Speed", Flt(p->fWalk_Speed));
		NPC_FIELD(L"Run_Speed_Origin", Flt(p->fRun_Speed_Origin));
		NPC_FIELD(L"Run_Speed", Flt(p->fRun_Speed));
		NPC_FIELD(L"Radius_X", Flt(p->fRadius_X));
		NPC_FIELD(L"Radius_Z", Flt(p->fRadius_Z));
		NPC_FIELD(L"Attack_Speed_Rate", DecW(p->wAttack_Speed_Rate));
		NPC_FIELD(L"Attack_Cool_Time", DecW(p->wAttackCoolTime));
		NPC_FIELD(L"Attack_Range", Flt(p->fAttack_Range));
		NPC_FIELD(L"Basic_Attack_Rate", DecW(p->wAttack_Rate));
		NPC_FIELD(L"Basic_Dodge_Rate", DecW(p->wDodge_Rate));
		NPC_FIELD(L"Basic_Block_Rate", DecW(p->wBlock_Rate));
		NPC_FIELD(L"Basic_Curse_Success_Rate", DecW(p->wCurse_Success_Rate));
		NPC_FIELD(L"Basic_Curse_Tolerance_Rate", DecW(p->wCurse_Tolerance_Rate));
		NPC_FIELD(L"Sight_Range", DecW(p->wSight_Range));
		NPC_FIELD(L"Scan_Range", DecW(p->wScan_Range));
		NPC_FIELD(L"Visible_Sight_Range", DecB(p->byVisible_Sight_Range));
		NPC_FIELD(L"Camera_Bone_Name", AnsiToXml(p->szCamera_Bone_Name));
		NPC_FIELD(L"Status_Transform_Tblidx", Dec(p->statusTransformTblidx));
		NPC_FIELD(L"Fly_Height", Flt(p->fFly_Height));
		NPC_FIELD(L"Spawn_Animation", Bl(p->bSpawn_Animation));
		NPC_FIELD(L"ILLust", AnsiToXml(p->szILLust));
		NPC_FIELD(L"Contents_Tblidx", Dec(p->contentsTblidx));
		NPC_FIELD(L"Dialog_Group", Dec(p->dwDialogGroup));
		NPC_FIELD(L"Alliance_Idx", Dec(p->dwAllianceIdx));
		NPC_FIELD(L"Aggro_Max_Count", DecW(p->wAggroMaxCount));
		NPC_FIELD(L"Basic_Aggro_Point", DecW(p->wBasic_Aggro_Point));

		for (int i = 0; i < (int)NTL_MAX_NPC_HAVE_SKILL; ++i)
		{
			CStringW key;
			key.Format(L"Use_Skill_Time_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sNPC_TBLDAT* p = (sNPC_TBLDAT*)pRow; return DecW(p->wUse_Skill_Time[i]); };
			key.Format(L"Use_Skill_Tblidx_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sNPC_TBLDAT* p = (sNPC_TBLDAT*)pRow; return Dec(p->use_Skill_Tblidx[i]); };
			key.Format(L"Use_Skill_Basis_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sNPC_TBLDAT* p = (sNPC_TBLDAT*)pRow; return DecB(p->byUse_Skill_Basis[i]); };
			key.Format(L"Use_Skill_LP_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sNPC_TBLDAT* p = (sNPC_TBLDAT*)pRow; return DecW(p->wUse_Skill_LP[i]); };
		}
		for (int i = 0; i < (int)NTL_MAX_MERCHANT_TAB_COUNT; ++i)
		{
			CStringW key; key.Format(L"Merchant_Tblidx_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sNPC_TBLDAT* p = (sNPC_TBLDAT*)pRow; return Dec(p->amerchant_Tblidx[i]); };
		}
	}
	return m;
}
#undef NPC_FIELD

#define MOB_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sMOB_TBLDAT* p = (sMOB_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetMobExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		MOB_FIELD(L"Tblidx", Dec(p->tblidx));
		MOB_FIELD(L"Validity_Able", Bl(p->bValidity_Able));
		MOB_FIELD(L"Name", Dec(p->Name));
		MOB_FIELD(L"Name_Text", WideToXml(p->wszNameText));
		MOB_FIELD(L"Model", AnsiToXml(p->szModel));
		MOB_FIELD(L"Level", DecB(p->byLevel));
		MOB_FIELD(L"Mob_Group", Dec(p->dwMobGroup));
		MOB_FIELD(L"Mob_Kind", DecW(p->wMob_Kind));
		MOB_FIELD(L"Exp", Dec(p->dwExp));
		MOB_FIELD(L"Grade", DecB(p->byGrade));
		MOB_FIELD(L"Battle_Attribute", DecB(p->byBattle_Attribute));
		MOB_FIELD(L"Mob_type", DecB(p->byMob_Type));
		MOB_FIELD(L"Ai_Bit_Flag", Hex(p->dwAi_Bit_Flag));
		MOB_FIELD(L"Basic_LP", Dec((DWORD)p->dwBasic_LP));
		MOB_FIELD(L"LP_Regeneration", DecW(p->wLP_Regeneration));
		MOB_FIELD(L"Basic_EP", DecW(p->wBasic_EP));
		MOB_FIELD(L"EP_Regeneration", DecW(p->wEP_Regeneration));
		MOB_FIELD(L"Attack_Type", DecB(p->byAttack_Type));
		MOB_FIELD(L"Basic_Physical_Offence", DecW(p->wBasic_Physical_Offence));
		MOB_FIELD(L"Basic_Energy_Offence", DecW(p->wBasic_Energy_Offence));
		MOB_FIELD(L"Basic_Physical_Defence", DecW(p->wBasic_Physical_Defence));
		MOB_FIELD(L"Basic_Energy_Defence", DecW(p->wBasic_Energy_Defence));
		MOB_FIELD(L"Str", DecW(p->wBasicStr));
		MOB_FIELD(L"Con", DecW(p->wBasicCon));
		MOB_FIELD(L"Foc", DecW(p->wBasicFoc));
		MOB_FIELD(L"Dex", DecW(p->wBasicDex));
		MOB_FIELD(L"Sol", DecW(p->wBasicSol));
		MOB_FIELD(L"Eng", DecW(p->wBasicEng));
		MOB_FIELD(L"Scale", Flt(p->fScale));
		MOB_FIELD(L"Walk_Speed_Origin", Flt(p->fWalk_Speed_Origin));
		MOB_FIELD(L"Walk_Speed", Flt(p->fWalk_Speed));
		MOB_FIELD(L"Run_Speed_Origin", Flt(p->fRun_Speed_Origin));
		MOB_FIELD(L"Run_Speed", Flt(p->fRun_Speed));
		MOB_FIELD(L"Radius_X", Flt(p->fRadius_X));
		MOB_FIELD(L"Radius_Z", Flt(p->fRadius_Z));
		MOB_FIELD(L"Attack_Speed_Rate", DecW(p->wAttack_Speed_Rate));
		MOB_FIELD(L"Attack_Cool_Time", DecW(p->wAttackCoolTime));
		MOB_FIELD(L"Attack_Range", Flt(p->fAttack_Range));
		MOB_FIELD(L"Basic_Attack_Rate", DecW(p->wAttack_Rate));
		MOB_FIELD(L"Basic_Dodge_Rate", DecW(p->wDodge_Rate));
		MOB_FIELD(L"Basic_Block_Rate", DecW(p->wBlock_Rate));
		MOB_FIELD(L"Basic_Curse_Success_Rate", DecW(p->wCurse_Success_Rate));
		MOB_FIELD(L"Basic_Curse_Tolerance_Rate", DecW(p->wCurse_Tolerance_Rate));
		MOB_FIELD(L"Stomachache_Defence", DecW(p->wStomachacheDefence));
		MOB_FIELD(L"Poison_Defence", DecW(p->wPoisonDefence));
		MOB_FIELD(L"Bleed_Defence", DecW(p->wBleedDefence));
		MOB_FIELD(L"Burn_Defence", DecW(p->wBurnDefence));
		MOB_FIELD(L"Sight_Range", DecW(p->wSight_Range));
		MOB_FIELD(L"Scan_Range", DecW(p->wScan_Range));
		MOB_FIELD(L"Attack_Animation_Quantity", DecB(p->byAttack_Animation_Quantity));
		MOB_FIELD(L"Drop_Zenny", Dec(p->dwDrop_Zenny));
		MOB_FIELD(L"Drop_Zenny_Rate", Flt(p->fDrop_Zenny_Rate));
		MOB_FIELD(L"Visible_Sight_Range", DecB(p->byVisible_Sight_Range));
		MOB_FIELD(L"Fly_Height", Flt(p->fFly_Height));
		MOB_FIELD(L"Camera_Bone_Name", AnsiToXml(p->szCamera_Bone_Name));
		MOB_FIELD(L"ILLust", AnsiToXml(p->szILLust));
		MOB_FIELD(L"Size", Bl(p->bSize));
		MOB_FIELD(L"Spawn_Animation", Bl(p->bSpawn_Animation));
		MOB_FIELD(L"Dialog_Group", Dec(p->dwDialogGroup));
		MOB_FIELD(L"TMQ_Point", DecW(p->wTMQPoint));
		MOB_FIELD(L"Drop_Quest_Tblidx", Dec(p->dropQuestTblidx));
		MOB_FIELD(L"idxBigBag1", Dec(p->idxBigBag1));
		MOB_FIELD(L"byDropRate1", DecB(p->byDropRate1));
		MOB_FIELD(L"byTryCount1", DecB(p->byTryCount1));
		MOB_FIELD(L"idxBigBag2", Dec(p->idxBigBag2));
		MOB_FIELD(L"byDropRate2", DecB(p->byDropRate2));
		MOB_FIELD(L"byTryCount2", DecB(p->byTryCount2));
		MOB_FIELD(L"idxBigBag3", Dec(p->idxBigBag3));
		MOB_FIELD(L"byDropRate3", DecB(p->byDropRate3));
		MOB_FIELD(L"byTryCount3", DecB(p->byTryCount3));
		MOB_FIELD(L"Alliance_Idx", Dec(p->dwAllianceIdx));
		MOB_FIELD(L"Aggro_Max_Count", DecW(p->wAggroMaxCount));
		MOB_FIELD(L"Basic_Aggro_Point", DecW(p->wBasic_Aggro_Point));
		MOB_FIELD(L"Sight_Angle", DecW(p->wSightAngle));
		MOB_FIELD(L"Attribute_Bit_Flag", Hex(p->dwNpcAttributeFlag));
		MOB_FIELD(L"Immunity_Bit_Flag", Hex(p->dwImmunity_Bit_Flag));
		MOB_FIELD(L"IsDragonballDrop", Bl(p->bIsDragonBallDrop));
		MOB_FIELD(L"Class", DecW(p->wMonsterClass));
		MOB_FIELD(L"UseRace", DecW(p->wUseRace));
		MOB_FIELD(L"RewardExpRate", Flt(p->fRewardExpRate));
		MOB_FIELD(L"RewardZennyRate", Flt(p->fRewardZennyRate));
		MOB_FIELD(L"FormulaOffset", Dec(p->dwFormulaOffset));
		MOB_FIELD(L"SettingRate_LP", Flt(p->fSettingRate_LP));
		MOB_FIELD(L"SettingRate_LPRegen", Flt(p->fSettingRate_LPRegen));
		MOB_FIELD(L"SettingRate_PhyOffence", Flt(p->fSettingRate_PhyOffence));
		MOB_FIELD(L"SettingRate_EngOffence", Flt(p->fSettingRate_EngOffence));
		MOB_FIELD(L"SettingRate_PhyDefence", Flt(p->fSettingRate_PhyDefence));
		MOB_FIELD(L"SettingRate_EngDefence", Flt(p->fSettingRate_EngDefence));
		MOB_FIELD(L"SettingRate_AttackRate", Flt(p->fSettingRate_AttackRate));
		MOB_FIELD(L"SettingRate_DodgeRate", Flt(p->fSettingRate_DodgeRate));
		MOB_FIELD(L"SettingPhyOffenceRate", Flt(p->fSettingPhyOffenceRate));
		MOB_FIELD(L"SettingEngOffenceRate", Flt(p->fSettingEngOffenceRate));
		MOB_FIELD(L"SettingRate_Defence_Role", Flt(p->fSettingRate_Defence_Role));

		for (int i = 0; i < (int)NTL_MAX_NPC_HAVE_SKILL; ++i)
		{
			CStringW key;
			key.Format(L"Use_Skill_Time_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sMOB_TBLDAT* p = (sMOB_TBLDAT*)pRow; return DecW(p->wUse_Skill_Time[i]); };
			key.Format(L"Use_Skill_Tblidx_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sMOB_TBLDAT* p = (sMOB_TBLDAT*)pRow; return Dec(p->use_Skill_Tblidx[i]); };
			key.Format(L"Use_Skill_Basis_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sMOB_TBLDAT* p = (sMOB_TBLDAT*)pRow; return DecB(p->byUse_Skill_Basis[i]); };
			key.Format(L"Use_Skill_LP_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sMOB_TBLDAT* p = (sMOB_TBLDAT*)pRow; return DecW(p->wUse_Skill_LP[i]); };
		}
	}
	return m;
}
#undef MOB_FIELD

#define DWCMISSION_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sDWCMISSION_TBLDAT* p = (sDWCMISSION_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetDwcMissionExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		DWCMISSION_FIELD(L"Tblidx", Dec(p->tblidx));
	}
	return m;
}
#undef DWCMISSION_FIELD

#define QUESTDROP_FIELD(name, expr) m[name] = [](sTBLDAT* pRow) -> CStringW { sQUEST_DROP_TBLDAT* p = (sQUEST_DROP_TBLDAT*)pRow; return (expr); };

const FieldExtractorMap& GetQuestDropExtractors()
{
	static FieldExtractorMap m;
	if (m.empty())
	{
		QUESTDROP_FIELD(L"Quest_Drop_Tblidx", Dec(p->tblidx));

		for (int i = 0; i < (int)QUEST_ITEM_DROP_MAX_COUNT; ++i)
		{
			CStringW key; key.Format(L"Quest_Item_Index%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_DROP_TBLDAT* p = (sQUEST_DROP_TBLDAT*)pRow; return Dec(p->aQuestItemTblidx[i]); };
		}
		for (int i = 0; i < (int)QUEST_ITEM_DROP_MAX_COUNT; ++i)
		{
			CStringW key; key.Format(L"Drop_Rate_%d", i + 1);
			m[key] = [i](sTBLDAT* pRow) -> CStringW { sQUEST_DROP_TBLDAT* p = (sQUEST_DROP_TBLDAT*)pRow; return Dec(p->aDropRate[i]); };
		}
	}
	return m;
}
#undef QUESTDROP_FIELD

const FieldExtractorMap* GetExtractorsFor(CTableContainer::eTABLE eTable)
{
	switch (eTable)
	{
	case CTableContainer::TABLE_ITEM:				return &GetItemExtractors();
	case CTableContainer::TABLE_NEWBIE:			return &GetNewbieExtractors();
	case CTableContainer::TABLE_MASCOT:			return &GetMascotExtractors();
	case CTableContainer::TABLE_MASCOT_GRADE:		return &GetMascotGradeExtractors();
	case CTableContainer::TABLE_MASCOT_STATUS:		return &GetMascotStatusExtractors();
	case CTableContainer::TABLE_CHARTITLE:			return &GetCharTitleExtractors();
	case CTableContainer::TABLE_CHATTING_FILTER:	return &GetChatFilterExtractors();
	case CTableContainer::TABLE_ITEM_ENCHANT:		return &GetItemEnchantExtractors();
	case CTableContainer::TABLE_MERCHANT:			return &GetMerchantExtractors();
	case CTableContainer::TABLE_FORMULA:			return &GetFormulaExtractors();
	case CTableContainer::TABLE_ITEM_MIX_EXP:		return &GetItemMixExpExtractors();
	case CTableContainer::TABLE_CHAT_COMMAND:		return &GetChatCommandExtractors();
	case CTableContainer::TABLE_VEHICLE:			return &GetVehicleExtractors();
	case CTableContainer::TABLE_DUNGEON:			return &GetDungeonExtractors();
	case CTableContainer::TABLE_DRAGONBALL_REWARD:	return &GetDragonBallRewardExtractors();
	case CTableContainer::TABLE_EXP:				return &GetExpExtractors();
	case CTableContainer::TABLE_CHARM:				return &GetCharmExtractors();
	case CTableContainer::TABLE_ACTION:			return &GetActionExtractors();
	case CTableContainer::TABLE_HELP:				return &GetHelpExtractors();
	case CTableContainer::TABLE_GUIDE_HINT:		return &GetGuideHintExtractors();
	case CTableContainer::TABLE_DYNAMIC_OBJECT:	return &GetDynamicObjectExtractors();
	case CTableContainer::TABLE_USE_ITEM:			return &GetUseItemExtractors();
	case CTableContainer::TABLE_SET_ITEM:			return &GetSetItemExtractors();
	case CTableContainer::TABLE_QUEST_REWARD:		return &GetQuestRewardExtractors();
	case CTableContainer::TABLE_QUEST_REWARD_SELECT: return &GetQuestRewardSelectExtractors();
	case CTableContainer::TABLE_NPC_SERVER:		return &GetNpcServerExtractors();
	case CTableContainer::TABLE_WORLD:				return &GetWorldExtractors();
	case CTableContainer::TABLE_SYSTEM_EFFECT:		return &GetSystemEffectExtractors();
	case CTableContainer::TABLE_ITEM_OPTION:		return &GetItemOptionExtractors();
	case CTableContainer::TABLE_SKILL:				return &GetSkillExtractors();
	case CTableContainer::TABLE_HLS_ITEM:			return &GetHlsItemExtractors();
	case CTableContainer::TABLE_STATUS_TRANSFORM:	return &GetStatusTransformExtractors();
	case CTableContainer::TABLE_QUEST_PROBABILITY:	return &GetQuestProbabilityExtractors();
	case CTableContainer::TABLE_WORLD_PLAY:		return &GetWorldPlayExtractors();
	case CTableContainer::TABLE_HLS_SLOT_MACHINE:	return &GetSlotMachineExtractors();
	case CTableContainer::TABLE_HLS_SLOT_MACHINE_ITEM: return &GetSlotMachineItemExtractors();
	case CTableContainer::TABLE_ITEM_UPGRADE_RATE_NEW: return &GetItemUpgradeRateNewExtractors();
	case CTableContainer::TABLE_ITEM_BAG_LIST:		return &GetItemBagListExtractors();
	case CTableContainer::TABLE_ITEM_GROUP_LIST:	return &GetItemGroupListExtractors();
	case CTableContainer::TABLE_MOB_SERVER:		return &GetMobServerExtractors();
	case CTableContainer::TABLE_DRAGONBALL_RETURN_POINT: return &GetDragonBallReturnPointExtractors();
	case CTableContainer::TABLE_EVENT_SYSTEM:		return &GetEventSystemExtractors();
	case CTableContainer::TABLE_DYNAMIC_FIELD_SYSTEM: return &GetDynamicFieldSystemExtractors();
	case CTableContainer::TABLE_SPEECH:			return &GetSpeechExtractors();
	case CTableContainer::TABLE_HTB_SET:			return &GetHtbSetExtractors();
	case CTableContainer::TABLE_DIRECTION_LINK:	return &GetDirectionLinkExtractors();
	case CTableContainer::TABLE_DOJO:				return &GetDojoExtractors();
	case CTableContainer::TABLE_MOB_MOVE_PATTERN:	return &GetMobMovePatternExtractors();
	case CTableContainer::TABLE_LAND_MARK:			return &GetLandMarkExtractors();
	case CTableContainer::TABLE_AIR_COSTUME:		return &GetAirCostumeExtractors();
	case CTableContainer::TABLE_QUEST_TEXT_DATA:	return &GetQuestTextDataExtractors();
	case CTableContainer::TABLE_TIMEQUEST:			return &GetTimeQuestExtractors();
	case CTableContainer::TABLE_BUDOKAI:			return &GetBudokaiExtractors();
	case CTableContainer::TABLE_RANKBATTLE:		return &GetRankBattleExtractors();
	case CTableContainer::TABLE_SCRIPT_LINK:		return &GetScriptLinkExtractors();
	case CTableContainer::TABLE_QUEST_NARRATION:	return &GetQuestNarrationExtractors();
	case CTableContainer::TABLE_QUEST_ITEM:		return &GetQuestItemExtractors();
	case CTableContainer::TABLE_ITEM_DISASSEMBLE:	return &GetItemDisassembleExtractors();
	case CTableContainer::TABLE_PORTAL:			return &GetPortalExtractors();
	case CTableContainer::TABLE_ITEM_RECIPE:		return &GetItemRecipeExtractors();
	case CTableContainer::TABLE_MIX_MACHINE:		return &GetMixMachineExtractors();
	case CTableContainer::TABLE_DRAGONBALL:		return &GetDragonBallExtractors();
	case CTableContainer::TABLE_WORLD_ZONE:		return &GetWorldZoneExtractors();
	case CTableContainer::TABLE_COMMON_CONFIG:		return &GetCommonConfigExtractors();
	case CTableContainer::TABLE_DWC:				return &GetDwcExtractors();
	case CTableContainer::TABLE_WORLD_MAP:			return &GetWorldMapExtractors();
	case CTableContainer::TABLE_PC:				return &GetPcExtractors();
	case CTableContainer::TABLE_NPC:				return &GetNpcExtractors();
	case CTableContainer::TABLE_MOB:				return &GetMobExtractors();
	case CTableContainer::TABLE_DWCMISSION:		return &GetDwcMissionExtractors();
	case CTableContainer::TABLE_QUEST_DROP:		return &GetQuestDropExtractors();
	default:										return nullptr;
	}
}

CTable* GetTableFor(CTableContainer::eTABLE eTable)
{
	if (!GetTableContainer())
	{
		return nullptr;
	}

	switch (eTable)
	{
	case CTableContainer::TABLE_ITEM:				return GetTableContainer()->GetItemTable();
	case CTableContainer::TABLE_NEWBIE:			return GetTableContainer()->GetNewbieTable();
	case CTableContainer::TABLE_MASCOT:			return GetTableContainer()->GetMascotTable();
	case CTableContainer::TABLE_MASCOT_GRADE:		return GetTableContainer()->GetMascotGradeTable();
	case CTableContainer::TABLE_MASCOT_STATUS:		return GetTableContainer()->GetMascotStatusTable();
	case CTableContainer::TABLE_CHARTITLE:			return GetTableContainer()->GetCharTitleTable();
	case CTableContainer::TABLE_CHATTING_FILTER:	return GetTableContainer()->GetChattingFilterTable();
	case CTableContainer::TABLE_ITEM_ENCHANT:		return GetTableContainer()->GetItemEnchantTable();
	case CTableContainer::TABLE_MERCHANT:			return GetTableContainer()->GetMerchantTable();
	case CTableContainer::TABLE_FORMULA:			return GetTableContainer()->GetFormulaTable();
	case CTableContainer::TABLE_ITEM_MIX_EXP:		return GetTableContainer()->GetItemMixExpTable();
	case CTableContainer::TABLE_CHAT_COMMAND:		return GetTableContainer()->GetChatCommandTable();
	case CTableContainer::TABLE_VEHICLE:			return GetTableContainer()->GetVehicleTable();
	case CTableContainer::TABLE_DUNGEON:			return GetTableContainer()->GetDungeonTable();
	case CTableContainer::TABLE_DRAGONBALL_REWARD:	return GetTableContainer()->GetDragonBallRewardTable();
	case CTableContainer::TABLE_EXP:				return GetTableContainer()->GetExpTable();
	case CTableContainer::TABLE_CHARM:				return GetTableContainer()->GetCharmTable();
	case CTableContainer::TABLE_ACTION:			return GetTableContainer()->GetActionTable();
	case CTableContainer::TABLE_HELP:				return GetTableContainer()->GetHelpTable();
	case CTableContainer::TABLE_GUIDE_HINT:		return GetTableContainer()->GetGuideHintTable();
	case CTableContainer::TABLE_DYNAMIC_OBJECT:	return GetTableContainer()->GetDynamicObjectTable();
	case CTableContainer::TABLE_USE_ITEM:			return GetTableContainer()->GetUseItemTable();
	case CTableContainer::TABLE_SET_ITEM:			return GetTableContainer()->GetSetItemTable();
	case CTableContainer::TABLE_QUEST_REWARD:		return GetTableContainer()->GetQuestRewardTable();
	case CTableContainer::TABLE_QUEST_REWARD_SELECT: return GetTableContainer()->GetQuestRewardSelectTable();
	case CTableContainer::TABLE_NPC_SERVER:		return GetTableContainer()->GetNpcServerTable();
	case CTableContainer::TABLE_WORLD:				return GetTableContainer()->GetWorldTable();
	case CTableContainer::TABLE_SYSTEM_EFFECT:		return GetTableContainer()->GetSystemEffectTable();
	case CTableContainer::TABLE_ITEM_OPTION:		return GetTableContainer()->GetItemOptionTable();
	case CTableContainer::TABLE_SKILL:				return GetTableContainer()->GetSkillTable();
	case CTableContainer::TABLE_HLS_ITEM:			return GetTableContainer()->GetHLSItemTable();
	case CTableContainer::TABLE_STATUS_TRANSFORM:	return GetTableContainer()->GetStatusTransformTable();
	case CTableContainer::TABLE_QUEST_PROBABILITY:	return GetTableContainer()->GetQuestProbabilityTable();
	case CTableContainer::TABLE_WORLD_PLAY:		return GetTableContainer()->GetWorldPlayTable();
	case CTableContainer::TABLE_HLS_SLOT_MACHINE:	return GetTableContainer()->GetSlotMachineTable();
	case CTableContainer::TABLE_HLS_SLOT_MACHINE_ITEM: return GetTableContainer()->GetSlotMachineItemTable();
	case CTableContainer::TABLE_ITEM_UPGRADE_RATE_NEW: return GetTableContainer()->GetItemUpgradeRateNewTable();
	case CTableContainer::TABLE_ITEM_BAG_LIST:		return GetTableContainer()->GetItemBagListTable();
	case CTableContainer::TABLE_ITEM_GROUP_LIST:	return GetTableContainer()->GetItemGroupListTable();
	case CTableContainer::TABLE_MOB_SERVER:		return GetTableContainer()->GetMobServerTable();
	case CTableContainer::TABLE_DRAGONBALL_RETURN_POINT: return GetTableContainer()->GetDragonBallReturnPointTable();
	case CTableContainer::TABLE_EVENT_SYSTEM:		return GetTableContainer()->GetEventSystemTable();
	case CTableContainer::TABLE_DYNAMIC_FIELD_SYSTEM: return GetTableContainer()->GetDynamicFieldSystemTable();
	case CTableContainer::TABLE_SPEECH:			return GetTableContainer()->GetNpcSpeechTable();
	case CTableContainer::TABLE_HTB_SET:			return GetTableContainer()->GetHTBSetTable();
	case CTableContainer::TABLE_DIRECTION_LINK:	return GetTableContainer()->GetDirectionLinkTable();
	case CTableContainer::TABLE_DOJO:				return GetTableContainer()->GetDojoTable();
	case CTableContainer::TABLE_MOB_MOVE_PATTERN:	return GetTableContainer()->GetMobMovePatternTable();
	case CTableContainer::TABLE_LAND_MARK:			return GetTableContainer()->GetLandMarkTable();
	case CTableContainer::TABLE_AIR_COSTUME:		return GetTableContainer()->GetAirCostumeTable();
	case CTableContainer::TABLE_QUEST_TEXT_DATA:	return GetTableContainer()->GetQuestTextDataTable();
	case CTableContainer::TABLE_TIMEQUEST:			return GetTableContainer()->GetTimeQuestTable();
	case CTableContainer::TABLE_BUDOKAI:			return GetTableContainer()->GetBudokaiTable();
	case CTableContainer::TABLE_RANKBATTLE:		return GetTableContainer()->GetRankBattleTable();
	case CTableContainer::TABLE_SCRIPT_LINK:		return GetTableContainer()->GetScriptLinkTable();
	case CTableContainer::TABLE_QUEST_NARRATION:	return GetTableContainer()->GetQuestNarrationTable();
	case CTableContainer::TABLE_QUEST_ITEM:		return GetTableContainer()->GetQuestItemTable();
	case CTableContainer::TABLE_ITEM_DISASSEMBLE:	return GetTableContainer()->GetItemDisassembleTable();
	case CTableContainer::TABLE_PORTAL:			return GetTableContainer()->GetPortalTable();
	case CTableContainer::TABLE_ITEM_RECIPE:		return GetTableContainer()->GetItemRecipeTable();
	case CTableContainer::TABLE_MIX_MACHINE:		return GetTableContainer()->GetItemMixMachineTable();
	case CTableContainer::TABLE_DRAGONBALL:		return GetTableContainer()->GetDragonBallTable();
	case CTableContainer::TABLE_WORLD_ZONE:		return GetTableContainer()->GetWorldZoneTable();
	case CTableContainer::TABLE_COMMON_CONFIG:		return GetTableContainer()->GetCommonConfigTable();
	case CTableContainer::TABLE_DWC:				return GetTableContainer()->GetDwcTable();
	case CTableContainer::TABLE_WORLD_MAP:			return GetTableContainer()->GetWorldMapTable();
	case CTableContainer::TABLE_PC:				return GetTableContainer()->GetPcTable();
	case CTableContainer::TABLE_NPC:				return GetTableContainer()->GetNpcTable();
	case CTableContainer::TABLE_MOB:				return GetTableContainer()->GetMobTable();
	case CTableContainer::TABLE_DWCMISSION:		return GetTableContainer()->GetDwcMissionTable();
	case CTableContainer::TABLE_QUEST_DROP:		return GetTableContainer()->GetQuestDropTable();
	default:										return nullptr;
	}
}

// TextAll is 28 separate CTextTable sub-tables (see CTextAllTable), not one
// flat CTable like every other entry in GetTableFor/GetExtractorsFor above
// -- there's no single sTBLDAT* row sequence to hand the generic
// SaveSingleTableXml loop below. This is the one place that has to know
// that shape; the field list/order still comes from
// schema/table_text_all_data.schema.json like any other table; only the
// "which struct member does this column read" part is hardcoded here,
// because there are just three of them.
bool SaveTextAllXml(const CString& strFolder)
{
	if (!GetTableContainer())
	{
		return false;
	}

	CTextAllTable* pTextAll = GetTableContainer()->GetTextAllTable();
	if (!pTextAll)
	{
		return false;
	}

	STableSchema schema;
	CString strSchemaError;
	if (!LoadTableSchema("Table_Text_All_Data", schema, strSchemaError))
	{
		AfxMessageBox(strSchemaError);
		return false;
	}

	for (const SFieldSchema& field : schema.fields)
	{
		if (field.strName != L"Category" && field.strName != L"Tblidx" && field.strName != L"Text")
		{
			CString strMsg;
			strMsg.Format(_T("Schema field \"%s\" has no matching export code for TextAll. Aborting export."),
				(LPCTSTR)CString(field.strName));
			AfxMessageBox(strMsg);
			return false;
		}
	}

	CString strPath = strFolder + _T("\\Table_Text_All_Data.xml");

	CStringW out;
	out = L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n<dataroot>\r\n";

	std::vector<CStringW> headerCells;
	for (const SFieldSchema& field : schema.fields)
	{
		headerCells.push_back(field.strName);
	}
	AppendRow(out, schema.strSheetName, headerCells);

	std::vector<CStringW> rowCells;
	rowCells.reserve(schema.fields.size());

	const int nCategoryCount = 28;
	for (int nCategory = 0; nCategory < nCategoryCount; ++nCategory)
	{
		CTextTable* pSub = pTextAll->GetTextTbl((CTextAllTable::TABLETYPE)nCategory);
		if (!pSub)
		{
			continue;
		}

		CStringW strCategory = WideToXml(GetTextAllCategoryName(nCategory));

		for (CTable::TABLEIT it = pSub->Begin(); it != pSub->End(); ++it)
		{
			sTEXT_TBLDAT* pRow = (sTEXT_TBLDAT*)it->second;

			rowCells.clear();
			for (const SFieldSchema& field : schema.fields)
			{
				if (field.strName == L"Category")
				{
					rowCells.push_back(strCategory);
				}
				else if (field.strName == L"Tblidx")
				{
					rowCells.push_back(Dec(pRow->tblidx));
				}
				else // "Text", already validated above
				{
					rowCells.push_back(WideToXml(pRow->wstrText.c_str()));
				}
			}

			AppendRow(out, schema.strSheetName, rowCells);
		}
	}

	out += L"</dataroot>\r\n";

	return WriteUtf16File(strPath, out);
}

} // namespace


CString GetXmlExportCaveat(CTableContainer::eTABLE eTable)
{
	if (eTable == CTableContainer::TABLE_TEXT_ALL)
	{
		return _T("Note: this file merges all 28 TextAll categories into one, tagged by a ")
			_T("Category column. The real XML reader (CTextTable::InitializeFromXmlDoc) is ")
			_T("positional per-category (/dataroot/Sheet/F<n>, matched by field index, one ")
			_T("category per file) and has no concept of a combined multi-category file or a ")
			_T("Category column -- it never calls the named-field SetTableData overload at all ")
			_T("(that overload is a no-op stub). This export is for readability/interop only and ")
			_T("won't be recognized if fed back through the real engine's XML loader.");
	}

	switch (eTable)
	{
	case CTableContainer::TABLE_ITEM:
		return _T("Note: byScouter_Parts_Type1-4 and byUseDisassemble have no XML column ")
			_T("in the table engine's reader -- they'll come back as uninitialized data ")
			_T("if this file is loaded back. This is a pre-existing gap in the engine's ")
			_T("XML reader, not something specific to this export.");

	case CTableContainer::TABLE_NEWBIE:
		return _T("Note: vSpawn_Dir.y, vBind_Dir.y, defaultPortalId[0..2], and wMixLevelData ")
			_T("have no XML column in the table engine's reader -- they'll come back as ")
			_T("uninitialized data if this file is loaded back. This is a pre-existing gap ")
			_T("in the engine's XML reader, not something specific to this export.");

	case CTableContainer::TABLE_MASCOT:
	case CTableContainer::TABLE_MASCOT_GRADE:
	case CTableContainer::TABLE_MASCOT_STATUS:
	case CTableContainer::TABLE_ITEM_ENCHANT:
		return _T("IMPORTANT: this table's XML reader (SetTableData) only recognizes the ")
			_T("Tblidx column -- every other field errors out on import (\"Unknown field ")
			_T("name found!\"). This export only writes Tblidx; every other value is NOT in ")
			_T("this file and cannot be. Use RDF for this table -- XML can't carry its data.");

	case CTableContainer::TABLE_CHARTITLE:
		return _T("Note: System_Effect_Value1-3 are doubles in the struct, but the engine's ")
			_T("reader parses that column with READ_BYTE (a pre-existing bug, not this ")
			_T("export's doing) -- values are written truncated to a whole number 0-255. ")
			_T("Fractional or larger values will not round-trip through XML.");

	case CTableContainer::TABLE_ITEM_MIX_EXP:
		return _T("IMPORTANT: this table's XML reader (SetTableData) only recognizes the ")
			_T("Tblidx column -- dwNeedEXP and byUnknown both error out on import (\"Unknown ")
			_T("field name found!\"). This export only writes Tblidx. Use RDF for this table ")
			_T("-- XML can't carry its data.");

	case CTableContainer::TABLE_FORMULA:
		return _T("Note: bValidity_Able has no XML column in the table engine's reader -- ")
			_T("it'll come back as uninitialized data if this file is loaded back. This is a ")
			_T("pre-existing gap in the engine's XML reader, not something specific to this ")
			_T("export.");

	case CTableContainer::TABLE_EXP:
		return _T("Note: the individual/team rank-point fields have no XML column in the ")
			_T("table engine's reader -- they'll come back as uninitialized data if this ")
			_T("file is loaded back. This is a pre-existing gap in the engine's XML reader, ")
			_T("not something specific to this export.");

	case CTableContainer::TABLE_QUEST_REWARD_SELECT:
	case CTableContainer::TABLE_NPC_SERVER:
	case CTableContainer::TABLE_ITEM_DISASSEMBLE:
	case CTableContainer::TABLE_COMMON_CONFIG:
	case CTableContainer::TABLE_AIR_COSTUME:
		return _T("IMPORTANT: this table's XML reader (SetTableData) only recognizes the ")
			_T("Tblidx column -- every other field errors out on import (\"Unknown field ")
			_T("name found!\"). This export only writes Tblidx; every other value is NOT in ")
			_T("this file and cannot be. Use RDF for this table -- XML can't carry its data.");

	case CTableContainer::TABLE_MOB_MOVE_PATTERN:
		return _T("Note: the Note field is recognized by the table engine's reader but its ")
			_T("assignment is commented out (a pre-existing no-op) -- it is not exported here ")
			_T("since there is nothing to read back.");

	case CTableContainer::TABLE_LAND_MARK:
		return _T("Note: LandmarkLoc.y has no XML column in the table engine's reader -- only ")
			_T("X and Z round-trip through XML. This is a pre-existing gap in the engine's ")
			_T("reader, not something specific to this export.");

	case CTableContainer::TABLE_BUDOKAI:
		return _T("Note: the Note field is recognized by the table engine's reader but its ")
			_T("assignment is commented out (a pre-existing no-op) -- it is not exported here. ")
			_T("Also, all of this table's real configuration (rules, timings, rewards, etc.) ")
			_T("lives in a separate binary-only sBUDOKAI_TBLINFO block that has no XML reader at ")
			_T("all -- only the generic Name/Value1-10 columns round-trip through XML.");

	case CTableContainer::TABLE_TIMEQUEST:
		return _T("Note: Name_Easy has no XML column in the table engine's reader (only ")
			_T("Name_Normal and Name_Hard do) -- it'll come back as uninitialized data if this ")
			_T("file is loaded back. This is a pre-existing gap in the engine's XML reader, not ")
			_T("something specific to this export.");

	case CTableContainer::TABLE_RANKBATTLE:
		return _T("Note: byMatchType, wszBGMName, and szCameraName have no XML column in the ")
			_T("table engine's reader -- they'll come back as uninitialized data if this file is ")
			_T("loaded back. This is a pre-existing gap in the engine's XML reader, not something ")
			_T("specific to this export.");

	case CTableContainer::TABLE_PC:
		return _T("Note: the Child/Adult Fly and Dash and Accel speed fields, dwWeightLimit, and ")
			_T("fRadius have no XML column in the table engine's reader (fRadius's reader branch ")
			_T("is commented out) -- they'll come back as uninitialized data if this file is ")
			_T("loaded back. This is a pre-existing gap in the engine's XML reader, not something ")
			_T("specific to this export.");

	case CTableContainer::TABLE_NPC:
		return _T("Note: byGrade, byAttack_Animation_Quantity, dwNpcAttributeFlag, ")
			_T("wStomachacheDefence/wPoisonDefence/wBleedDefence/wBurnDefence, the legacy ")
			_T("szNameText char buffer, and wUnknown/dwUnknown2/dwUnknown3 have no XML column in ")
			_T("the table engine's reader (Mob's reader recognizes some of these; NPC's does ")
			_T("not) -- they'll come back as uninitialized data if this file is loaded back. This ")
			_T("is a pre-existing gap in the engine's XML reader, not something specific to this ")
			_T("export.");

	case CTableContainer::TABLE_MOB:
		return _T("Note: drop_Item_Tblidx, dwUnknown, byUnknown, and byUnknown2 have no XML ")
			_T("column in the table engine's reader (drop_Item_Tblidx's reader branch is ")
			_T("commented out) -- they'll come back as uninitialized data if this file is loaded ")
			_T("back. bShow_Name's column is recognized but its assignment is commented out (a ")
			_T("pre-existing no-op), so it is not exported. Also, the Big_Bag2/Big_Bag3 XML ")
			_T("columns (idxBigBag2/3, byDropRate2/3, byTryCount2/3) all write into the ")
			_T("idxBigBag1/byDropRate1/byTryCount1 fields instead of their own (a pre-existing ")
			_T("copy/paste bug in the reader) -- this export still writes each field's own ")
			_T("current value under its own column, but reloading this file through the real ")
			_T("engine will collapse them back into slot 1.");

	case CTableContainer::TABLE_WORLD_MAP:
		return _T("Note: vUiModify has no XML column in the table engine's reader -- it'll come ")
			_T("back as uninitialized data if this file is loaded back. This is a pre-existing ")
			_T("gap in the engine's XML reader, not something specific to this export.");

	case CTableContainer::TABLE_WORLD:
		return _T("Note: dwDynamicCreateCountShareGroup, vBattleStart2Loc, vBattleEnd2Loc, both ")
			_T("vWaitingPoint1/2 Loc/Dir, and dwProhibition_Bit_Flag have no XML column in the ")
			_T("table engine's reader. outWorldDir.z also can't be set independently -- both ")
			_T("Out_Field_Dir_X and Out_Field_Dir_Z write to outWorldDir.x (a copy/paste bug in ")
			_T("the reader), so only Out_Field_Dir_X is exported. These are pre-existing gaps in ")
			_T("the engine's XML reader, not something specific to this export.");

	case CTableContainer::TABLE_SKILL:
		return _T("Note: byClass_Type, bySkill_Group, dwRequire_VP, and ")
			_T("dwUse_Restriction_Rule_Bit_Flag have no XML column in the table engine's reader ")
			_T("-- they'll come back as uninitialized data if this file is loaded back. This is ")
			_T("a pre-existing gap in the engine's XML reader, not something specific to this ")
			_T("export.");

	case CTableContainer::TABLE_HLS_ITEM:
		return _T("Note: byCategory has no XML column in the table engine's reader -- it'll ")
			_T("come back as uninitialized data if this file is loaded back. This is a ")
			_T("pre-existing gap in the engine's XML reader, not something specific to this ")
			_T("export.");

	case CTableContainer::TABLE_STATUS_TRANSFORM:
		return _T("Note: dwDurationInMilliSecs has no XML column of its own -- the reader ")
			_T("derives it inline from Duration (* 1000), so it's not exported separately here ")
			_T("either.");

	case CTableContainer::TABLE_HLS_SLOT_MACHINE:
	case CTableContainer::TABLE_HLS_SLOT_MACHINE_ITEM:
	case CTableContainer::TABLE_ITEM_UPGRADE_RATE_NEW:
	case CTableContainer::TABLE_ITEM_BAG_LIST:
	case CTableContainer::TABLE_ITEM_GROUP_LIST:
	case CTableContainer::TABLE_MOB_SERVER:
	case CTableContainer::TABLE_DRAGONBALL_RETURN_POINT:
	case CTableContainer::TABLE_EVENT_SYSTEM:
	case CTableContainer::TABLE_DYNAMIC_FIELD_SYSTEM:
	case CTableContainer::TABLE_DWCMISSION:
		return _T("IMPORTANT: this table's XML reader (SetTableData) only recognizes the ")
			_T("Tblidx column -- every other field errors out on import (\"Unknown field ")
			_T("name found!\"). This export only writes Tblidx; every other value is NOT in ")
			_T("this file and cannot be. Use RDF for this table -- XML can't carry its data.");

	case CTableContainer::TABLE_QUEST_PROBABILITY:
		return _T("Note: eUseType and byCount have no XML column in the table engine's reader ")
			_T("-- they'll come back as uninitialized data if this file is loaded back. This is ")
			_T("a pre-existing gap in the engine's XML reader, not something specific to this ")
			_T("export.");

	default:
		return CString();
	}
}

bool SaveSingleTableXml(CTableContainer::eTABLE eTable, const CString& strFolder)
{
	if (eTable == CTableContainer::TABLE_TEXT_ALL)
	{
		return SaveTextAllXml(strFolder);
	}

	CTable* pTable = GetTableFor(eTable);
	const FieldExtractorMap* pExtractors = GetExtractorsFor(eTable);

	if (!pTable || !pExtractors)
	{
		return false;
	}

	int nCount = 0;
	const STableInfo* pTables = GetRegisteredTables(nCount);
	const char* pszFileName = nullptr;
	for (int i = 0; i < nCount; ++i)
	{
		if (pTables[i].eType == eTable)
		{
			pszFileName = pTables[i].pszFileName;
			break;
		}
	}
	if (!pszFileName)
	{
		return false;
	}
	CString strFileName(pszFileName);

	STableSchema schema;
	CString strSchemaError;
	if (!LoadTableSchema(pszFileName, schema, strSchemaError))
	{
		AfxMessageBox(strSchemaError);
		return false;
	}

	// Fail loudly rather than silently drop a field: a schema entry with
	// no matching C++ extractor would otherwise export a file that's
	// short one column, corrupting every column after it on reload
	// (columns are matched by position, not name -- see AppendRow).
	for (const SFieldSchema& field : schema.fields)
	{
		if (pExtractors->find(field.strName) == pExtractors->end())
		{
			CString strMsg;
			strMsg.Format(_T("Schema field \"%s\" has no matching export code for this table. Aborting export."),
				(LPCTSTR)CString(field.strName));
			AfxMessageBox(strMsg);
			return false;
		}
	}

	CString strPath = strFolder + _T("\\") + strFileName + _T(".xml");

	CStringW out;
	out = L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n<dataroot>\r\n";

	std::vector<CStringW> headerCells;
	for (const SFieldSchema& field : schema.fields)
	{
		headerCells.push_back(field.strName);
	}
	AppendRow(out, schema.strSheetName, headerCells);

	std::vector<CStringW> rowCells;
	rowCells.reserve(schema.fields.size());

	for (CTable::TABLEIT it = pTable->Begin(); it != pTable->End(); ++it)
	{
		sTBLDAT* pRow = it->second;

		rowCells.clear();
		for (const SFieldSchema& field : schema.fields)
		{
			CStringW strValue = pExtractors->at(field.strName)(pRow);

			CString strValidateError;
			if (!field.strType.IsEmpty() && !ValidateFieldValue(field.strType, strValue, strValidateError))
			{
				CString strMsg;
				strMsg.Format(_T("Row tblidx=%u, field \"%s\": %s. Aborting export."),
					pRow->tblidx, (LPCTSTR)CString(field.strName), (LPCTSTR)strValidateError);
				AfxMessageBox(strMsg);
				return false;
			}

			rowCells.push_back(strValue);
		}

		AppendRow(out, schema.strSheetName, rowCells);
	}

	out += L"</dataroot>\r\n";

	return WriteUtf16File(strPath, out);
}
