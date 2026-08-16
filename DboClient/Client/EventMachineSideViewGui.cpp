#include "precomp_dboclient.h"
#include "EventMachineSideViewGui.h"

// core
#include "NtlDebug.h"

// presentation
#include "NtlPLGuiManager.h"

// sl
#include "NtlSLLogic.h"

// dbo
#include "DisplayStringManager.h"
#include "DialogDefine.h"
#include "DialogManager.h"

CEventMachineSideViewGui::CEventMachineSideViewGui(const RwChar* pName)
	: CSideViewBase(pName)
	, m_stbViewName(NULL)
{
}

CEventMachineSideViewGui::~CEventMachineSideViewGui(void)
{
}

RwBool CEventMachineSideViewGui::Create()
{
	if (!CNtlPLGui::Create("", "gui\\EventMachineSideIcon.srf", "gui\\EventMachineSideView.frm"))
		return FALSE;

	CNtlPLGui::CreateComponents(CNtlPLGuiManager::GetInstance()->GetGuiManager());

	m_pThis = (gui::CDialog*)GetComponent("dlgMain");

	// background
	m_BackPanel.SetType(CWindowby3::WT_HORIZONTAL);
	m_BackPanel.SetSurface(0, GetNtlGuiManager()->GetSurfaceManager()->GetSurface("EventMachineSideIcon.srf", "srfDialogBackUp"));
	m_BackPanel.SetSurface(1, GetNtlGuiManager()->GetSurfaceManager()->GetSurface("EventMachineSideIcon.srf", "srfDialogBackCenter"));
	m_BackPanel.SetSurface(2, GetNtlGuiManager()->GetSurfaceManager()->GetSurface("EventMachineSideIcon.srf", "srfDialogBackDown"));

	// sig
	m_slotPaint = m_pThis->SigPaint().Connect(this, &CEventMachineSideViewGui::OnPaint);
	m_slotMove = m_pThis->SigMove().Connect(this, &CEventMachineSideViewGui::OnMove);
	m_slotResize = m_pThis->SigMove().Connect(this, &CEventMachineSideViewGui::OnResize);

	m_stbViewName = (gui::CStaticBox*)GetComponent("stbViewName");

	Show(false);

	return TRUE;
}

VOID CEventMachineSideViewGui::Destroy()
{
	CNtlPLGui::DestroyComponents();
	CNtlPLGui::Destroy();
}

VOID CEventMachineSideViewGui::OnPressESC()
{
}

VOID CEventMachineSideViewGui::OnSideViewOpen(const void* pData)
{
	SetText();
	Show(true);
}

VOID CEventMachineSideViewGui::OnSideViewClose()
{
	Show(false);
}

VOID CEventMachineSideViewGui::OnSideViewLocate(const CRectangle& rectSideIcon)
{
	LocateComponent();
	m_pThis->SetPosition(rectSideIcon.left - m_pThis->GetWidth() + rectSideIcon.GetWidth(), rectSideIcon.top - m_pThis->GetHeight());
}

VOID CEventMachineSideViewGui::HandleEvents(RWS::CMsg &msg)
{
}

VOID CEventMachineSideViewGui::LocateComponent()
{
	m_BackPanel.SetRect(m_pThis->GetScreenRect());
}

VOID CEventMachineSideViewGui::OnMove(RwInt32 iOldX, RwInt32 iOldY)
{
	LocateComponent();
}

VOID CEventMachineSideViewGui::OnResize(RwInt32 iOldW, RwInt32 iOldH)
{
	LocateComponent();
}

VOID CEventMachineSideViewGui::OnPaint()
{
	m_BackPanel.Render();
}

VOID CEventMachineSideViewGui::SetText()
{
	if (!m_stbViewName)
		return;

	m_stbViewName->Clear();
	WCHAR buf[256];
	swprintf_s(buf, 256, GetDisplayStringManager()->GetString("DST_EVENTMACHINE_SIDEICON_TOOLTIP"), Logic_GetEventCoin());
	m_stbViewName->SetText(buf);
}
