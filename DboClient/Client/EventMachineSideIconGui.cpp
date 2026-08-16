#include "precomp_dboclient.h"
#include "EventMachineSideIconGui.h"

// core
#include "NtlDebug.h"

// presentation
#include "NtlPLGuiManager.h"

// dbo
#include "DboEvent.h"
#include "DboEventGenerator.h"
#include "DboPacketGenerator.h"
#include "DialogManager.h"
#include "DboGlobal.h"
#include "HLShopGui.h"

CEventMachineSideIconGui::CEventMachineSideIconGui(const RwChar* pName)
	: CSideIconBase(pName)
	, m_pBtnWagu(NULL)
{
}

CEventMachineSideIconGui::~CEventMachineSideIconGui(void)
{
}

RwBool CEventMachineSideIconGui::Create()
{
	NTL_FUNCTION(__FUNCTION__);

	if (!CNtlPLGui::Create("gui\\EventMachineSideIcon.rsr", "gui\\EventMachineSideIcon.srf", "gui\\EventMachineSideIcon.frm"))
		NTL_RETURN(FALSE);

	CNtlPLGui::CreateComponents(CNtlPLGuiManager::GetInstance()->GetGuiManager());

	m_pThis = (gui::CDialog*)GetComponent("dlgMain");

	m_pBtnWagu = (gui::CButton*)GetComponent("btnIcon");
	if (m_pBtnWagu)
	{
		m_slotWaguBtn = m_pBtnWagu->SigClicked().Connect(this, &CEventMachineSideIconGui::OnIconButtonClicked);
		m_slotWaguMouseEnter = m_pBtnWagu->SigMouseEnter().Connect(this, &CEventMachineSideIconGui::OnMouseEnter);
		m_slotWaguMouseLeave = m_pBtnWagu->SigMouseLeave().Connect(this, &CEventMachineSideIconGui::OnMouseLeave);
	}

	Show(true);

	NTL_RETURN(TRUE);
}

VOID CEventMachineSideIconGui::Destroy()
{
	CNtlPLGui::DestroyComponents();
	CNtlPLGui::Destroy();
	Show(false);
}

VOID CEventMachineSideIconGui::HandleEvents(RWS::CMsg &msg)
{
}

VOID CEventMachineSideIconGui::OnIconButtonClicked(gui::CComponent* pComponent)
{
	if (GetDialogManager()->IsOpenDialog(DIALOG_HLSHOP) == FALSE)
		GetDboGlobal()->GetGamePacketGenerator()->SendCashItemHLShopStartReq();

	CDboEventGenerator::HLShopSelectCategory(eHLS_CATEGORY_EVENT_MACHINE);
}

VOID CEventMachineSideIconGui::OnSideViewClosed()
{
}

void CEventMachineSideIconGui::OnMouseEnter(gui::CComponent* pComponent)
{
	CSideIconGui::GetInstance()->OpenSideView(this, SIDEVIEW_EVENT_WAGU, NULL);
}

void CEventMachineSideIconGui::OnMouseLeave(gui::CComponent* pComponent)
{
	CSideIconGui::GetInstance()->CloseSideView(SIDEVIEW_EVENT_WAGU);
}

void CEventMachineSideIconGui::Show(bool bShow)
{
	__super::Show(bShow);
}
