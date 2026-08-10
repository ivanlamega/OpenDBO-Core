
#pragma once

#include "Table.h"
#include "ViewTree.h"
#include "PropertiesWnd.h"
#include "FlatButton.h"

// Fixed left-hand panel: search bar (text + Accurate exact-match toggle +
// Prev/Next) above a row list for whichever table is selected in
// CFileView's tab bar. Shows each row as "[tblidx] Name" with its icon
// (for TABLE_ITEM, pulled from texture/gui/icon) and loads the selected
// row into m_wndProperties as soon as it's clicked. Plain child window --
// not a dockable/floatable pane.
class CClassView : public CWnd
{
public:
	CClassView();
	virtual ~CClassView();

	BOOL Create(CWnd* pParentWnd, UINT nID);

	void AdjustLayout();

	void LoadTableData(int nTableType);

	// Each individual per-World spawn table (e.g. "spawn_npc_dungeon_001")
	// gets its own nav tab in CFileView, same as any other RDF table, but
	// none of them is kept in one CTableContainer::eTABLE slot -- see
	// Util.h's SPAWN_ROW_NPC_OR_MOB comment -- so they get their own load
	// entry point instead of going through LoadTableData(int).
	// nCategory: 0 = Npc Spawn, 1 = Mob Spawn, 2 = Object Spawn.
	void LoadSpawnTable(int nCategory, TBLIDX worldTblidx);

	// Clears the row list and property grid. Call before the table
	// container they point into is destroyed/recreated.
	void ResetView();

	CPropertiesWnd    m_wndProperties;

protected:
	CViewTree m_wndClassView;
	CImageList m_ClassViewIcons;
	CMap<CString, LPCTSTR, int, int> m_mapIconIndex;
	int m_nDefaultIconIndex;

	CEdit m_editSearch;
	CButton m_chkAccurate;
	CFlatButton m_btnPrev;
	CFlatButton m_btnNext;
	CBrush m_brEditBg;
	CBrush m_brPanelBg;

	CArray<HTREEITEM, HTREEITEM> m_aMatches;
	int m_nMatchIndex;

	int		m_nTableType;

	// TABLE_TEXT_ALL's tab shows a two-level tree instead of a flat row
	// list: 28 category nodes (one per CTextAllTable::TABLETYPE), each
	// lazily expanded into its rows on first expand -- the Item category
	// alone is ~54,000 rows, so building all 28 up front isn't viable.
	CMap<HTREEITEM, HTREEITEM, int, int> m_mapTextAllCategoryByNode;
	CArray<bool, bool> m_aTextAllCategoryPopulated;

	int  GetOrLoadIconIndex(const CString& strIconName);
	void RunSearch();
	void GoToMatch(int nDelta);
	void PopulateTextAllCategory(HTREEITEM hCategoryNode, int nCategory);

public:
	// Called from CMainFrame::PreTranslateMessage so Enter in the search
	// box jumps to the next match -- plain child windows don't get their
	// own PreTranslateMessage invoked by the message pump.
	BOOL HandleSearchKeyDown(const MSG* pMsg);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTvnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSearchTextChanged();
	afx_msg void OnAccurateClicked();
	afx_msg void OnPrevClicked();
	afx_msg void OnNextClicked();

	DECLARE_MESSAGE_MAP()
};
