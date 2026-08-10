#pragma once

#include <functional>

// A small "please wait" popup shown while RunWithProgress() runs a task on
// a background thread. Built as a plain CWnd (no dialog resource), same
// pattern as the other panels in this tool (ClassView/PropertiesWnd/etc).
class CProgressPopup : public CWnd
{
public:
	BOOL Create(CWnd* pParentWnd, const CString& strMessage);

protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

private:
	CProgressCtrl m_wndBar;
	CString m_strMessage;
};

// Runs fn() on a background thread while keeping the UI thread's message
// pump alive (via MsgWaitForMultipleObjects + PeekMessage), so long RDF/XML
// loads for big tables (Item, QuestText, TextAll, ...) don't make Windows
// mark the app "Not Responding". pParentWnd is disabled and a small
// progress popup is shown for the duration; both are restored before
// returning. Returns whatever fn() returned.
//
// fn must not touch any CWnd/UI object -- it runs on a worker thread. Only
// the caller, after RunWithProgress returns, should touch the UI with the
// result.
bool RunWithProgress(CWnd* pParentWnd, const CString& strMessage, std::function<bool()> fn);
