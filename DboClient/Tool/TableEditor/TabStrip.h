#pragma once

#include <vector>

// Flat, fully self-painted tab strip for CFileView. Standing in for a
// native CTabCtrl: even with TCS_OWNERDRAWFIXED, comctl32 still paints its
// own themed tab frame/edges underneath owner-drawn item content, and its
// built-in overflow (a spin-arrow cluster) looks dated -- with ~59
// registered tables needing a tab each, overflow isn't optional here. This
// control owns 100% of its own painting and scrolling instead, so there is
// no leftover comctl32 chrome at all.
class CTabStrip : public CWnd
{
public:
	CTabStrip();

	BOOL Create(CWnd* pParentWnd, UINT nID);

	int AddTab(const CString& strText);
	void DeleteAllTabs();
	int GetTabCount() const { return (int)m_aTabs.size(); }

	int GetCurSel() const { return m_nCurSel; }
	void SetCurSel(int nIndex);

	// Client-coordinate hit test; returns -1 if the point isn't over a tab.
	int HitTest(CPoint ptClient) const;

protected:
	struct STabItem
	{
		CString strText;
		int nWidth;
	};

	std::vector<STabItem> m_aTabs;
	int m_nCurSel;
	int m_nHotIndex;
	int m_nScrollOffset;
	bool m_bTrackingMouse;

	bool NeedsScrolling() const;
	int GetContentWidth() const;
	CRect GetContentRect() const;
	CRect GetTabRect(int nIndex) const;
	void ScrollBy(int nDelta);
	void EnsureVisible(int nIndex);

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	DECLARE_MESSAGE_MAP()
};

// Sent to the parent window when the user clicks a tab. WPARAM is the new
// selection index. Not sent for programmatic SetCurSel calls.
#define TBSTN_SELCHANGE (WM_APP + 600)
// Sent to the parent window on a right-click anywhere in the strip, before
// any selection change -- the parent hit-tests for itself (via HitTest) and
// decides what, if anything, to select.
#define TBSTN_RCLICK (WM_APP + 601)
