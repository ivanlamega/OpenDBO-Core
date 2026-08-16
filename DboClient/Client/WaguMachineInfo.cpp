#include "precomp_dboclient.h"
#include "WaguMachineInfo.h"

// core
#include "NtlDebug.h"

// presentation
#include "NtlPLDef.h"
#include "NtlPLGui.h"
#include "NtlPLGuiManager.h"

#include "DisplayStringManager.h"
#include "DboEvent.h"
#include "NtlSLApi.h"
#include "InfoWndManager.h"
#include "DialogManager.h"
#include "TableContainer.h"

//table
#include "HLSItemTable.h"
#include "ItemTable.h"
#include "TextAllTable.h"

#include <sstream>
#include <iomanip>

#define sWAGU_MACHINE_INFO_MAX_SLOT		3

CWaguMachineInfoGui::CWaguMachineInfoGui(const RwChar* pName)
	:CNtlPLGui(pName)
{
	Init();
}

CWaguMachineInfoGui::~CWaguMachineInfoGui()
{

}

void CWaguMachineInfoGui::Init()
{

}

RwBool CWaguMachineInfoGui::Create()
{
	NTL_FUNCTION("CWaguMachineInfoGui::Create");

	if (!CNtlPLGui::Create("gui\\WaguMachineInfo.rsr", "gui\\WaguMachineInfo.srf", "gui\\WaguMachineInfo.frm"))
		NTL_RETURN(FALSE);

	CNtlPLGui::CreateComponents(GetNtlGuiManager()->GetGuiManager());

	// Get Component
	m_pThis = (gui::CDialog*)GetComponent("dlgMain");

	// Dialog Priority
	m_pThis->SetPriority(dDIALOGPRIORITY_HLSHOP);

	m_btnClose = (gui::CButton*)GetComponent("btnClose");
	m_pnlTitleMark = (gui::CPanel*)GetComponent("pnlTitleMark");

	m_stbTextTitle = (gui::CStaticBox*)GetComponent("stbTextTitle");
	m_stbTextTitle->SetText(GetDisplayStringManager()->GetString("DST_WAGU_ITEM_INFO_BUTTON"));
	m_stbMachineTitle = (gui::CStaticBox*)GetComponent("stbMachineTitle");


	std::ostringstream osStr;
	for (int i = 0; i < 10; i++) {
		osStr.str("");
		osStr << "stbProductList" << i;
		m_stbProductList[i] = (gui::CStaticBox*)GetComponent(osStr.str());
	}

	for (int i = 0; i < sWAGU_MACHINE_INFO_MAX_SLOT; i++)
	{
		osStr.str("");
		osStr << "pnlSlotItem" << i;
		m_pnlSlotItem[i] = (gui::CPanel*)GetComponent(osStr.str());
		m_pnlSlotItem[i]->Show(false);
		slotMouseEnterItem[i] = m_pnlSlotItem[i]->SigMouseEnter().Connect(this, &CWaguMachineInfoGui::OnMouseEnterWaguItem);
		slotMouseLeaveItem[i] = m_pnlSlotItem[i]->SigMouseLeave().Connect(this, &CWaguMachineInfoGui::OnMouseLeaveWaguItem);
		osStr.str("");
		osStr << "stbWinnerName" << i;
		m_stbWinnerName[i] = (gui::CStaticBox*)GetComponent(osStr.str());
		osStr.str("");
		osStr << "stbWinnerTry" << i;
		m_stbWinnerTry[i] = (gui::CStaticBox*)GetComponent(osStr.str());
		osStr.str("");
		osStr << "stbWinnerTime" << i;
		m_stbWinnerTime[i] = (gui::CStaticBox*)GetComponent(osStr.str());

		ItemSlot[i].Create(m_pnlSlotItem[i], DIALOG_HLSHOP, REGULAR_SLOT_ITEM_TABLE, SDS_COUNT);
		ItemSlot[i].SetSize(NTL_ITEM_ICON_SIZE);
		ItemSlot[i].SetPosition_fromParent(2, 2);
		ItemSlot[i].SetParentPosition(m_pnlSlotItem[i]->GetScreenRect().left, m_pnlSlotItem[i]->GetScreenRect().top);

		m_slotPaint[i] = m_pnlSlotItem[i]->SigPaint().Connect(this, &CWaguMachineInfoGui::OnPaint);
	}

	m_stbTextWinnerList = (gui::CStaticBox*)GetComponent("stbTextWinnerList");
	m_stbTextWinnerList->SetText(GetDisplayStringManager()->GetString("DST_WAGUWAGU_WINNER"));

	m_slogClose = m_btnClose->SigClicked().Connect(this, &CWaguMachineInfoGui::OnClickCloseBtn);

	m_slotMove = m_pThis->SigMove().Connect(this, &CWaguMachineInfoGui::OnMove);

	LinkMsg(g_EventHLShopEventWaguInfo);
	LinkMsg(g_EventWaguWinnerInfoRes);

	Show(false);

	NTL_RETURN(TRUE);
}

void CWaguMachineInfoGui::Destroy()
{
	UnLinkMsg(g_EventHLShopEventWaguInfo);
	UnLinkMsg(g_EventWaguWinnerInfoRes);

	CNtlPLGui::DestroyComponents();
	CNtlPLGui::Destroy();
}

RwInt32 CWaguMachineInfoGui::SwitchDialog(bool bOpen)
{
	Show(bOpen);

	NTL_RETURN(TRUE);
}

void CWaguMachineInfoGui::HandleEvents(RWS::CMsg& msg)
{
	if (msg.Id == g_EventHLShopEventWaguInfo)
	{
		SDboEventHLShopEventWaguInfo* pData = reinterpret_cast<SDboEventHLShopEventWaguInfo*>(msg.pData);

		m_MachineIndex = pData->index;

		CHLSItemTable* pHlsItemTable = API_GetTableContainer()->GetHLSItemTable();
		CItemTable* pItemTable = API_GetTableContainer()->GetItemTable();
		CTextTable* pText = API_GetTableContainer()->GetTextAllTable()->GetItemTbl();

		m_stbMachineTitle->SetText(pData->Name.c_str());

		for (int i = 0; i < 10; i++)
		{
			sHLS_ITEM_TBLDAT* pHlsItem = (sHLS_ITEM_TBLDAT*)pHlsItemTable->FindData(pData->pData[i]);
			if (!pHlsItem)
				continue;

			sITEM_TBLDAT* pItemData = (sITEM_TBLDAT*)pItemTable->FindData(pHlsItem->itemTblidx);
			if (!pItemData)
				continue;

			m_CurSlotItemTblidx[i] = pItemData->tblidx;
			WCHAR Buff[256];
			swprintf_s(Buff, 256, GetDisplayStringManager()->GetString("DST_WAGUWAGU_ITEMRANK"), i + 1, pText->GetText(pItemData->Name).c_str());
			m_stbProductList[i]->SetText(Buff);
		}


		GetDialogManager()->OpenDialog(DIALOG_HLSHOP_WAGU_INFO);
	}
	else if (msg.Id == g_EventWaguWinnerInfoRes)
	{
		SDboEventWaguWinnerInfo* pData = reinterpret_cast<SDboEventWaguWinnerInfo*>(msg.pData);

		if (pData->wMachineIndex == m_MachineIndex)
		{
			// show the winner list, most recent first
			int j = 0;
			for (int i = pData->byInfoCount - 1; i >= 0; i--)
			{
				std::stringstream time;
				time_t tExtractTime = (time_t)pData->nExtractTime[i];
				time << std::put_time(std::localtime(&tExtractTime), "%F %R");

				WCHAR buff1[256];
				swprintf_s(buff1, 256, GetDisplayStringManager()->GetString("DST_WAGUWAGU_INFOTRY"), (int)pData->nWinnerIndex[i]);
				m_stbWinnerTry[j]->SetText(buff1);

				WCHAR buff2[256];
				swprintf_s(buff2, 256, GetDisplayStringManager()->GetString("DST_WAGUWAGU_INFODEGREE"), pData->wWinCount[i], pData->wszPlayer[i]);
				m_stbWinnerName[j]->SetText(buff2);

				std::string strTime = time.str();
				std::wstring wstrTime(strTime.begin(), strTime.end());
				m_stbWinnerTime[j]->SetText(wstrTime.c_str());

				ItemSlot[j].SetIcon(m_CurSlotItemTblidx[0], 0);
				m_pnlSlotItem[j]->Show(true);
				j++;
			}

			// clear the remaining slots
			for (int i = 0; i < sWAGU_MACHINE_INFO_MAX_SLOT; i++)
			{
				if (!(pData->byInfoCount >= i + 1))
				{
					m_stbWinnerTry[i]->Clear();
					m_stbWinnerName[i]->Clear();
					m_stbWinnerTime[i]->Clear();
					ItemSlot[i].Clear();
					m_pnlSlotItem[i]->Show(false);
				}
			}

		}
	}
}

void CWaguMachineInfoGui::OnClickCloseBtn(gui::CComponent* pComponent)
{
	GetDialogManager()->CloseDialog(DIALOG_HLSHOP_WAGU_INFO);
}

void CWaguMachineInfoGui::OnPaint()
{
	for (int i = 0; i < sWAGU_MACHINE_INFO_MAX_SLOT; i++)
	{
		ItemSlot[i].Paint();
	}
}

void CWaguMachineInfoGui::OnMove(RwInt32 iOldX, RwInt32 iOldY)
{
	for (int i = 0; i < sWAGU_MACHINE_INFO_MAX_SLOT; i++)
	{
		CRectangle rect = m_pnlSlotItem[i]->GetScreenRect();
		ItemSlot[i].SetParentPosition(rect.left, rect.top);
	}
}

void CWaguMachineInfoGui::OnMouseEnterWaguItem(gui::CComponent* pComponent)
{
	for (int i = 0; i < sWAGU_MACHINE_INFO_MAX_SLOT; i++)
	{
		if (m_pnlSlotItem[i] == pComponent)
		{
			ShowItemInfoWindow(true, i);
		}
	}
}

void CWaguMachineInfoGui::OnMouseLeaveWaguItem(gui::CComponent* pComponent)
{
	ShowItemInfoWindow(false, 0);
}

void CWaguMachineInfoGui::ShowItemInfoWindow(RwBool isShow, BYTE i)
{
	if (isShow)
	{
		CRectangle rect = m_pnlSlotItem[i]->GetScreenRect();

		GetInfoWndManager()->ShowInfoWindow(TRUE, CInfoWndManager::INFOWND_TABLE_ITEM, rect.left, rect.top, ItemSlot[i].GetItemTable(), DIALOG_HLSHOP);
	}
	else
	{
		if (GetInfoWndManager()->GetRequestGui() == DIALOG_HLSHOP)
			GetInfoWndManager()->ShowInfoWindow(FALSE);
	}
}
