#include "precomp_dboclient.h"
#include "WaguMachineSideViewGui.h"

// core
#include "NtlDebug.h"

// presentation
#include "NtlPLGuiManager.h"

// sl
#include "NtlSLLogic.h"

// dbo
#include "DisplayStringManager.h"
#include "DialogDefine.h"

CWaguMachineSideViewGui::CWaguMachineSideViewGui(const RwChar* pName)
	: CSideViewBase(pName)
	, m_stbViewName(NULL)
{
}

CWaguMachineSideViewGui::~CWaguMachineSideViewGui(void)
{
}

RwBool CWaguMachineSideViewGui::Create()
{
	if (!CNtlPLGui::Create("", "gui\\WaguMachineSideIcon.srf", "gui\\WaguMachineSideView.frm"))
		return FALSE;

	CNtlPLGui::CreateComponents(CNtlPLGuiManager::GetInstance()->GetGuiManager());

	m_pThis = (gui::CDialog*)GetComponent("dlgMain");

	// background
	m_BackPanel.SetType(CWindowby3::WT_HORIZONTAL);
	m_BackPanel.SetSurface(0, GetNtlGuiManager()->GetSurfaceManager()->GetSurface("WaguMachineSideIcon.srf", "srfDialogBackUp"));
	m_BackPanel.SetSurface(1, GetNtlGuiManager()->GetSurfaceManager()->GetSurface("WaguMachineSideIcon.srf", "srfDialogBackCenter"));
	m_BackPanel.SetSurface(2, GetNtlGuiManager()->GetSurfaceManager()->GetSurface("WaguMachineSideIcon.srf", "srfDialogBackDown"));

	// sig
	m_slotPaint = m_pThis->SigPaint().Connect(this, &CWaguMachineSideViewGui::OnPaint);
	m_slotMove = m_pThis->SigMove().Connect(this, &CWaguMachineSideViewGui::OnMove);
	m_slotResize = m_pThis->SigMove().Connect(this, &CWaguMachineSideViewGui::OnResize);

	m_stbViewName = (gui::CStaticBox*)GetComponent("stbViewName");
	if (m_stbViewName)
		m_stbViewName->SetText(GetDisplayStringManager()->GetString("DST_WAGUWAGU_MACHINE"));

	Show(false);

	return TRUE;
}

VOID CWaguMachineSideViewGui::Destroy()
{
	CNtlPLGui::DestroyComponents();
	CNtlPLGui::Destroy();
}

VOID CWaguMachineSideViewGui::OnPressESC()
{
}

VOID CWaguMachineSideViewGui::OnSideViewOpen(const void* pData)
{
	Show(true);
}

VOID CWaguMachineSideViewGui::OnSideViewClose()
{
	Show(false);
}

VOID CWaguMachineSideViewGui::OnSideViewLocate(const CRectangle& rectSideIcon)
{
	LocateComponent();
	m_pThis->SetPosition(rectSideIcon.left - m_pThis->GetWidth() + rectSideIcon.GetWidth(), rectSideIcon.top - m_pThis->GetHeight());
}

VOID CWaguMachineSideViewGui::HandleEvents(RWS::CMsg &msg)
{
}

VOID CWaguMachineSideViewGui::LocateComponent()
{
	m_BackPanel.SetRect(m_pThis->GetScreenRect());
}

VOID CWaguMachineSideViewGui::OnMove(RwInt32 iOldX, RwInt32 iOldY)
{
	LocateComponent();
}

VOID CWaguMachineSideViewGui::OnResize(RwInt32 iOldW, RwInt32 iOldH)
{
	LocateComponent();
}

VOID CWaguMachineSideViewGui::OnPaint()
{
	m_BackPanel.Render();
}
