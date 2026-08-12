#include "pch.h"
#include "framework.h"
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "TableEditor.h"

#include "Util.h"
#include "XmlExport.h"
#include "ProgressDlg.h"
#include "WorldTable.h"
#include "Theme.h"

#include <atlconv.h>
#include <vector>
#include <algorithm>

#define IDC_TABLE_TABS 2001

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileView

CFileView::CFileView()
{
}

CFileView::~CFileView()
{
	DeleteTableContainer();
}

BEGIN_MESSAGE_MAP(CFileView, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(TBSTN_SELCHANGE, OnTabSelChange)
	ON_MESSAGE(TBSTN_RCLICK, OnTabRightClick)
END_MESSAGE_MAP()

BOOL CFileView::Create(CWnd* pParentWnd, UINT nID)
{
	static CString strClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
		::LoadCursor(nullptr, IDC_ARROW), (HBRUSH)nullptr, nullptr);

	return CWnd::Create(strClass, _T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), pParentWnd, nID);
}

/////////////////////////////////////////////////////////////////////////////
// CFileView message handlers

int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndTabs.Create(this, IDC_TABLE_TABS))
	{
		TRACE0("Failed to create table tab bar\n");
		return -1;      // fail to create
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//	Table
	//
	//////////////////////////////////////////////////////////////////////////
	if (CreateTableContainer("./data/", CTable::LOADING_METHOD_BINARY) == false)
	{
		AfxMessageBox(_T("Building tables failed"));
		return -1;
	}

	RefreshTables();

	AdjustLayout();

	return 0;
}

void CFileView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

BOOL CFileView::OnEraseBkgnd(CDC* pDC)
{
	CRect rectClient;
	GetClientRect(rectClient);
	pDC->FillSolidRect(rectClient, Theme::Bg1);
	return TRUE;
}

void CFileView::RefreshTables()
{
	m_wndTabs.DeleteAllTabs();
	m_aTabToTableIndex.RemoveAll();
	m_aTabSpawnInfo.RemoveAll();

	int nCount = 0;
	const STableInfo* pTables = GetRegisteredTables(nCount);

	STabSpawnInfo emptySpawnInfo = { 0, 0 };

	for (int i = 0; i < nCount; ++i)
	{
		if (!pTables[i].bBrowsable)
		{
			continue;
		}

		m_wndTabs.AddTab(pTables[i].pszDisplayName);
		m_aTabToTableIndex.Add(i);
		m_aTabSpawnInfo.Add(emptySpawnInfo);
	}

	// Each individual per-World spawn table (e.g. "spawn_npc_dungeon_001")
	// gets its own tab, same as any other RDF table -- they're genuinely
	// different tables that just happen to share a struct shape, not
	// sub-categories of one shared "Spawn" table. CTableContainer only
	// knows about the ones an actually-loaded World row references, so
	// this list comes from what's really loaded, not a fixed count.
	CTableContainer* pContainer = GetTableContainer();
	CWorldTable* pWorldTable = pContainer ? pContainer->GetWorldTable() : nullptr;

	if (pContainer)
	{
		struct SSpawnTabCandidate
		{
			CString strLabel;
			int nCategory;
			TBLIDX worldTblidx;
		};

		std::vector<SSpawnTabCandidate> spawnTabs;

		for (CTableContainer::SPAWNTABLEIT it = pContainer->BeginNpcSpawnTable(); it != pContainer->EndNpcSpawnTable(); ++it)
		{
			spawnTabs.push_back({ GetSpawnTabLabel(0, it->first, pWorldTable), 0, it->first });
		}
		for (CTableContainer::SPAWNTABLEIT it = pContainer->BeginMobSpawnTable(); it != pContainer->EndMobSpawnTable(); ++it)
		{
			spawnTabs.push_back({ GetSpawnTabLabel(1, it->first, pWorldTable), 1, it->first });
		}
		for (CTableContainer::OBJTABLEIT it = pContainer->BeginObjectTable(); it != pContainer->EndObjectTable(); ++it)
		{
			spawnTabs.push_back({ GetSpawnTabLabel(2, it->first, pWorldTable), 2, it->first });
		}

		// Sort by label so tabs read alphabetically and same-basename,
		// different-number files (spawn_npc_dungeon_001, _002, ...) stay
		// grouped together in the tab strip.
		std::sort(spawnTabs.begin(), spawnTabs.end(), [](const SSpawnTabCandidate& a, const SSpawnTabCandidate& b)
		{
			return a.strLabel.CompareNoCase(b.strLabel) < 0;
		});

		// Multiple Worlds can reference the exact same spawn file (e.g.
		// several instanced copies of the same dungeon) -- CTableContainer
		// loads a separate copy per World tblidx, but that would otherwise
		// show one tab per copy, repeating the same file under the same
		// name many times over. Duplicates are adjacent after the sort
		// above, so a single pass catches them all.
		CString strLastLabel;
		bool bFirst = true;
		for (const SSpawnTabCandidate& candidate : spawnTabs)
		{
			if (!bFirst && candidate.strLabel.CompareNoCase(strLastLabel) == 0)
			{
				continue;
			}

			AddSpawnTab(candidate.nCategory, candidate.worldTblidx, pWorldTable);
			strLastLabel = candidate.strLabel;
			bFirst = false;
		}
	}

	if (m_aTabToTableIndex.GetSize() > 0)
	{
		m_wndTabs.SetCurSel(0);
		OpenTable(0);
	}
}

// Labels a per-World spawn table with its actual spawn file name (falling
// back to "Spawn [worldTblidx]" if the World table isn't loaded or the name
// field is empty).
CString CFileView::GetSpawnTabLabel(int nCategory, TBLIDX worldTblidx, CWorldTable* pWorldTable)
{
	sWORLD_TBLDAT* pWorldData = pWorldTable ? (sWORLD_TBLDAT*)pWorldTable->FindData(worldTblidx) : nullptr;

	CString strLabel;
	if (pWorldData)
	{
		LPCWSTR pwszFileName = (nCategory == 0) ? pWorldData->wszNpcSpawn_Table_Name
			: (nCategory == 1) ? pWorldData->wszMobSpawn_Table_Name
			: pWorldData->wszObjSpawn_Table_Name;

		if (pwszFileName && pwszFileName[0] != L'\0')
		{
			strLabel = CString(pwszFileName);
		}
	}

	if (strLabel.IsEmpty())
	{
		strLabel.Format(_T("Spawn [%u]"), worldTblidx);
	}

	return strLabel;
}

// Adds one tab for a single per-World spawn table.
void CFileView::AddSpawnTab(int nCategory, TBLIDX worldTblidx, CWorldTable* pWorldTable)
{
	CString strLabel = GetSpawnTabLabel(nCategory, worldTblidx, pWorldTable);

	m_wndTabs.AddTab(strLabel);
	m_aTabToTableIndex.Add(-1);

	STabSpawnInfo info;
	info.nCategory = nCategory;
	info.worldTblidx = worldTblidx;
	m_aTabSpawnInfo.Add(info);
}

void CFileView::OpenTable(int nTabIndex)
{
	if (nTabIndex < 0 || nTabIndex >= m_aTabToTableIndex.GetSize())
	{
		return;
	}

	int nTableIndex = m_aTabToTableIndex[nTabIndex];

	if (nTableIndex < 0)
	{
		const STabSpawnInfo& info = m_aTabSpawnInfo[nTabIndex];
		m_wndClassView.LoadSpawnTable(info.nCategory, info.worldTblidx);
		return;
	}

	int nCount = 0;
	const STableInfo* pTables = GetRegisteredTables(nCount);

	m_wndClassView.LoadTableData(pTables[nTableIndex].eType);
}

LRESULT CFileView::OnTabSelChange(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	OpenTable((int)wParam);

	return 0;
}

LRESULT CFileView::OnTabRightClick(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	CPoint ptScreen;
	::GetCursorPos(&ptScreen);

	CPoint ptClient = ptScreen;
	m_wndTabs.ScreenToClient(&ptClient);

	int nTab = m_wndTabs.HitTest(ptClient);

	// Right-clicking a tab selects it first, so "Load" below is obviously
	// scoped to the table you right-clicked.
	if (nTab >= 0)
	{
		m_wndTabs.SetCurSel(nTab);
		OpenTable(nTab);
	}

	// "Load" reloads everything from the chosen folder -- same as the File
	// menu commands, since the table file formats are all read/written
	// together as one data set (see CTableContainer). "Save" below is
	// scoped to just this one table -- not offered for the Spawn tabs,
	// which have no single eTABLE to save (see CClassView::LoadSpawnTable).
	enum { CMD_SAVE_RDF = 9001, CMD_SAVE_XML };

	int nCount = 0;
	const STableInfo* pTables = GetRegisteredTables(nCount);

	bool bCanSaveSingleTable = (nTab >= 0 && nTab < m_aTabToTableIndex.GetSize() && m_aTabToTableIndex[nTab] >= 0);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_TABLE_LOAD_RDF, _T("Load RDF Folder..."));
	menu.AppendMenu(MF_STRING, ID_TABLE_LOAD_XML, _T("Load XML Folder..."));
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(bCanSaveSingleTable ? MF_STRING : (MF_STRING | MF_GRAYED), CMD_SAVE_RDF, _T("Save This Table as RDF..."));
	menu.AppendMenu(bCanSaveSingleTable ? MF_STRING : (MF_STRING | MF_GRAYED), CMD_SAVE_XML, _T("Save This Table as XML..."));

	UINT nCmd = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, ptScreen.x, ptScreen.y, this);

	if (nCmd == ID_TABLE_LOAD_RDF || nCmd == ID_TABLE_LOAD_XML)
	{
		CWnd* pFrame = GetParentFrame();
		if (pFrame)
		{
			pFrame->SendMessage(WM_COMMAND, MAKEWPARAM(nCmd, 0));
		}
	}
	else if ((nCmd == CMD_SAVE_RDF || nCmd == CMD_SAVE_XML) && bCanSaveSingleTable)
	{
		CTableContainer::eTABLE eTable = pTables[m_aTabToTableIndex[nTab]].eType;

		CFolderPickerDialog dlg(nullptr, 0, this);
		if (dlg.DoModal() != IDOK)
		{
			return 0;
		}

		if (nCmd == CMD_SAVE_RDF)
		{
			CString strPath = dlg.GetPathName();
			bool bSuccess = RunWithProgress(this, _T("Saving table..."), [eTable, strPath]() -> bool
			{
				// SaveSingleTableRdf takes a narrow path (it calls straight
				// into the shared table engine, which is narrow-only) --
				// CT2A converts from whatever strPath's width is.
				CT2A pszPath(strPath);
				return SaveSingleTableRdf(eTable, pszPath, false);
			});

			AfxMessageBox(bSuccess ? _T("Table saved.") : _T("Failed to save table."));
		}
		else
		{
			CString strCaveat = GetXmlExportCaveat(eTable);
			CString strPath = dlg.GetPathName();

			bool bSuccess = RunWithProgress(this, _T("Saving table..."), [eTable, strPath]() -> bool
			{
				return SaveSingleTableXml(eTable, strPath);
			});

			if (bSuccess)
			{
				AfxMessageBox(strCaveat.IsEmpty() ? _T("Table saved.") : (_T("Table saved.\n\n") + strCaveat));
			}
			else
			{
				AfxMessageBox(_T("Failed to save table."));
			}
		}
	}

	return 0;
}

void CFileView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	m_wndTabs.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}
