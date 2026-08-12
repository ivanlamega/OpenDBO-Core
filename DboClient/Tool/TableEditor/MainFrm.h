
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "FileView.h"
#include "Table.h"
#include "ActionBar.h"

// Fixed, flat layout: a thin action bar, the table tab strip below it, and
// the row list / property grid splitting the rest of the window. No
// dockable/floatable/auto-hide panes -- CMainFrame positions everything
// itself in RepositionPanels().
class CMainFrame : public CFrameWndEx
{

protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// The doc/view framework rebuilds the caption from the (unused, always
	// "Untitled") document title every time it thinks something relevant
	// changed -- a one-time SetWindowText would just get overwritten later.
	// Overriding this hook instead makes every such rebuild land on our
	// fixed title.
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// No CMFCMenuBar/docking machinery at all -- a flat, self-drawn action
	// bar (see ActionBar.h) stands in for the traditional dropdown "File"
	// menu, and everything below it is a plain fixed child window.
	CActionBar        m_wndActionBar;
	CFileView         m_wndFileView;

	void RepositionPanels();
	void ApplyModernChrome();

	// Reloads the table container from strPath using eLoadingMethod and
	// refreshes the tab bar / row list / property grid to match. Returns
	// false (and shows a message box) if the reload failed.
	bool ReloadTables(const CString& strPath, CTable::eLOADING_METHOD eLoadingMethod);

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTableLoadRdf();
	afx_msg void OnTableSaveRdf();
	afx_msg void OnTableLoadXml();
	afx_msg void OnTableSaveXml();
	DECLARE_MESSAGE_MAP()
};
