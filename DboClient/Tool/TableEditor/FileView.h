
#pragma once

#include "ClassView.h"
#include "TabStrip.h"

class CWorldTable;

// Fixed panel spanning the top of the window: one flat tab per registered
// table (Item / Newbie / Speech / ...). Selecting a tab loads that table
// into m_wndClassView. Not a dockable/floatable pane -- just a plain child
// window CMainFrame positions itself.
class CFileView : public CWnd
{
// Construction
public:
	CFileView();

	BOOL Create(CWnd* pParentWnd, UINT nID);

	void AdjustLayout();

	CClassView        m_wndClassView;

	// Reloads the tab strip from GetRegisteredTables() and selects the first
	// tab (if any). Call after the table container has been (re)created.
	void RefreshTables();

// Attributes
protected:
	CTabStrip m_wndTabs;

	// Tab position -> index into GetRegisteredTables(), or -1 if this tab
	// is one of the per-World Spawn tables instead (see m_aTabSpawnInfo).
	CArray<int, int> m_aTabToTableIndex;

	// Parallel to m_aTabToTableIndex (same size, one entry per tab); only
	// meaningful where that array holds -1. Each individual per-World
	// spawn table (e.g. "spawn_npc_dungeon_001") is its own tab, same as
	// any other RDF table -- see AddSpawnTab -- rather than being grouped
	// under one shared "Spawn" tab, since they're genuinely different
	// tables that just happen to share a struct shape.
	struct STabSpawnInfo
	{
		int nCategory; // 0 = Npc Spawn, 1 = Mob Spawn, 2 = Object Spawn
		TBLIDX worldTblidx;
	};
	CArray<STabSpawnInfo, STabSpawnInfo&> m_aTabSpawnInfo;

protected:
	void OpenTable(int nTabIndex);
	void AddSpawnTab(int nCategory, TBLIDX worldTblidx, CWorldTable* pWorldTable);
	static CString GetSpawnTabLabel(int nCategory, TBLIDX worldTblidx, CWorldTable* pWorldTable);

// Implementation
public:
	virtual ~CFileView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnTabSelChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTabRightClick(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
