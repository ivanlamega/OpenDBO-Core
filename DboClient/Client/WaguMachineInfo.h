#pragma once

#include "SlotGui.h"
#include "ceventhandler.h"

class CWaguMachineInfoGui : public CNtlPLGui, public RWS::CEventHandler
{
public:

	CWaguMachineInfoGui(const RwChar* pName);
	~CWaguMachineInfoGui();

	//! Operation
	void						Init();
	virtual RwBool				Create();
	virtual void				Destroy();

	RwInt32						SwitchDialog(bool bOpen);
	virtual void				HandleEvents(RWS::CMsg& msg);


protected:

	void				OnPaint();
	void				OnMove(RwInt32 iOldX, RwInt32 iOldY);
	void				OnClickCloseBtn(gui::CComponent* pComponent);
	void				OnMouseEnterWaguItem(gui::CComponent* pComponent);
	void				OnMouseLeaveWaguItem(gui::CComponent* pComponent);

	void				ShowItemInfoWindow(RwBool isShow, BYTE i);

protected:

	TBLIDX					m_MachineIndex;
	TBLIDX					m_CurSlotItemTblidx[10];

	gui::CSlot				m_slogClose;
	gui::CSlot				m_slotMove;
	gui::CSlot				m_slotPaint[3];
	gui::CSlot				slotMouseEnterItem[3];
	gui::CSlot				slotMouseLeaveItem[3];

	gui::CButton*			m_btnClose;
	gui::CPanel*			m_pnlTitleMark;
	gui::CStaticBox*		m_stbTextTitle;
	gui::CStaticBox*		m_stbMachineTitle;
	gui::CStaticBox*		m_stbProductList[10];
	gui::CStaticBox*		m_stbTextWinnerList;
	gui::CStaticBox*		m_stbWinnerName[3];
	gui::CStaticBox*		m_stbWinnerTry[3];
	gui::CStaticBox*		m_stbWinnerTime[3];
	gui::CPanel*			m_pnlSlotItem[3];
	CRegularSlotGui			ItemSlot[3];

};
