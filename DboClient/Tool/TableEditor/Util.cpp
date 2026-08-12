#include "pch.h"
#include "Util.h"
#include "TableFileNameList.h"
#include "NtlXMLDoc.h"
#include "NtlBitFlagManager.h"

CTableContainer* m_pTableContainer = nullptr;

// Tables this tool currently knows how to display/edit. Add an entry here
// (and the matching sub-item list in CPropertiesWnd::LoadTableData) to wire
// up another table.
static const STableInfo s_aRegisteredTables[] =
{
	{ CTableContainer::TABLE_ITEM,			"Table_Item_Data",				_T("Item"),        true  },
	{ CTableContainer::TABLE_NEWBIE,		"Table_Newbie_Data",			_T("Newbie"),      true  },
	{ CTableContainer::TABLE_SPEECH,		"Table_NPC_Speech_Data",		_T("Speech"),      true  },
	{ CTableContainer::TABLE_MASCOT,		"table_mascot_data",			_T("Mascot"),      true  },
	{ CTableContainer::TABLE_MASCOT_GRADE,	"Table_Mascot_Grade_Data",		_T("MascotGrade"), true  },
	{ CTableContainer::TABLE_MASCOT_STATUS,"Table_Mascot_Status_Data",	_T("MascotStatus"),true  },
	{ CTableContainer::TABLE_CHARTITLE,		"Table_CharTitle_Data",			_T("CharTitle"),   true  },
	{ CTableContainer::TABLE_CHATTING_FILTER,"table_chatting_filter_data",	_T("ChatFilter"),  true  },
	{ CTableContainer::TABLE_ITEM_ENCHANT,	"Table_Item_Enchant_Data",		_T("ItemEnchant"), true  },
	{ CTableContainer::TABLE_MERCHANT,		"Table_Merchant_Data",			_T("Merchant"),    true  },
	{ CTableContainer::TABLE_FORMULA,		"TD_Formula",					_T("Formula"),      true  },
	{ CTableContainer::TABLE_ITEM_MIX_EXP,	"Table_Item_Mix_Exp_Data",		_T("ItemMixExp"),   true  },
	{ CTableContainer::TABLE_CHAT_COMMAND,	"Table_Chat_Command_Data",		_T("ChatCommand"),  true  },
	{ CTableContainer::TABLE_HTB_SET,		"Table_HTB_Set_Data",			_T("HtbSet"),       true  },
	{ CTableContainer::TABLE_DIRECTION_LINK,"Table_Direction_Link_Data",	_T("DirectionLink"),true  },
	{ CTableContainer::TABLE_DOJO,			"Table_Dojo_Data",				_T("Dojo"),         true  },
	{ CTableContainer::TABLE_VEHICLE,		"Table_Vehicle_Data",			_T("Vehicle"),      true  },
	{ CTableContainer::TABLE_DUNGEON,		"Table_Dungeon_Data",			_T("Dungeon"),      true  },
	{ CTableContainer::TABLE_MOB_MOVE_PATTERN,"Table_Mob_Move_Pattern_Data",_T("MobMovePattern"),true },
	{ CTableContainer::TABLE_DRAGONBALL_REWARD,"Table_DB_Reward_Data",		_T("DbReward"),     true  },
	{ CTableContainer::TABLE_EXP,			"table_exp_data",				_T("Exp"),          true  },
	{ CTableContainer::TABLE_LAND_MARK,		"Table_Landmark_Data",			_T("LandMark"),     true  },
	{ CTableContainer::TABLE_CHARM,			"Table_Charm_Data",				_T("Charm"),        true  },
	{ CTableContainer::TABLE_ACTION,		"Table_Action_Data",			_T("Action"),       true  },
	{ CTableContainer::TABLE_AIR_COSTUME,	"Table_Aircostume_Data",		_T("AirCostume"),   true  },
	{ CTableContainer::TABLE_QUEST_TEXT_DATA,"Table_Quest_Text_Data",		_T("QuestText"),    true  },
	{ CTableContainer::TABLE_HELP,			"Table_Help_Data",				_T("Help"),         true  },
	{ CTableContainer::TABLE_GUIDE_HINT,	"Table_Guide_Hint_Data",		_T("GuideHint"),    true  },
	{ CTableContainer::TABLE_TIMEQUEST,		"Table_TMQ_Data",				_T("TimeQuest"),    true  },
	{ CTableContainer::TABLE_BUDOKAI,		"Table_Tenkaichibudokai_Data",	_T("Budokai"),      true  },
	{ CTableContainer::TABLE_RANKBATTLE,	"Table_RankBattle_Data",		_T("RankBattle"),   true  },
	{ CTableContainer::TABLE_SCRIPT_LINK,	"Table_Script_Link_Data",		_T("ScriptLink"),   true  },
	{ CTableContainer::TABLE_QUEST_NARRATION,"Table_Quest_Narration_Data",	_T("QuestNarration"),true },
	{ CTableContainer::TABLE_DYNAMIC_OBJECT,"Table_Dynamic_Object_Data",	_T("DynamicObject"),true  },
	{ CTableContainer::TABLE_USE_ITEM,		"Table_Use_Item_Data",			_T("UseItem"),      true  },
	{ CTableContainer::TABLE_SET_ITEM,		"Table_Set_Item_Data",			_T("SetItem"),      true  },
	{ CTableContainer::TABLE_QUEST_ITEM,	"Table_Quest_Item_Data",		_T("QuestItem"),    true  },
	{ CTableContainer::TABLE_ITEM_DISASSEMBLE,"table_item_disassemble_data",_T("ItemDisassemble"),true },
	{ CTableContainer::TABLE_PORTAL,		"Table_Portal_Data",			_T("Portal"),       true  },
	{ CTableContainer::TABLE_ITEM_RECIPE,	"Table_Item_Recipe_Data",		_T("ItemRecipe"),   true  },
	{ CTableContainer::TABLE_MIX_MACHINE,	"Table_Item_Mix_Machine_Data",	_T("MixMachine"),   true  },
	{ CTableContainer::TABLE_DRAGONBALL,	"Table_Dragon_Ball_Data",		_T("DragonBall"),   true  },
	{ CTableContainer::TABLE_QUEST_REWARD,	"Table_Quest_Reward_Data",		_T("QuestReward"),  true  },
	{ CTableContainer::TABLE_QUEST_REWARD_SELECT,"Table_Quest_Reward_Select_Data",_T("QuestRewardSelect"),true },
	{ CTableContainer::TABLE_WORLD_ZONE,	"Table_World_Zone_Data",		_T("WorldZone"),    true  },
	{ CTableContainer::TABLE_COMMON_CONFIG,"table_common_config_data",	_T("CommonConfig"), true  },
	{ CTableContainer::TABLE_DWC,			"table_dwc_data",				_T("Dwc"),          true  },
	{ CTableContainer::TABLE_NPC_SERVER,	"table_npc_data_server",		_T("NpcServer"),    true  },
	{ CTableContainer::TABLE_WORLD,			"Table_World_Data",				_T("World"),        true  },
	{ CTableContainer::TABLE_WORLD_MAP,		"Table_Worldmap_Data",			_T("WorldMap"),     true  },
	{ CTableContainer::TABLE_SYSTEM_EFFECT,	"Table_System_Effect_Data",		_T("SystemEffect"), true  },
	{ CTableContainer::TABLE_ITEM_OPTION,	"Table_Item_Option_Data",		_T("ItemOption"),   true  },
	{ CTableContainer::TABLE_SKILL,			"Table_Skill_Data",				_T("Skill"),        true  },
	{ CTableContainer::TABLE_PC,				"Table_PC_Data",					_T("PC"),           true  },
	{ CTableContainer::TABLE_MOB,			"Table_MOB_Data",				_T("Mob"),          true  },
	{ CTableContainer::TABLE_NPC,			"Table_NPC_Data",				_T("Npc"),          true  },
	{ CTableContainer::TABLE_HLS_ITEM,		"table_hls_item_data",			_T("HlsItem"),      true  },
	{ CTableContainer::TABLE_STATUS_TRANSFORM,"table_status_transform_data",_T("StatusTransform"),true },
	{ CTableContainer::TABLE_DWCMISSION,	"table_dwc_mission_data",		_T("DwcMission"),   true  },
	{ CTableContainer::TABLE_QUEST_DROP,	"table_quest_drop_data",		_T("QuestDrop"),    true  },
	{ CTableContainer::TABLE_QUEST_PROBABILITY,"table_quest_probability_data",_T("QuestProbability"),true },
	{ CTableContainer::TABLE_WORLD_PLAY,	"table_world_play_data",		_T("WorldPlay"),    true  },
	{ CTableContainer::TABLE_HLS_SLOT_MACHINE,"table_slot_machine_data",	_T("SlotMachine"),  true  },
	{ CTableContainer::TABLE_HLS_SLOT_MACHINE_ITEM,"table_slot_machine_item_data",_T("SlotMachineItem"),true },
	{ CTableContainer::TABLE_ITEM_UPGRADE_RATE_NEW,"table_item_upgrade_newrate_data",_T("ItemUpgradeRate"),true },
	{ CTableContainer::TABLE_ITEM_BAG_LIST,	"table_item_bag_list_data",	_T("ItemBagList"),  true  },
	{ CTableContainer::TABLE_ITEM_GROUP_LIST,	"table_item_group_list_data",	_T("ItemGroupList"),true  },
	{ CTableContainer::TABLE_MOB_SERVER,	"table_mob_data_server",		_T("MobServer"),    true  },
	{ CTableContainer::TABLE_DRAGONBALL_RETURN_POINT,"table_db_returnpoint",_T("DbReturnPoint"),true },
	{ CTableContainer::TABLE_EVENT_SYSTEM,	"table_event_system_data",		_T("EventSystem"),  true  },
	{ CTableContainer::TABLE_DYNAMIC_FIELD_SYSTEM,"table_event_system_dynamic_data",_T("DynamicField"),true },
	// Also supports item-name translation lookups (see
	// CClassView::LoadTableData). Not itself a per-row sTBLDAT table --
	// it's 28 separate id->text lookup tables -- so its one tab shows a
	// two-level tree (28 categories, lazily expanded into rows) instead of
	// a flat row list; see CClassView::LoadTableData/OnTreeItemExpanding.
	// "Save as XML" for this tab merges all 28 categories into one file,
	// tagged by category (see XmlExport.cpp's SaveTextAllXml).
	{ CTableContainer::TABLE_TEXT_ALL,		"Table_Text_All_Data",			_T("TextAll"),     true  },

	// Not real single-file tables -- each per-World spawn file is loaded
	// separately, keyed by World tblidx (see CTableContainer::Create's
	// TABLE_OBJECT/TABLE_NPC_SPAWN/TABLE_MOB_SPAWN handling, which reads the
	// filename from each World row instead of pszFileName below). These
	// entries exist only so CreateTableContainer's flagManager.Set() turns
	// the load on; bBrowsable=false keeps CFileView::RefreshTables from
	// generating a bogus tab for them (spawn tabs come from AddSpawnTab
	// instead, once per World that references a spawn file).
	{ CTableContainer::TABLE_OBJECT,		"",								_T(""),            false },
	{ CTableContainer::TABLE_NPC_SPAWN,		"",								_T(""),            false },
	{ CTableContainer::TABLE_MOB_SPAWN,		"",								_T(""),            false },
};

const STableInfo* GetRegisteredTables(int& nCount)
{
	nCount = sizeof(s_aRegisteredTables) / sizeof(s_aRegisteredTables[0]);
	return s_aRegisteredTables;
}

// Display names for CTextAllTable::TABLETYPE, in enum declaration order
// (see DboShared/NtlGameTable/TextAllTable.h) -- position in this array IS
// the TABLETYPE value.
static const LPCTSTR s_apszTextAllCategoryNames[] =
{
	_T("Action"), _T("ChatCommand"), _T("HtbSet"), _T("Item"), _T("Merchant"),
	_T("Mob"), _T("Npc"), _T("Skill"), _T("SystemEffect"), _T("UseItem"),
	_T("MapName"), _T("Object"), _T("QuestItem"), _T("Etc"), _T("Help"),
	_T("HelpPopoHint"), _T("DragonBall"), _T("DbReward"), _T("Tmq"), _T("CsText"),
	_T("Milepost"), _T("Filtering"), _T("NpcDialog"), _T("GmTool"), _T("DboTip"),
	_T("Dwc"), _T("CharTitle"), _T("PartyDungeon"),
};

LPCTSTR GetTextAllCategoryName(int nCategory)
{
	static const int nCategoryCount = (int)(sizeof(s_apszTextAllCategoryNames) / sizeof(s_apszTextAllCategoryNames[0]));
	if (nCategory < 0 || nCategory >= nCategoryCount)
	{
		return _T("");
	}
	return s_apszTextAllCategoryNames[nCategory];
}

bool CreateTableContainer(const char* pPath, CTable::eLOADING_METHOD eLoadingMethod)
{
	CNtlBitFlagManager flagManager;
	if (false == flagManager.Create(CTableContainer::TABLE_COUNT))
	{
		return false;
	}

	CTableFileNameList fileNameList;
	if (false == fileNameList.Create())
	{
		return false;
	}

	int nCount = 0;
	const STableInfo* pTables = GetRegisteredTables(nCount);

	for (int i = 0; i < nCount; ++i)
	{
		flagManager.Set(pTables[i].eType);
		fileNameList.SetFileName(pTables[i].eType, (char*)pTables[i].pszFileName);
	}

	m_pTableContainer = new CTableContainer;

	if (!m_pTableContainer->Create(flagManager, (char*)pPath, &fileNameList, eLoadingMethod, GetACP(), nullptr))
	{
		delete m_pTableContainer;
		m_pTableContainer = nullptr;

		return false;
	}

	return true;
}

bool SaveTableContainer(const char* pPath, bool bNeedToEncrypt)
{
	if (!m_pTableContainer)
	{
		return false;
	}

	CNtlBitFlagManager flagManager;
	if (false == flagManager.Create(CTableContainer::TABLE_COUNT))
	{
		return false;
	}

	CTableFileNameList fileNameList;
	if (false == fileNameList.Create())
	{
		return false;
	}

	int nCount = 0;
	const STableInfo* pTables = GetRegisteredTables(nCount);

	for (int i = 0; i < nCount; ++i)
	{
		flagManager.Set(pTables[i].eType);
		fileNameList.SetFileName(pTables[i].eType, (char*)pTables[i].pszFileName);
	}

	m_pTableContainer->SetPath((char*)pPath);

	return m_pTableContainer->SaveToFile(flagManager, &fileNameList, bNeedToEncrypt);
}

bool SaveSingleTableRdf(CTableContainer::eTABLE eTable, const char* pPath, bool bNeedToEncrypt)
{
	if (!m_pTableContainer)
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

	CNtlBitFlagManager flagManager;
	if (false == flagManager.Create(CTableContainer::TABLE_COUNT))
	{
		return false;
	}

	CTableFileNameList fileNameList;
	if (false == fileNameList.Create())
	{
		return false;
	}

	// Only this one table's flag is set, so SaveToFile's per-table
	// "if (rTableFlag.IsSet(...))" checks skip every other table.
	flagManager.Set(eTable);
	fileNameList.SetFileName(eTable, (char*)pszFileName);

	m_pTableContainer->SetPath((char*)pPath);

	return m_pTableContainer->SaveToFile(flagManager, &fileNameList, bNeedToEncrypt);
}

void DeleteTableContainer()
{
	if (m_pTableContainer)
	{
		delete m_pTableContainer;
		m_pTableContainer = nullptr;
	}
}

CTableContainer* GetTableContainer()
{
	return m_pTableContainer;
}
