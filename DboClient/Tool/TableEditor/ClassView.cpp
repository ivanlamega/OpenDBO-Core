#include "pch.h"
#include "framework.h"
#include "MainFrm.h"
#include "ClassView.h"
#include "Resource.h"
#include "TableEditor.h"

#include "Util.h"
#include "ItemTable.h"
#include "NewbieTable.h"
#include "NpcSpeechTable.h"
#include "TextAllTable.h"
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
#include "SpawnTable.h"
#include "ObjectTable.h"
#include "Theme.h"

#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

#define IDC_SEARCH_EDIT			2011
#define IDC_SEARCH_ACCURATE		2012
#define IDC_SEARCH_PREV			2013
#define IDC_SEARCH_NEXT			2014

// Display names for CTextAllTable::TABLETYPE, in enum declaration order
// (see DboShared/NtlGameTable/TextAllTable.h) -- position in this array IS
// the TABLETYPE value.
static const LPCTSTR s_apszTextAllCategories[] =
{
	_T("Action"), _T("ChatCommand"), _T("HtbSet"), _T("Item"), _T("Merchant"),
	_T("Mob"), _T("Npc"), _T("Skill"), _T("SystemEffect"), _T("UseItem"),
	_T("MapName"), _T("Object"), _T("QuestItem"), _T("Etc"), _T("Help"),
	_T("HelpPopoHint"), _T("DragonBall"), _T("DbReward"), _T("Tmq"), _T("CsText"),
	_T("Milepost"), _T("Filtering"), _T("NpcDialog"), _T("GmTool"), _T("DboTip"),
	_T("Dwc"), _T("CharTitle"), _T("PartyDungeon"),
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClassView::CClassView()
{
	m_nTableType = -1;
	m_nDefaultIconIndex = -1;
	m_nMatchIndex = -1;
}

CClassView::~CClassView()
{
}

BEGIN_MESSAGE_MAP(CClassView, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_NOTIFY(TVN_SELCHANGED, 2, OnTvnSelChanged)
	ON_NOTIFY(NM_CUSTOMDRAW, 2, OnTreeCustomDraw)
	ON_NOTIFY(TVN_ITEMEXPANDING, 2, OnTreeItemExpanding)
	ON_EN_CHANGE(IDC_SEARCH_EDIT, OnSearchTextChanged)
	ON_BN_CLICKED(IDC_SEARCH_ACCURATE, OnAccurateClicked)
	ON_BN_CLICKED(IDC_SEARCH_PREV, OnPrevClicked)
	ON_BN_CLICKED(IDC_SEARCH_NEXT, OnNextClicked)
END_MESSAGE_MAP()

BOOL CClassView::Create(CWnd* pParentWnd, UINT nID)
{
	static CString strClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
		::LoadCursor(nullptr, IDC_ARROW), (HBRUSH)nullptr, nullptr);

	return CWnd::Create(strClass, _T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), pParentWnd, nID);
}

/////////////////////////////////////////////////////////////////////////////
// CClassView message handlers

int CClassView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_brEditBg.CreateSolidBrush(Theme::Bg2);
	m_brPanelBg.CreateSolidBrush(Theme::Bg1);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_editSearch.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, rectDummy, this, IDC_SEARCH_EDIT))
	{
		TRACE0("Failed to create search edit\n");
		return -1;
	}

	if (!m_chkAccurate.Create(_T("Accurate"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, rectDummy, this, IDC_SEARCH_ACCURATE))
	{
		TRACE0("Failed to create Accurate checkbox\n");
		return -1;
	}

	// A themed checkbox paints its own label using system colors and
	// ignores the text color OnCtlColor below returns, which is why it
	// read as unreadable black-on-dark -- disabling its visual style
	// switches it to classic rendering, which does respect OnCtlColor.
	::SetWindowTheme(m_chkAccurate.GetSafeHwnd(), L"", L"");

	if (!m_btnPrev.Create(_T("Prev"), WS_CHILD | WS_VISIBLE, rectDummy, this, IDC_SEARCH_PREV))
	{
		TRACE0("Failed to create Prev button\n");
		return -1;
	}

	if (!m_btnNext.Create(_T("Next"), WS_CHILD | WS_VISIBLE, rectDummy, this, IDC_SEARCH_NEXT))
	{
		TRACE0("Failed to create Next button\n");
		return -1;
	}

	m_editSearch.SetFont(&afxGlobalData.fontRegular);
	m_chkAccurate.SetFont(&afxGlobalData.fontRegular);
	m_btnPrev.SetFont(&afxGlobalData.fontRegular);
	m_btnNext.SetFont(&afxGlobalData.fontRegular);

	// Create view -- no HASLINES/HASBUTTONS: this is a flat list of rows,
	// not a real hierarchy, and the dotted connector lines + [+]/[-]
	// boxes are exactly the dated "Windows Explorer, 2000" tree look.
	// FULLROWSELECT gives the modern full-width row highlight instead of
	// just the text getting highlighted.
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_FULLROWSELECT | TVS_SHOWSELALWAYS | TVS_NONEVENHEIGHT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndClassView.Create(dwViewStyle, rectDummy, this, 2))
	{
		TRACE0("Failed to create Class View\n");
		return -1;      // fail to create
	}

	// A manually-created control never gets a font unless you set one --
	// without this it silently falls back to the ancient stock GDI font,
	// which reads as Windows-2000-dated no matter what theming/manifest
	// work is done elsewhere.
	m_wndClassView.SetFont(&afxGlobalData.fontRegular);

	// Give rows room to breathe around their 16x16 icon -- the tree
	// control's default row height is too tight once every row has an
	// icon next to its text.
	m_wndClassView.SetItemHeight(22);

	// Switching off the Explorer visual style for just this control lets
	// OnTreeCustomDraw's colors fully take over the selection highlight --
	// left themed, a fully-visual-styled tree view ignores custom draw's
	// background color for the selected row and falls back to the theme's
	// own (much more muted) highlight.
	::SetWindowTheme(m_wndClassView.GetSafeHwnd(), L" ", L" ");

	// Base colors for whatever OnTreeCustomDraw doesn't explicitly paint
	// per-row (the space to the right of each row's text, the empty area
	// below the last item).
	m_wndClassView.SetBkColor(Theme::Bg1);
	m_wndClassView.SetTextColor(Theme::Text);

	// Create the property grid now too -- it's a sibling window (parented
	// directly to CMainFrame, same as this panel), so CMainFrame positions
	// it, but it needs to exist before anything tries to load data into it.

	AdjustLayout();

	return 0;
}

void CClassView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

BOOL CClassView::OnEraseBkgnd(CDC* pDC)
{
	CRect rectClient;
	GetClientRect(rectClient);
	pDC->FillSolidRect(rectClient, Theme::Bg2);
	return TRUE;
}

// Recolors the native controls this panel can't fully own-draw without a
// lot more code (the search edit, the Accurate checkbox's label) to match
// the dark palette. The checkbox glyph itself stays the native black/white
// square -- BS_AUTOCHECKBOX doesn't expose it for recoloring short of full
// owner-draw, which isn't worth it for one checkbox.
HBRUSH CClassView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetSafeHwnd() == m_editSearch.GetSafeHwnd())
	{
		pDC->SetTextColor(Theme::Text);
		pDC->SetBkColor(Theme::Bg2);
		return (HBRUSH)m_brEditBg.GetSafeHandle();
	}

	if (pWnd->GetSafeHwnd() == m_chkAccurate.GetSafeHwnd())
	{
		pDC->SetTextColor(Theme::Text);
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)m_brPanelBg.GetSafeHandle();
	}

	return CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CClassView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	const int cyBar = 30;
	const int cyMargin = 4;
	const int cxAccurate = 64;
	const int cxNav = 36;
	const int cxGap = 6;

	int x = rectClient.left + cyMargin;
	int cxEdit = rectClient.Width() - cyMargin * 2 - cxAccurate - cxNav * 2 - cxGap * 3;
	if (cxEdit < 20)
	{
		cxEdit = 20;
	}

	m_editSearch.SetWindowPos(nullptr, x, rectClient.top + cyMargin, cxEdit, cyBar - cyMargin * 2, SWP_NOACTIVATE | SWP_NOZORDER);
	x += cxEdit + cxGap;

	m_chkAccurate.SetWindowPos(nullptr, x, rectClient.top + cyMargin, cxAccurate, cyBar - cyMargin * 2, SWP_NOACTIVATE | SWP_NOZORDER);
	x += cxAccurate + cxGap;

	m_btnPrev.SetWindowPos(nullptr, x, rectClient.top + cyMargin, cxNav, cyBar - cyMargin * 2, SWP_NOACTIVATE | SWP_NOZORDER);
	x += cxNav + cxGap;

	m_btnNext.SetWindowPos(nullptr, x, rectClient.top + cyMargin, cxNav, cyBar - cyMargin * 2, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndClassView.SetWindowPos(nullptr, rectClient.left, rectClient.top + cyBar, rectClient.Width(), rectClient.Height() - cyBar, SWP_NOACTIVATE | SWP_NOZORDER);
}

int CClassView::GetOrLoadIconIndex(const CString& strIconName)
{
	if (strIconName.IsEmpty())
	{
		return m_nDefaultIconIndex;
	}

	int nExisting = -1;
	if (m_mapIconIndex.Lookup(strIconName, nExisting))
	{
		return nExisting;
	}

	int nIndex = m_nDefaultIconIndex;

	CImage image;
	CString strPath = _T("./texture/gui/icon/") + strIconName;

	if (image.Load(strPath) == S_OK)
	{
		CImage resized;
		resized.Create(16, 16, 32);

		image.StretchBlt(resized.GetDC(), 0, 0, 16, 16, SRCCOPY);
		resized.ReleaseDC();

		CBitmap bmp;
		bmp.Attach((HBITMAP)resized.Detach());
		nIndex = m_ClassViewIcons.Add(&bmp, RGB(255, 0, 255));

		image.Destroy();
	}

	m_mapIconIndex.SetAt(strIconName, nIndex);

	return nIndex;
}

void CClassView::LoadTableData(int nTableType)
{
	if (m_nTableType == nTableType)
		return;

	m_wndClassView.DeleteAllItems();

	m_aMatches.RemoveAll();
	m_nMatchIndex = -1;

	m_mapTextAllCategoryByNode.RemoveAll();
	m_aTextAllCategoryPopulated.RemoveAll();

	// Rebuild the row icon list for the newly selected table.
	m_ClassViewIcons.DeleteImageList();
	m_ClassViewIcons.Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 32);
	m_mapIconIndex.RemoveAll();
	m_nDefaultIconIndex = m_ClassViewIcons.Add(::LoadIcon(nullptr, IDI_APPLICATION));
	m_wndClassView.SetImageList(&m_ClassViewIcons, TVSIL_NORMAL);

	HTREEITEM hRoot = m_wndClassView.InsertItem(_T("Data"), 0, 0);
	m_wndClassView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	CTable* pTable = nullptr;

	switch (nTableType)
	{
		case CTableContainer::TABLE_ITEM:
		{
			pTable = GetTableContainer()->GetItemTable();
		}
		break;

		case CTableContainer::TABLE_NEWBIE:
		{
			pTable = GetTableContainer()->GetNewbieTable();
		}
		break;


		case CTableContainer::TABLE_MASCOT:
		{
			pTable = GetTableContainer()->GetMascotTable();
		}
		break;

		case CTableContainer::TABLE_MASCOT_GRADE:
		{
			pTable = GetTableContainer()->GetMascotGradeTable();
		}
		break;

		case CTableContainer::TABLE_MASCOT_STATUS:
		{
			pTable = GetTableContainer()->GetMascotStatusTable();
		}
		break;

		case CTableContainer::TABLE_CHARTITLE:
		{
			pTable = GetTableContainer()->GetCharTitleTable();
		}
		break;

		case CTableContainer::TABLE_CHATTING_FILTER:
		{
			pTable = GetTableContainer()->GetChattingFilterTable();
		}
		break;

		case CTableContainer::TABLE_ITEM_ENCHANT:
		{
			pTable = GetTableContainer()->GetItemEnchantTable();
		}
		break;

		case CTableContainer::TABLE_MERCHANT:
		{
			pTable = GetTableContainer()->GetMerchantTable();
		}
		break;

		case CTableContainer::TABLE_FORMULA:
		{
			pTable = GetTableContainer()->GetFormulaTable();
		}
		break;

		case CTableContainer::TABLE_ITEM_MIX_EXP:
		{
			pTable = GetTableContainer()->GetItemMixExpTable();
		}
		break;

		case CTableContainer::TABLE_CHAT_COMMAND:
		{
			pTable = GetTableContainer()->GetChatCommandTable();
		}
		break;




		case CTableContainer::TABLE_VEHICLE:
		{
			pTable = GetTableContainer()->GetVehicleTable();
		}
		break;

		case CTableContainer::TABLE_DUNGEON:
		{
			pTable = GetTableContainer()->GetDungeonTable();
		}
		break;


		case CTableContainer::TABLE_DRAGONBALL_REWARD:
		{
			pTable = GetTableContainer()->GetDragonBallRewardTable();
		}
		break;

		case CTableContainer::TABLE_EXP:
		{
			pTable = GetTableContainer()->GetExpTable();
		}
		break;


		case CTableContainer::TABLE_CHARM:
		{
			pTable = GetTableContainer()->GetCharmTable();
		}
		break;

		case CTableContainer::TABLE_ACTION:
		{
			pTable = GetTableContainer()->GetActionTable();
		}
		break;



		case CTableContainer::TABLE_HELP:
		{
			pTable = GetTableContainer()->GetHelpTable();
		}
		break;

		case CTableContainer::TABLE_GUIDE_HINT:
		{
			pTable = GetTableContainer()->GetGuideHintTable();
		}
		break;






		case CTableContainer::TABLE_DYNAMIC_OBJECT:
		{
			pTable = GetTableContainer()->GetDynamicObjectTable();
		}
		break;

		case CTableContainer::TABLE_USE_ITEM:
		{
			pTable = GetTableContainer()->GetUseItemTable();
		}
		break;

		case CTableContainer::TABLE_SET_ITEM:
		{
			pTable = GetTableContainer()->GetSetItemTable();
		}
		break;







		case CTableContainer::TABLE_QUEST_REWARD:
		{
			pTable = GetTableContainer()->GetQuestRewardTable();
		}
		break;

		case CTableContainer::TABLE_QUEST_REWARD_SELECT:
		{
			pTable = GetTableContainer()->GetQuestRewardSelectTable();
		}
		break;




		case CTableContainer::TABLE_NPC_SERVER:
		{
			pTable = GetTableContainer()->GetNpcServerTable();
		}
		break;

		case CTableContainer::TABLE_WORLD:
		{
			pTable = GetTableContainer()->GetWorldTable();
		}
		break;


		case CTableContainer::TABLE_SYSTEM_EFFECT:
		{
			pTable = GetTableContainer()->GetSystemEffectTable();
		}
		break;

		case CTableContainer::TABLE_ITEM_OPTION:
		{
			pTable = GetTableContainer()->GetItemOptionTable();
		}
		break;

		case CTableContainer::TABLE_SKILL:
		{
			pTable = GetTableContainer()->GetSkillTable();
		}
		break;




		case CTableContainer::TABLE_HLS_ITEM:
		{
			pTable = GetTableContainer()->GetHLSItemTable();
		}
		break;

		case CTableContainer::TABLE_STATUS_TRANSFORM:
		{
			pTable = GetTableContainer()->GetStatusTransformTable();
		}
		break;



		case CTableContainer::TABLE_QUEST_PROBABILITY:
		{
			pTable = GetTableContainer()->GetQuestProbabilityTable();
		}
		break;

		case CTableContainer::TABLE_WORLD_PLAY:
		{
			pTable = GetTableContainer()->GetWorldPlayTable();
		}
		break;

		case CTableContainer::TABLE_HLS_SLOT_MACHINE:
		{
			pTable = GetTableContainer()->GetSlotMachineTable();
		}
		break;

		case CTableContainer::TABLE_HLS_SLOT_MACHINE_ITEM:
		{
			pTable = GetTableContainer()->GetSlotMachineItemTable();
		}
		break;

		case CTableContainer::TABLE_ITEM_UPGRADE_RATE_NEW:
		{
			pTable = GetTableContainer()->GetItemUpgradeRateNewTable();
		}
		break;

		case CTableContainer::TABLE_ITEM_BAG_LIST:
		{
			pTable = GetTableContainer()->GetItemBagListTable();
		}
		break;

		case CTableContainer::TABLE_ITEM_GROUP_LIST:
		{
			pTable = GetTableContainer()->GetItemGroupListTable();
		}
		break;

		case CTableContainer::TABLE_MOB_SERVER:
		{
			pTable = GetTableContainer()->GetMobServerTable();
		}
		break;

		case CTableContainer::TABLE_DRAGONBALL_RETURN_POINT:
		{
			pTable = GetTableContainer()->GetDragonBallReturnPointTable();
		}
		break;

		case CTableContainer::TABLE_EVENT_SYSTEM:
		{
			pTable = GetTableContainer()->GetEventSystemTable();
		}
		break;

		case CTableContainer::TABLE_DYNAMIC_FIELD_SYSTEM:
		{
			pTable = GetTableContainer()->GetDynamicFieldSystemTable();
		}
		break;

		case CTableContainer::TABLE_SPEECH:
		{
			pTable = GetTableContainer()->GetNpcSpeechTable();
		}
		break;

		case CTableContainer::TABLE_HTB_SET:
		{
			pTable = GetTableContainer()->GetHTBSetTable();
		}
		break;

		case CTableContainer::TABLE_DIRECTION_LINK:
		{
			pTable = GetTableContainer()->GetDirectionLinkTable();
		}
		break;

		case CTableContainer::TABLE_DOJO:
		{
			pTable = GetTableContainer()->GetDojoTable();
		}
		break;

		case CTableContainer::TABLE_MOB_MOVE_PATTERN:
		{
			pTable = GetTableContainer()->GetMobMovePatternTable();
		}
		break;

		case CTableContainer::TABLE_LAND_MARK:
		{
			pTable = GetTableContainer()->GetLandMarkTable();
		}
		break;

		case CTableContainer::TABLE_AIR_COSTUME:
		{
			pTable = GetTableContainer()->GetAirCostumeTable();
		}
		break;

		case CTableContainer::TABLE_QUEST_TEXT_DATA:
		{
			pTable = GetTableContainer()->GetQuestTextDataTable();
		}
		break;

		case CTableContainer::TABLE_TIMEQUEST:
		{
			pTable = GetTableContainer()->GetTimeQuestTable();
		}
		break;

		case CTableContainer::TABLE_BUDOKAI:
		{
			pTable = GetTableContainer()->GetBudokaiTable();
		}
		break;

		case CTableContainer::TABLE_RANKBATTLE:
		{
			pTable = GetTableContainer()->GetRankBattleTable();
		}
		break;

		case CTableContainer::TABLE_SCRIPT_LINK:
		{
			pTable = GetTableContainer()->GetScriptLinkTable();
		}
		break;

		case CTableContainer::TABLE_QUEST_NARRATION:
		{
			pTable = GetTableContainer()->GetQuestNarrationTable();
		}
		break;

		case CTableContainer::TABLE_QUEST_ITEM:
		{
			pTable = GetTableContainer()->GetQuestItemTable();
		}
		break;

		case CTableContainer::TABLE_ITEM_DISASSEMBLE:
		{
			pTable = GetTableContainer()->GetItemDisassembleTable();
		}
		break;

		case CTableContainer::TABLE_PORTAL:
		{
			pTable = GetTableContainer()->GetPortalTable();
		}
		break;

		case CTableContainer::TABLE_ITEM_RECIPE:
		{
			pTable = GetTableContainer()->GetItemRecipeTable();
		}
		break;

		case CTableContainer::TABLE_MIX_MACHINE:
		{
			pTable = GetTableContainer()->GetItemMixMachineTable();
		}
		break;

		case CTableContainer::TABLE_DRAGONBALL:
		{
			pTable = GetTableContainer()->GetDragonBallTable();
		}
		break;

		case CTableContainer::TABLE_WORLD_ZONE:
		{
			pTable = GetTableContainer()->GetWorldZoneTable();
		}
		break;

		case CTableContainer::TABLE_COMMON_CONFIG:
		{
			pTable = GetTableContainer()->GetCommonConfigTable();
		}
		break;

		case CTableContainer::TABLE_DWC:
		{
			pTable = GetTableContainer()->GetDwcTable();
		}
		break;

		case CTableContainer::TABLE_WORLD_MAP:
		{
			pTable = GetTableContainer()->GetWorldMapTable();
		}
		break;

		case CTableContainer::TABLE_PC:
		{
			pTable = GetTableContainer()->GetPcTable();
		}
		break;

		case CTableContainer::TABLE_MOB:
		{
			pTable = GetTableContainer()->GetMobTable();
		}
		break;

		case CTableContainer::TABLE_NPC:
		{
			pTable = GetTableContainer()->GetNpcTable();
		}
		break;

		case CTableContainer::TABLE_DWCMISSION:
		{
			pTable = GetTableContainer()->GetDwcMissionTable();
		}
		break;

		case CTableContainer::TABLE_QUEST_DROP:
		{
			pTable = GetTableContainer()->GetQuestDropTable();
		}
		break;

		default: break;
	}

	// TextAll isn't a single per-row table -- it's 28 separate
	// CTextAllTable categories -- so its tab shows a two-level tree
	// instead: one node per category, lazily expanded into rows on first
	// expand (see OnTreeItemExpanding). Item alone is ~54,000 rows, so
	// building all 28 up front isn't viable.
	if (nTableType == CTableContainer::TABLE_TEXT_ALL)
	{
		for (int i = 0; i < (int)_countof(s_apszTextAllCategories); ++i)
		{
			HTREEITEM hCategory = m_wndClassView.InsertItem(s_apszTextAllCategories[i], m_nDefaultIconIndex, m_nDefaultIconIndex, hRoot);
			m_wndClassView.SetItemData(hCategory, 0);
			m_mapTextAllCategoryByNode.SetAt(hCategory, i);
			m_aTextAllCategoryPopulated.Add(false);

			// Dummy child so the expand arrow shows -- swapped out for the
			// real rows on first expand.
			HTREEITEM hDummy = m_wndClassView.InsertItem(_T("Loading..."), m_nDefaultIconIndex, m_nDefaultIconIndex, hCategory);
			m_wndClassView.SetItemData(hDummy, 0);
		}
	}
	else if (pTable)
	{
		CTextAllTable* pTextAllTable = GetTableContainer()->GetTextAllTable();
		CTextTable* pItemTextTable = pTextAllTable ? pTextAllTable->GetItemTbl() : nullptr;

		for (CTable::TABLEIT it = pTable->Begin(); it != pTable->End(); it++)
		{
			sTBLDAT* pTableData = it->second;

			CString strLabel;
			int nImage = m_nDefaultIconIndex;

			if (nTableType == CTableContainer::TABLE_ITEM)
			{
				sITEM_TBLDAT* pItemData = (sITEM_TBLDAT*)pTableData;

				CString strName;
				std::wstring wstrTranslated;
				if (pItemTextTable && pItemTextTable->GetText(pItemData->Name, &wstrTranslated))
				{
					strName = wstrTranslated.c_str();
				}

				strLabel.Format(_T("[%u] %s"), pTableData->tblidx, (LPCTSTR)strName);

				nImage = GetOrLoadIconIndex(CString(pItemData->szIcon_Name));
			}
			else
			{
				strLabel.Format(_T("[%u]"), pTableData->tblidx);

				// Every table below has its own Icon_Name-shaped field (see
				// the schema/*.json files with an "Icon_Name" entry) and
				// stores it the same way Item does -- same textures/gui/icon
				// folder, same lookup/cache, just no linked name-translation
				// table like Item's.
				switch (nTableType)
				{
				case CTableContainer::TABLE_ACTION:
					nImage = GetOrLoadIconIndex(CString(((sACTION_TBLDAT*)pTableData)->szIcon_Name));
					break;
				case CTableContainer::TABLE_HTB_SET:
					nImage = GetOrLoadIconIndex(CString(((sHTB_SET_TBLDAT*)pTableData)->szIcon_Name));
					break;
				case CTableContainer::TABLE_QUEST_ITEM:
					nImage = GetOrLoadIconIndex(CString(((sQUESTITEM_TBLDAT*)pTableData)->szIconName));
					break;
				case CTableContainer::TABLE_SKILL:
					nImage = GetOrLoadIconIndex(CString(((sSKILL_TBLDAT*)pTableData)->szIcon_Name));
					break;
				case CTableContainer::TABLE_LAND_MARK:
					nImage = GetOrLoadIconIndex(CString(((sLAND_MARK_TBLDAT*)pTableData)->wszIconName));
					break;
				default: break;
				}
			}

			HTREEITEM item = m_wndClassView.InsertItem(strLabel, nImage, nImage, hRoot);
			m_wndClassView.SetItemData(item, (DWORD_PTR)pTableData);
		}
	}

	m_nTableType = nTableType;

	m_wndClassView.Expand(hRoot, TVE_EXPAND);
}

// Each per-World spawn table is its own CFileView tab (see
// CFileView::RefreshTables/AddSpawnTab), so unlike TextAll this is a flat
// row list, not a lazy tree -- there's exactly one table to show per call.
void CClassView::LoadSpawnTable(int nCategory, TBLIDX worldTblidx)
{
	m_wndClassView.DeleteAllItems();

	m_aMatches.RemoveAll();
	m_nMatchIndex = -1;

	m_mapTextAllCategoryByNode.RemoveAll();
	m_aTextAllCategoryPopulated.RemoveAll();

	m_ClassViewIcons.DeleteImageList();
	m_ClassViewIcons.Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 32);
	m_mapIconIndex.RemoveAll();
	m_nDefaultIconIndex = m_ClassViewIcons.Add(::LoadIcon(nullptr, IDI_APPLICATION));
	m_wndClassView.SetImageList(&m_ClassViewIcons, TVSIL_NORMAL);

	HTREEITEM hRoot = m_wndClassView.InsertItem(_T("Data"), 0, 0);
	m_wndClassView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	CTableContainer* pContainer = GetTableContainer();
	CTable* pTable = nullptr;
	int nRowTableType = SPAWN_ROW_NPC_OR_MOB;

	if (pContainer)
	{
		if (nCategory == 0)
		{
			pTable = pContainer->GetNpcSpawnTable(worldTblidx);
		}
		else if (nCategory == 1)
		{
			pTable = pContainer->GetMobSpawnTable(worldTblidx);
		}
		else
		{
			pTable = pContainer->GetObjectTable(worldTblidx);
			nRowTableType = SPAWN_ROW_OBJECT;
		}
	}

	if (pTable)
	{
		for (CTable::TABLEIT it = pTable->Begin(); it != pTable->End(); ++it)
		{
			sTBLDAT* pRow = it->second;

			CString strLabel;
			strLabel.Format(_T("[%u]"), pRow->tblidx);

			HTREEITEM item = m_wndClassView.InsertItem(strLabel, m_nDefaultIconIndex, m_nDefaultIconIndex, hRoot);
			m_wndClassView.SetItemData(item, (DWORD_PTR)pRow);
		}
	}

	m_nTableType = nRowTableType;

	m_wndClassView.Expand(hRoot, TVE_EXPAND);
}

void CClassView::ResetView()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	m_wndClassView.DeleteAllItems();
	m_aMatches.RemoveAll();
	m_nMatchIndex = -1;
	m_nTableType = -1;
	m_mapTextAllCategoryByNode.RemoveAll();
	m_aTextAllCategoryPopulated.RemoveAll();
	m_editSearch.SetWindowText(_T(""));
}

// Replaces a TextAll category node's "Loading..." placeholder with its
// real rows. Called once per category, on first expand.
void CClassView::PopulateTextAllCategory(HTREEITEM hCategoryNode, int nCategory)
{
	CWaitCursor wait;

	HTREEITEM hChild = m_wndClassView.GetChildItem(hCategoryNode);
	while (hChild)
	{
		HTREEITEM hNext = m_wndClassView.GetNextSiblingItem(hChild);
		m_wndClassView.DeleteItem(hChild);
		hChild = hNext;
	}

	CTextAllTable* pTextAllTable = GetTableContainer() ? GetTableContainer()->GetTextAllTable() : nullptr;
	CTextTable* pSub = pTextAllTable ? pTextAllTable->GetTextTbl((CTextAllTable::TABLETYPE)nCategory) : nullptr;

	if (pSub)
	{
		for (CTable::TABLEIT it = pSub->Begin(); it != pSub->End(); ++it)
		{
			sTEXT_TBLDAT* pRow = (sTEXT_TBLDAT*)it->second;

			CString strText(pRow->wstrText.c_str());
			strText.Replace(_T("\r\n"), _T(" "));
			strText.Replace(_T("\n"), _T(" "));
			if (strText.GetLength() > 80)
			{
				strText = strText.Left(80) + _T("...");
			}

			CString strLabel;
			strLabel.Format(_T("[%u] %s"), pRow->tblidx, (LPCTSTR)strText);

			HTREEITEM item = m_wndClassView.InsertItem(strLabel, m_nDefaultIconIndex, m_nDefaultIconIndex, hCategoryNode);
			m_wndClassView.SetItemData(item, (DWORD_PTR)(sTBLDAT*)pRow);
		}
	}

	if (nCategory >= 0 && nCategory < m_aTextAllCategoryPopulated.GetSize())
	{
		m_aTextAllCategoryPopulated[nCategory] = true;
	}
}

void CClassView::OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;

	if (pNMTreeView->action != TVE_EXPAND)
	{
		return;
	}

	int nCategory = -1;
	if (!m_mapTextAllCategoryByNode.Lookup(pNMTreeView->itemNew.hItem, nCategory))
	{
		return; // not a TextAll category node
	}

	if (nCategory < m_aTextAllCategoryPopulated.GetSize() && m_aTextAllCategoryPopulated[nCategory])
	{
		return; // already populated
	}

	PopulateTextAllCategory(pNMTreeView->itemNew.hItem, nCategory);
}

void CClassView::OnTvnSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	sTBLDAT* pTableData = (sTBLDAT*)m_wndClassView.GetItemData(pNMTreeView->itemNew.hItem);
	if (pTableData)
	{
		m_wndProperties.LoadTableData(m_nTableType, pTableData);
	}

	*pResult = 0;
}

void CClassView::OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVCUSTOMDRAW pCD = reinterpret_cast<LPNMTVCUSTOMDRAW>(pNMHDR);

	switch (pCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;

	case CDDS_ITEMPREPAINT:
	{
		bool bSelected = (pCD->nmcd.uItemState & CDIS_SELECTED) != 0;

		pCD->clrText = bSelected ? Theme::AccentText : Theme::Text;
		pCD->clrTextBk = bSelected ? Theme::Accent : Theme::Bg1;

		// Suppress the dotted focus rectangle default drawing would add
		// around the selected row's text -- a flat highlight block reads
		// as modern, a focus rect around highlighted text reads dated.
		pCD->nmcd.uItemState &= ~CDIS_FOCUS;

		*pResult = CDRF_DODEFAULT;
		return;
	}

	default:
		*pResult = CDRF_DODEFAULT;
		return;
	}
}

void CClassView::RunSearch()
{
	m_aMatches.RemoveAll();
	m_nMatchIndex = -1;

	CString strQuery;
	m_editSearch.GetWindowText(strQuery);
	strQuery.Trim();

	if (strQuery.IsEmpty())
	{
		return;
	}

	BOOL bAccurate = (m_chkAccurate.GetCheck() == BST_CHECKED);

	HTREEITEM hRoot = m_wndClassView.GetRootItem();
	if (!hRoot)
	{
		return;
	}

	// GetNextVisibleItem walks in on-screen top-to-bottom order and
	// naturally skips collapsed subtrees -- which for TextAll's two-level
	// tree also means it only searches categories that have already been
	// expanded (and thus lazily populated, see OnTreeItemExpanding), not
	// the ~28 categories' full ~90,000 rows on every keystroke.
	for (HTREEITEM hItem = m_wndClassView.GetChildItem(hRoot); hItem != nullptr; hItem = m_wndClassView.GetNextVisibleItem(hItem))
	{
		CString strLabel = m_wndClassView.GetItemText(hItem);

		CString strName = strLabel;
		int nNameStart = strLabel.Find(_T("] "));
		if (nNameStart >= 0)
		{
			strName = strLabel.Mid(nNameStart + 2);
		}

		sTBLDAT* pTableData = (sTBLDAT*)m_wndClassView.GetItemData(hItem);
		CString strId;
		if (pTableData)
		{
			strId.Format(_T("%u"), pTableData->tblidx);
		}

		BOOL bMatch = FALSE;
		if (bAccurate)
		{
			bMatch = (strId.CompareNoCase(strQuery) == 0) || (strName.CompareNoCase(strQuery) == 0);
		}
		else
		{
			CString strLabelLower = strLabel;
			CString strQueryLower = strQuery;
			strLabelLower.MakeLower();
			strQueryLower.MakeLower();
			bMatch = (strLabelLower.Find(strQueryLower) >= 0);
		}

		if (bMatch)
		{
			m_aMatches.Add(hItem);
		}
	}

	if (m_aMatches.GetSize() > 0)
	{
		m_nMatchIndex = 0;

		HTREEITEM hMatch = m_aMatches[0];
		m_wndClassView.SelectItem(hMatch);
		m_wndClassView.EnsureVisible(hMatch);
	}
}

void CClassView::GoToMatch(int nDelta)
{
	if (m_aMatches.GetSize() == 0)
	{
		RunSearch();
	}

	if (m_aMatches.GetSize() == 0)
	{
		return;
	}

	m_nMatchIndex += nDelta;

	if (m_nMatchIndex < 0)
	{
		m_nMatchIndex = (int)m_aMatches.GetSize() - 1;
	}
	else if (m_nMatchIndex >= m_aMatches.GetSize())
	{
		m_nMatchIndex = 0;
	}

	HTREEITEM hMatch = m_aMatches[m_nMatchIndex];
	m_wndClassView.SelectItem(hMatch);
	m_wndClassView.EnsureVisible(hMatch);
}

void CClassView::OnSearchTextChanged()
{
	RunSearch();
}

void CClassView::OnAccurateClicked()
{
	RunSearch();
}

void CClassView::OnPrevClicked()
{
	GoToMatch(-1);
}

void CClassView::OnNextClicked()
{
	GoToMatch(1);
}

BOOL CClassView::HandleSearchKeyDown(const MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && pMsg->hwnd == m_editSearch.GetSafeHwnd())
	{
		GoToMatch(m_aMatches.GetSize() > 0 ? 1 : 0);
		return TRUE;
	}

	return FALSE;
}
