#pragma once

// core
#include "ceventhandler.h"
#include "NtlSLEvent.h"

// presentation
#include "NtlPLGui.h"

// dbo
#include "SideIconGui.h"

/**
 * \ingroup Client
 * \brief HLS slot machine (wagu/event) side icon
 */
class CWaguMachineSideIconGui : public CSideIconBase, public RWS::CEventHandler
{
public:
	CWaguMachineSideIconGui(const RwChar* pName);
	virtual ~CWaguMachineSideIconGui(void);

	RwBool			Create();
	VOID			Destroy();

	virtual VOID	OnIconButtonClicked(gui::CComponent* pComponent);
	virtual VOID	OnSideViewClosed();
	virtual void	Show(bool bShow);

protected:
	virtual VOID	HandleEvents(RWS::CMsg &msg);
	void            OnMouseEnter(gui::CComponent* pComponent);
	void            OnMouseLeave(gui::CComponent* pComponent);

protected:
	gui::CSlot      m_slotWaguBtn;
	gui::CSlot      m_slotWaguMouseEnter;
	gui::CSlot      m_slotWaguMouseLeave;
	gui::CButton*   m_pBtnWagu;
};
