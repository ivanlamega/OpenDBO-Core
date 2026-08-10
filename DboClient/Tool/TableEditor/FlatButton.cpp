#include "pch.h"
#include "FlatButton.h"
#include "Theme.h"

BEGIN_MESSAGE_MAP(CFlatButton, CButton)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_MESSAGE(WM_MOUSEHOVER, &CFlatButton::OnMouseHover)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

CFlatButton::CFlatButton()
	: m_bHover(false)
	, m_bPressed(false)
	, m_bAccented(false)
	, m_bTrackingMouse(false)
{
}

void CFlatButton::SetAccented(bool bAccented)
{
	m_bAccented = bAccented;
	if (GetSafeHwnd())
	{
		Invalidate();
	}
}

void CFlatButton::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(rect);

	COLORREF clrBg = Theme::Bg3;
	if (m_bPressed)
	{
		clrBg = Theme::Accent;
	}
	else if (m_bHover)
	{
		clrBg = Theme::Bg4;
	}
	else if (m_bAccented)
	{
		clrBg = Theme::Bg4;
	}

	dc.FillSolidRect(rect, clrBg);

	if (m_bAccented && !m_bPressed)
	{
		CRect rectAccent(rect.left, rect.bottom - 2, rect.right, rect.bottom);
		dc.FillSolidRect(rectAccent, Theme::Accent);
	}

	CString strText;
	GetWindowText(strText);

	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(m_bPressed ? Theme::AccentText : (IsWindowEnabled() ? Theme::Text : Theme::TextDisabled));

	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);
	dc.DrawText(strText, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	dc.SelectObject(pOldFont);
}

void CFlatButton::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bTrackingMouse)
	{
		TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.hwndTrack = GetSafeHwnd();
		tme.dwHoverTime = 1;
		_TrackMouseEvent(&tme);
		m_bTrackingMouse = true;
	}

	if (!m_bHover)
	{
		m_bHover = true;
		Invalidate();
	}

	CButton::OnMouseMove(nFlags, point);
}

LRESULT CFlatButton::OnMouseHover(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	if (!m_bHover)
	{
		m_bHover = true;
		Invalidate();
	}
	return 0;
}

void CFlatButton::OnMouseLeave()
{
	m_bTrackingMouse = false;
	m_bHover = false;
	m_bPressed = false;
	Invalidate();
}

void CFlatButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bPressed = true;
	Invalidate();
	CButton::OnLButtonDown(nFlags, point);
}

void CFlatButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bPressed = false;
	Invalidate();
	CButton::OnLButtonUp(nFlags, point);
}
