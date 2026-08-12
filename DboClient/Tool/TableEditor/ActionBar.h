#pragma once

#include "FlatButton.h"

// Replaces the traditional CMFCMenuBar + dropdown "File" menu with a
// flat top bar: app name on the left, the 4 real actions (Load/Save RDF,
// Load/Save XML) as flat buttons on the right. The old menu's Edit/Help
// popups were unused MFC-wizard boilerplate (Undo/Cut/Copy/Paste/About
// wired to nothing this tool actually does) -- dropped along with it.
class CActionBar : public CWnd
{
public:
	CActionBar();

	BOOL Create(CWnd* pParentWnd, UINT nID);

	void AdjustLayout();

protected:
	CFlatButton m_btnLoadRdf;
	CFlatButton m_btnSaveRdf;
	CFlatButton m_btnLoadXml;
	CFlatButton m_btnSaveXml;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnLoadRdf();
	afx_msg void OnSaveRdf();
	afx_msg void OnLoadXml();
	afx_msg void OnSaveXml();

	DECLARE_MESSAGE_MAP()
};
