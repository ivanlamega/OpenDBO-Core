#pragma once

#include "FlatButton.h"

struct sTBLDAT;

// Fixed right-hand panel: the Name/Value grid for whatever row is
// currently selected in CClassView. Plain child window -- not a
// dockable/floatable pane.
class CPropertiesWnd : public CWnd
{
// Construction
public:
	CPropertiesWnd();

	BOOL Create(CWnd* pParentWnd, UINT nID);

	void AdjustLayout();

// Attributes
public:
	void SetVSDotNetLook(BOOL bSet)
	{
		m_wndPropList.SetVSDotNetLook(bSet);
		m_wndPropList.SetGroupNameFullWidth(bSet);
	}

	void	LoadTableData(int nTableType, sTBLDAT* pTbldat);

protected:
	CFont m_fntPropList;
	CMFCPropertyGridCtrl m_wndPropList;

	// Replaces the old CMFCToolBar (IDR_PROPERTIES, a stock MFC-wizard
	// bitmap toolbar) -- its only two live buttons, Expand All and
	// alphabetic Sort, as flat buttons matching the rest of the tool
	// instead of a light-gray strip that would clash with the dark theme.
	CFlatButton m_btnExpandAll;
	CFlatButton m_btnSortAZ;

	int			m_nTableType;
	sTBLDAT*	m_pTbldat;

// Implementation
public:
	virtual ~CPropertiesWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnExpandAllProperties();
	afx_msg void OnSortProperties();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	DECLARE_MESSAGE_MAP()

	void InitPropList();
	void SetPropListFont();
};

