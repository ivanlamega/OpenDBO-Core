#pragma once

// Self-drawn flat button -- dark background, accent-colored hover/press,
// no 3D bevel/native theme. Used for MainFrm's action bar (replacing the
// traditional dropdown menu) and ClassView's search Prev/Next buttons.
// A plain CButton subclass rather than BS_OWNERDRAW + parent WM_DRAWITEM
// plumbing -- it draws itself, the parent just creates it.
class CFlatButton : public CButton
{
public:
	CFlatButton();

	// Optional accent underline/highlight when this button represents the
	// "current" action -- unused today, available for later.
	void SetAccented(bool bAccented);

protected:
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg LRESULT OnMouseHover(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()

private:
	bool m_bHover;
	bool m_bPressed;
	bool m_bAccented;
	bool m_bTrackingMouse;
};
