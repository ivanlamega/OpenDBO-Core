
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "pch.h"
#include "framework.h"
#include "TableEditor.h"

#include "MainFrm.h"
#include "ClassView.h"
#include "PropertiesWnd.h"
#include "Util.h"
#include "ProgressDlg.h"
#include "Theme.h"

#include <atlconv.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

// Not guaranteed to be declared in the SDK headers this toolset ships
// with (they were added for Windows 11) -- the DWM API itself has been
// stable since Vista, so defining the numeric values directly and calling
// it is safe; it just no-ops (returns an error we ignore) on older
// Windows where the attribute doesn't exist.
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif
#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif
#ifndef DWMWA_TEXT_COLOR
#define DWMWA_TEXT_COLOR 36
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_TABLE_LOAD_RDF, &CMainFrame::OnTableLoadRdf)
	ON_COMMAND(ID_TABLE_SAVE_RDF, &CMainFrame::OnTableSaveRdf)
	ON_COMMAND(ID_TABLE_LOAD_XML, &CMainFrame::OnTableLoadXml)
	ON_COMMAND(ID_TABLE_SAVE_XML, &CMainFrame::OnTableSaveXml)
END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// This app doesn't use the doc/view machinery for anything real --
	// all content lives in the panels created below, which fully cover
	// the client area. Hide *and* zero-size the framework's initial view
	// so it can't steal focus, show through, or leave a stray gray patch
	// behind if the panels don't cover every pixel on some resize.
	if (CWnd* pView = GetActiveView())
	{
		pView->ShowWindow(SW_HIDE);
		pView->SetWindowPos(nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
	}

	// Flat, modern skin for whatever bit of MFC chrome still shows through
	// (the property grid's own small toolbar) -- this MFC toolset tops
	// out at Office2007, there's no "2016"-style manager available to
	// link against here. Everything else (action bar, tabs, tree, search
	// bar) is fully self-drawn instead of relying on a visual manager.
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
	CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);

	// No native Win32 dropdown menu -- CActionBar below replaces it.
	SetMenu(nullptr);

	ApplyModernChrome();

	if (!m_wndActionBar.Create(this, ID_VIEW_ACTIONBAR))
	{
		TRACE0("Failed to create action bar\n");
		return -1;
	}

	// No CMFCToolBar / dockable content panes below the action bar -- a
	// fixed layout of plain child windows reads as a flat tool instead
	// of a VS-style IDE.
	if (!m_wndFileView.Create(this, ID_VIEW_FILEVIEW))
	{
		TRACE0("Failed to create File View window\n");
		return -1;
	}

	if (!m_wndFileView.m_wndClassView.Create(this, ID_VIEW_CLASSVIEW))
	{
		TRACE0("Failed to create Class View window\n");
		return -1;
	}

	if (!m_wndFileView.m_wndClassView.m_wndProperties.Create(this, ID_VIEW_PROPERTIESWND))
	{
		TRACE0("Failed to create Properties window\n");
		return -1;
	}

	RepositionPanels();

	return 0;
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWndEx::OnSize(nType, cx, cy);
	RepositionPanels();
}

void CMainFrame::RepositionPanels()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	const int cyActionBar = 40;
	m_wndActionBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyActionBar, SWP_NOACTIVATE | SWP_NOZORDER);

	int nTop = rectClient.top + cyActionBar;

	const int cyTabs = 36;
	const int cxGap = 1;

	m_wndFileView.SetWindowPos(nullptr, rectClient.left, nTop, rectClient.Width(), cyTabs, SWP_NOACTIVATE | SWP_NOZORDER);

	int nContentTop = nTop + cyTabs;
	int nContentHeight = rectClient.bottom - nContentTop;
	if (nContentHeight < 0)
	{
		nContentHeight = 0;
	}

	// Row list gets roughly a third of the width, the property grid gets
	// the rest -- matching a typical id/name list next to a wider detail
	// grid, rather than splitting the window 50/50.
	int nListWidth = (int)(rectClient.Width() * 0.36);
	if (nListWidth > rectClient.Width())
	{
		nListWidth = rectClient.Width();
	}

	m_wndFileView.m_wndClassView.SetWindowPos(nullptr, rectClient.left, nContentTop, nListWidth, nContentHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	int nGridLeft = rectClient.left + nListWidth + cxGap;
	int nGridWidth = rectClient.Width() - nListWidth - cxGap;
	if (nGridWidth < 0)
	{
		nGridWidth = 0;
	}

	m_wndFileView.m_wndClassView.m_wndProperties.SetWindowPos(nullptr, nGridLeft, nContentTop, nGridWidth, nContentHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}

// Windows 11 DWM touches -- rounded corners and a dark, theme-matching
// title bar/border, so the one bit of chrome this app can't paint itself
// (the actual OS window frame) doesn't look like it belongs to a
// different, older application. All of these quietly no-op (non-zero
// HRESULT, ignored) on Windows versions that don't support them.
void CMainFrame::ApplyModernChrome()
{
	HWND hWnd = GetSafeHwnd();
	if (!hWnd)
	{
		return;
	}

	DWORD dwCornerPref = DWMWCP_ROUND;
	::DwmSetWindowAttribute(hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &dwCornerPref, sizeof(dwCornerPref));

	BOOL bDarkMode = TRUE;
	::DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &bDarkMode, sizeof(bDarkMode));

	COLORREF clrCaption = Theme::Bg1;
	::DwmSetWindowAttribute(hWnd, DWMWA_CAPTION_COLOR, &clrCaption, sizeof(clrCaption));

	COLORREF clrText = Theme::Text;
	::DwmSetWindowAttribute(hWnd, DWMWA_TEXT_COLOR, &clrText, sizeof(clrText));

	COLORREF clrBorder = Theme::Border;
	::DwmSetWindowAttribute(hWnd, DWMWA_BORDER_COLOR, &clrBorder, sizeof(clrBorder));
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC)
{
	CRect rectClient;
	GetClientRect(rectClient);
	pDC->FillSolidRect(rectClient, Theme::Bg0);
	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	UNREFERENCED_PARAMETER(bAddToTitle);
	SetWindowText(_T("DBO Table Editor"));
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// Plain child windows don't get their own PreTranslateMessage called
	// by the message pump, so Enter-in-the-search-box is routed here.
	if (m_wndFileView.m_wndClassView.HandleSearchKeyDown(pMsg))
	{
		return TRUE;
	}

	return CFrameWndEx::PreTranslateMessage(pMsg);
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

bool CMainFrame::ReloadTables(const CString& strPath, CTable::eLOADING_METHOD eLoadingMethod)
{
	// Clear anything pointing into the table container we're about to
	// destroy -- the sTBLDAT* pointers held by the row list and the
	// property grid become dangling once it's deleted.
	m_wndFileView.m_wndClassView.ResetView();
	m_wndFileView.m_wndClassView.m_wndProperties.LoadTableData(-1, nullptr);

	DeleteTableContainer();

	CString strPathCopy = strPath;
	bool bSuccess = RunWithProgress(this, _T("Loading tables..."), [strPathCopy, eLoadingMethod]() -> bool
	{
		// CreateTableContainer takes a narrow path (it calls straight into
		// the shared table engine, which is narrow-only) -- CT2A converts
		// from whatever strPathCopy's width is.
		CT2A pszPath(strPathCopy);
		return CreateTableContainer(pszPath, eLoadingMethod);
	});

	if (!bSuccess)
	{
		AfxMessageBox(_T("Failed to load tables from:\n") + strPath);
		return false;
	}

	m_wndFileView.RefreshTables();

	return true;
}

void CMainFrame::OnTableLoadRdf()
{
	CFolderPickerDialog dlg(nullptr, 0, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	// CTableContainer always appends its own separator before the file
	// name (see DBO_EXPORT_TABLE / InitializeTable), so pass the folder
	// path without a trailing backslash.
	ReloadTables(dlg.GetPathName(), CTable::LOADING_METHOD_BINARY);
}

void CMainFrame::OnTableSaveRdf()
{
	if (!GetTableContainer())
	{
		AfxMessageBox(_T("No tables are loaded."));
		return;
	}

	CFolderPickerDialog dlg(nullptr, 0, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	CString strPath = dlg.GetPathName();

	bool bSuccess = RunWithProgress(this, _T("Saving tables..."), [strPath]() -> bool
	{
		// SaveTableContainer takes a narrow path (it calls straight into
		// the shared table engine, which is narrow-only) -- CT2A converts
		// from whatever strPath's width is.
		CT2A pszPath(strPath);
		return SaveTableContainer(pszPath, false);
	});

	if (!bSuccess)
	{
		AfxMessageBox(_T("Failed to save tables to:\n") + strPath);
		return;
	}

	AfxMessageBox(_T("Tables saved."));
}

void CMainFrame::OnTableLoadXml()
{
	CFolderPickerDialog dlg(nullptr, 0, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	ReloadTables(dlg.GetPathName(), CTable::LOADING_METHOD_XML);
}

void CMainFrame::OnTableSaveXml()
{
	// The table engine only knows how to write the .rdf/.edf binary format
	// (see CTableContainer::SaveToFile / DBO_EXPORT_TABLE) -- there is no
	// XML writer counterpart to CTable::LoadFromXml yet. Wiring this up
	// means adding a SaveToXml path per table in DboShared/NtlGameTable.
	AfxMessageBox(_T("Saving to XML is not implemented yet."));
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	// base class does the real work

	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	return TRUE;
}
