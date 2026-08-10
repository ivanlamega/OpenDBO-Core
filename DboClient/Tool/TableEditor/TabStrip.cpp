#include "pch.h"
#include "TabStrip.h"
#include "Theme.h"

namespace
{
	const int kArrowWidth = 24;
	const int kTabPadding = 32;
	const int kTabMinWidth = 70;
	const int kTabMaxWidth = 220;
}

BEGIN_MESSAGE_MAP(CTabStrip, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

CTabStrip::CTabStrip()
	: m_nCurSel(-1)
	, m_nHotIndex(-1)
	, m_nScrollOffset(0)
	, m_bTrackingMouse(false)
{
}

BOOL CTabStrip::Create(CWnd* pParentWnd, UINT nID)
{
	static CString strClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
		::LoadCursor(nullptr, IDC_ARROW), (HBRUSH)nullptr, nullptr);

	return CWnd::Create(strClass, _T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), pParentWnd, nID);
}

int CTabStrip::AddTab(const CString& strText)
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);
	CSize sz = dc.GetTextExtent(strText);
	dc.SelectObject(pOldFont);

	STabItem item;
	item.strText = strText;
	item.nWidth = sz.cx + kTabPadding;
	if (item.nWidth < kTabMinWidth)
	{
		item.nWidth = kTabMinWidth;
	}
	if (item.nWidth > kTabMaxWidth)
	{
		item.nWidth = kTabMaxWidth;
	}

	m_aTabs.push_back(item);
	Invalidate();
	return (int)m_aTabs.size() - 1;
}

void CTabStrip::DeleteAllTabs()
{
	m_aTabs.clear();
	m_nCurSel = -1;
	m_nHotIndex = -1;
	m_nScrollOffset = 0;
	Invalidate();
}

void CTabStrip::SetCurSel(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_aTabs.size())
	{
		return;
	}

	m_nCurSel = nIndex;
	EnsureVisible(nIndex);
	Invalidate();
}

bool CTabStrip::NeedsScrolling() const
{
	CRect rc;
	GetClientRect(rc);
	return GetContentWidth() > rc.Width();
}

int CTabStrip::GetContentWidth() const
{
	int nWidth = 0;
	for (size_t i = 0; i < m_aTabs.size(); ++i)
	{
		nWidth += m_aTabs[i].nWidth;
	}
	return nWidth;
}

CRect CTabStrip::GetContentRect() const
{
	CRect rc;
	GetClientRect(rc);
	if (NeedsScrolling())
	{
		rc.left += kArrowWidth;
		rc.right -= kArrowWidth;
	}
	return rc;
}

CRect CTabStrip::GetTabRect(int nIndex) const
{
	CRect contentRect = GetContentRect();

	int x = contentRect.left - m_nScrollOffset;
	for (int i = 0; i < nIndex; ++i)
	{
		x += m_aTabs[i].nWidth;
	}

	return CRect(x, contentRect.top, x + m_aTabs[nIndex].nWidth, contentRect.bottom);
}

int CTabStrip::HitTest(CPoint ptClient) const
{
	CRect contentRect = GetContentRect();
	if (!contentRect.PtInRect(ptClient))
	{
		return -1;
	}

	for (int i = 0; i < (int)m_aTabs.size(); ++i)
	{
		if (GetTabRect(i).PtInRect(ptClient))
		{
			return i;
		}
	}
	return -1;
}

void CTabStrip::ScrollBy(int nDelta)
{
	CRect contentRect = GetContentRect();
	int nMaxScroll = GetContentWidth() - contentRect.Width();
	if (nMaxScroll < 0)
	{
		nMaxScroll = 0;
	}

	m_nScrollOffset += nDelta;
	if (m_nScrollOffset < 0)
	{
		m_nScrollOffset = 0;
	}
	if (m_nScrollOffset > nMaxScroll)
	{
		m_nScrollOffset = nMaxScroll;
	}

	Invalidate();
}

void CTabStrip::EnsureVisible(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_aTabs.size())
	{
		return;
	}

	CRect contentRect = GetContentRect();

	int x = 0;
	for (int i = 0; i < nIndex; ++i)
	{
		x += m_aTabs[i].nWidth;
	}
	int w = m_aTabs[nIndex].nWidth;

	if (x < m_nScrollOffset)
	{
		m_nScrollOffset = x;
	}
	else if (x + w > m_nScrollOffset + contentRect.Width())
	{
		m_nScrollOffset = x + w - contentRect.Width();
	}

	int nMaxScroll = GetContentWidth() - contentRect.Width();
	if (nMaxScroll < 0)
	{
		nMaxScroll = 0;
	}
	if (m_nScrollOffset < 0)
	{
		m_nScrollOffset = 0;
	}
	if (m_nScrollOffset > nMaxScroll)
	{
		m_nScrollOffset = nMaxScroll;
	}
}

BOOL CTabStrip::OnEraseBkgnd(CDC* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	return TRUE;
}

void CTabStrip::OnPaint()
{
	CPaintDC dc(this);

	CRect rectClient;
	GetClientRect(rectClient);
	dc.FillSolidRect(rectClient, Theme::Bg1);

	CRect contentRect = GetContentRect();

	dc.SetBkMode(TRANSPARENT);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	int nClip = dc.SaveDC();
	dc.IntersectClipRect(contentRect);

	for (int i = 0; i < (int)m_aTabs.size(); ++i)
	{
		CRect rectTab = GetTabRect(i);
		if (rectTab.right < contentRect.left || rectTab.left > contentRect.right)
		{
			continue;
		}

		bool bSelected = (i == m_nCurSel);
		bool bHot = (i == m_nHotIndex);

		dc.FillSolidRect(rectTab, (bSelected || bHot) ? Theme::Bg2 : Theme::Bg1);
		dc.SetTextColor((bSelected || bHot) ? Theme::Text : Theme::TextMuted);
		dc.DrawText(m_aTabs[i].strText, rectTab, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

		if (bSelected)
		{
			CRect rectAccent(rectTab.left, rectTab.bottom - 2, rectTab.right, rectTab.bottom);
			dc.FillSolidRect(rectAccent, Theme::Accent);
		}
	}

	dc.RestoreDC(nClip);

	if (NeedsScrolling())
	{
		CRect rectLeft(0, 0, kArrowWidth, rectClient.Height());
		CRect rectRight(rectClient.right - kArrowWidth, 0, rectClient.right, rectClient.Height());

		bool bCanScrollLeft = (m_nScrollOffset > 0);
		bool bCanScrollRight = (m_nScrollOffset < GetContentWidth() - contentRect.Width());

		dc.FillSolidRect(rectLeft, Theme::Bg2);
		dc.FillSolidRect(rectRight, Theme::Bg2);

		dc.SetTextColor(bCanScrollLeft ? Theme::Text : Theme::TextDisabled);
		dc.DrawText(_T("<"), rectLeft, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		dc.SetTextColor(bCanScrollRight ? Theme::Text : Theme::TextDisabled);
		dc.DrawText(_T(">"), rectRight, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	dc.SelectObject(pOldFont);
}

void CTabStrip::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	CRect contentRect = GetContentRect();
	int nMaxScroll = GetContentWidth() - contentRect.Width();
	if (nMaxScroll < 0)
	{
		nMaxScroll = 0;
	}
	if (m_nScrollOffset > nMaxScroll)
	{
		m_nScrollOffset = nMaxScroll;
	}

	Invalidate();
}

void CTabStrip::OnLButtonDown(UINT nFlags, CPoint point)
{
	int nHit = HitTest(point);
	if (nHit >= 0)
	{
		if (nHit != m_nCurSel)
		{
			m_nCurSel = nHit;
			EnsureVisible(nHit);
			Invalidate();

			CWnd* pParent = GetParent();
			if (pParent)
			{
				pParent->SendMessage(TBSTN_SELCHANGE, (WPARAM)nHit, 0);
			}
		}
		return;
	}

	if (NeedsScrolling())
	{
		CRect rectClient;
		GetClientRect(rectClient);
		CRect rectLeft(0, 0, kArrowWidth, rectClient.Height());
		CRect rectRight(rectClient.right - kArrowWidth, 0, rectClient.right, rectClient.Height());

		if (rectLeft.PtInRect(point))
		{
			ScrollBy(-120);
			return;
		}
		if (rectRight.PtInRect(point))
		{
			ScrollBy(120);
			return;
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CTabStrip::OnRButtonDown(UINT nFlags, CPoint point)
{
	CWnd* pParent = GetParent();
	if (pParent)
	{
		pParent->SendMessage(TBSTN_RCLICK, 0, 0);
	}

	CWnd::OnRButtonDown(nFlags, point);
}

void CTabStrip::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bTrackingMouse)
	{
		TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = GetSafeHwnd();
		_TrackMouseEvent(&tme);
		m_bTrackingMouse = true;
	}

	int nHit = HitTest(point);
	if (nHit != m_nHotIndex)
	{
		m_nHotIndex = nHit;
		Invalidate();
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CTabStrip::OnMouseLeave()
{
	m_bTrackingMouse = false;
	if (m_nHotIndex != -1)
	{
		m_nHotIndex = -1;
		Invalidate();
	}
}

BOOL CTabStrip::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	UNREFERENCED_PARAMETER(nFlags);
	UNREFERENCED_PARAMETER(pt);

	if (NeedsScrolling())
	{
		ScrollBy(-(int)zDelta / 2);
	}
	return TRUE;
}
