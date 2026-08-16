#include "precomp_dboclient.h"
#include "WaguMachineSideIconGui.h"

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

CWaguMachineSideIconGui::CWaguMachineSideIconGui(const RwChar* pName)
	: CSideIconBase(pName)
	, m_pBtnWagu(NULL)
{
}

CWaguMachineSideIconGui::~CWaguMachineSideIconGui(void)
{
}

RwBool CWaguMachineSideIconGui::Create()
{
	NTL_FUNCTION(__FUNCTION__);

	if (!CNtlPLGui::Create("gui\\WaguMachineSideIcon.rsr", "gui\\WaguMachineSideIcon.srf", "gui\\WaguMachineSideIcon.frm"))
		NTL_RETURN(FALSE);

	CNtlPLGui::CreateComponents(CNtlPLGuiManager::GetInstance()->GetGuiManager());

	m_pThis = (gui::CDialog*)GetComponent("dlgMain");

	m_pBtnWagu = (gui::CButton*)GetComponent("btnIcon");
	if (m_pBtnWagu)
	{
		m_slotWaguBtn = m_pBtnWagu->SigClicked().Connect(this, &CWaguMachineSideIconGui::OnIconButtonClicked);
		m_slotWaguMouseEnter = m_pBtnWagu->SigMouseEnter().Connect(this, &CWaguMachineSideIconGui::OnMouseEnter);
		m_slotWaguMouseLeave = m_pBtnWagu->SigMouseLeave().Connect(this, &CWaguMachineSideIconGui::OnMouseLeave);
	}

	Show(true);

	NTL_RETURN(TRUE);
}

VOID CWaguMachineSideIconGui::Destroy()
{
	CNtlPLGui::DestroyComponents();
	CNtlPLGui::Destroy();
	Show(false);
}

VOID CWaguMachineSideIconGui::HandleEvents(RWS::CMsg &msg)
{
}

VOID CWaguMachineSideIconGui::OnIconButtonClicked(gui::CComponent* pComponent)
{
	if (GetDialogManager()->IsOpenDialog(DIALOG_HLSHOP) == FALSE)
		GetDboGlobal()->GetGamePacketGenerator()->SendCashItemHLShopStartReq();

	CDboEventGenerator::HLShopSelectCategory(eHLS_CATEGORY_WAGU_MACHINE);
}

VOID CWaguMachineSideIconGui::OnSideViewClosed()
{
}

void CWaguMachineSideIconGui::OnMouseEnter(gui::CComponent* pComponent)
{
	CSideIconGui::GetInstance()->OpenSideView(this, SIDEVIEW_WAGU, NULL);
}

void CWaguMachineSideIconGui::OnMouseLeave(gui::CComponent* pComponent)
{
	CSideIconGui::GetInstance()->CloseSideView(SIDEVIEW_WAGU);
}

void CWaguMachineSideIconGui::Show(bool bShow)
{
	__super::Show(bShow);
}
