#include "pch.h"
#include "ProgressDlg.h"

BEGIN_MESSAGE_MAP(CProgressPopup, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CProgressPopup::Create(CWnd* pParentWnd, const CString& strMessage)
{
	m_strMessage = strMessage;

	static CString strClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
		::LoadCursor(nullptr, IDC_WAIT), (HBRUSH)(COLOR_3DFACE + 1), nullptr);

	const int cx = 340, cy = 110;
	CRect rect(0, 0, cx, cy);

	if (pParentWnd)
	{
		CRect rectParent;
		pParentWnd->GetWindowRect(&rectParent);
		rect.OffsetRect(rectParent.CenterPoint().x - cx / 2, rectParent.CenterPoint().y - cy / 2);
	}

	if (!CreateEx(WS_EX_DLGMODALFRAME, strClass, _T("Please Wait"),
		WS_POPUP | WS_CAPTION | WS_VISIBLE, rect, pParentWnd, 0))
	{
		return FALSE;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	m_wndBar.Create(WS_CHILD | WS_VISIBLE | PBS_MARQUEE, CRect(20, 55, rectClient.right - 20, 75), this, 1);
	m_wndBar.SendMessage(PBM_SETMARQUEE, TRUE, 30);

	UpdateWindow();

	return TRUE;
}

void CProgressPopup::OnPaint()
{
	CPaintDC dc(this);

	CRect rectClient;
	GetClientRect(rectClient);

	CRect rectText(20, 15, rectClient.right - 20, 50);
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(m_strMessage, rectText, DT_LEFT | DT_WORDBREAK);
}

namespace {

struct SProgressTask
{
	std::function<bool()> fn;
	bool bResult = false;
};

UINT AFX_CDECL ProgressThreadProc(LPVOID pParam)
{
	SProgressTask* pTask = (SProgressTask*)pParam;

	// Some tasks (Load XML) go through CNtlXMLDoc, which creates an MSXML
	// COM object -- COM needs to be initialized on whichever thread touches
	// it, and MFC's worker-thread overload of AfxBeginThread doesn't do
	// that automatically the way its UI-thread overload would.
	HRESULT hrCoInit = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	pTask->bResult = pTask->fn();

	if (SUCCEEDED(hrCoInit))
	{
		::CoUninitialize();
	}

	return 0;
}

} // namespace

bool RunWithProgress(CWnd* pParentWnd, const CString& strMessage, std::function<bool()> fn)
{
	SProgressTask task;
	task.fn = fn;

	CWinThread* pThread = AfxBeginThread(ProgressThreadProc, &task, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		// Couldn't spin up a worker thread -- fall back to running inline
		// rather than losing the operation entirely.
		return fn();
	}

	pThread->m_bAutoDelete = FALSE;
	HANDLE hThread = pThread->m_hThread;
	pThread->ResumeThread();

	CProgressPopup wndProgress;
	wndProgress.Create(pParentWnd, strMessage);

	if (pParentWnd)
	{
		pParentWnd->EnableWindow(FALSE);
	}

	for (;;)
	{
		DWORD dwWait = ::MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT);
		if (dwWait == WAIT_OBJECT_0)
		{
			break;
		}

		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				::PostQuitMessage((int)msg.wParam);
				continue;
			}

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	if (pParentWnd)
	{
		pParentWnd->EnableWindow(TRUE);
	}
	wndProgress.DestroyWindow();

	::WaitForSingleObject(hThread, INFINITE);
	delete pThread;

	return task.bResult;
}
