#include "pch.h"
#include "ActionBar.h"
#include "Resource.h"
#include "Theme.h"

BEGIN_MESSAGE_MAP(CActionBar, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_BN_CLICKED(ID_TABLE_LOAD_RDF, &CActionBar::OnLoadRdf)
	ON_BN_CLICKED(ID_TABLE_SAVE_RDF, &CActionBar::OnSaveRdf)
	ON_BN_CLICKED(ID_TABLE_LOAD_XML, &CActionBar::OnLoadXml)
	ON_BN_CLICKED(ID_TABLE_SAVE_XML, &CActionBar::OnSaveXml)
END_MESSAGE_MAP()

CActionBar::CActionBar()
{
}

BOOL CActionBar::Create(CWnd* pParentWnd, UINT nID)
{
	static CString strClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
		::LoadCursor(nullptr, IDC_ARROW), (HBRUSH)nullptr, nullptr);

	return CWnd::Create(strClass, _T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), pParentWnd, nID);
}

int CActionBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	const DWORD dwStyle = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT;

	m_btnLoadRdf.Create(_T("Load RDF..."), dwStyle, rectDummy, this, ID_TABLE_LOAD_RDF);
	m_btnSaveRdf.Create(_T("Save RDF..."), dwStyle, rectDummy, this, ID_TABLE_SAVE_RDF);
	m_btnLoadXml.Create(_T("Load XML..."), dwStyle, rectDummy, this, ID_TABLE_LOAD_XML);
	m_btnSaveXml.Create(_T("Save XML..."), dwStyle, rectDummy, this, ID_TABLE_SAVE_XML);

	AdjustLayout();

	return 0;
}

void CActionBar::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CActionBar::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	const int cyMargin = 6;
	const int cxBtn = 96;
	const int cxGap = 4;

	int x = rectClient.right - cyMargin - cxBtn;
	int cyBtn = rectClient.Height() - cyMargin * 2;

	m_btnSaveXml.SetWindowPos(nullptr, x, cyMargin, cxBtn, cyBtn, SWP_NOACTIVATE | SWP_NOZORDER);
	x -= cxBtn + cxGap;
	m_btnLoadXml.SetWindowPos(nullptr, x, cyMargin, cxBtn, cyBtn, SWP_NOACTIVATE | SWP_NOZORDER);
	x -= cxBtn + cxGap * 2;
	m_btnSaveRdf.SetWindowPos(nullptr, x, cyMargin, cxBtn, cyBtn, SWP_NOACTIVATE | SWP_NOZORDER);
	x -= cxBtn + cxGap;
	m_btnLoadRdf.SetWindowPos(nullptr, x, cyMargin, cxBtn, cyBtn, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CActionBar::OnPaint()
{
	CPaintDC dc(this);

	CRect rectClient;
	GetClientRect(rectClient);

	dc.FillSolidRect(rectClient, Theme::Bg2);

	CRect rectBorder(rectClient.left, rectClient.bottom - 1, rectClient.right, rectClient.bottom);
	dc.FillSolidRect(rectBorder, Theme::Border);

	CRect rectTitle(12, 0, 260, rectClient.bottom);
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(Theme::Text);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontBold);
	dc.DrawText(_T("DBO Table Editor"), rectTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	dc.SelectObject(pOldFont);
}

void CActionBar::OnLoadRdf()
{
	if (CWnd* pFrame = GetParentFrame())
	{
		pFrame->SendMessage(WM_COMMAND, MAKEWPARAM(ID_TABLE_LOAD_RDF, 0));
	}
}

void CActionBar::OnSaveRdf()
{
	if (CWnd* pFrame = GetParentFrame())
	{
		pFrame->SendMessage(WM_COMMAND, MAKEWPARAM(ID_TABLE_SAVE_RDF, 0));
	}
}

void CActionBar::OnLoadXml()
{
	if (CWnd* pFrame = GetParentFrame())
	{
		pFrame->SendMessage(WM_COMMAND, MAKEWPARAM(ID_TABLE_LOAD_XML, 0));
	}
}

void CActionBar::OnSaveXml()
{
	if (CWnd* pFrame = GetParentFrame())
	{
		pFrame->SendMessage(WM_COMMAND, MAKEWPARAM(ID_TABLE_SAVE_XML, 0));
	}
}
