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
 * \brief HLS event slot machine side icon
 */
class CEventMachineSideIconGui : public CSideIconBase, public RWS::CEventHandler
{
public:
	CEventMachineSideIconGui(const RwChar* pName);
	virtual ~CEventMachineSideIconGui(void);

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
