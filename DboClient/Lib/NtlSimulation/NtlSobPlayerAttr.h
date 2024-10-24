/*****************************************************************************
 *
 * File			: NtlSobPlayerAtt.h
 * Author		: HyungSuk, Jang
 * Copyright	: (��)NTL
 * Date			: 2005. 11. 30
 * Abstract		: Simulation object player attribute base class
 *****************************************************************************
 * Desc         :
 *
 *****************************************************************************/


#ifndef __NTL_SOBPLAYER_ATTR_H__
#define __NTL_SOBPLAYER_ATTR_H__

#include "NtlSharedDef.h"
#include "NtlSobAttr.h"


class CNtlSobPlayerAttr : public CNtlSobSocialAttr
{
	DECLEAR_MEMORY_POOL(CNtlSobPlayerAttr, NTL_DEFAULT_MEMORY_POOL)

public:

	CNtlSobPlayerAttr();

	virtual ~CNtlSobPlayerAttr() {}

	virtual RwBool Create(void) { SetClassID(SLCLASS_PLAYER); return TRUE; }

	virtual void Destroy(void) {}

	virtual void HandleEvents(RWS::CMsg& pMsg);


public:

	void		SetMainWeaponAttr(RwUInt8 bAttr) { m_ucMainWeaponBattleAttr = bAttr; }
	void		SetSubWeaponAttr(RwUInt8 bAttr) { m_ucSubWeaponBattleAttr = bAttr; }
	void		SetArmorWeaponAttr(RwUInt8 bAttr) { m_ucArmorBattleAttr = bAttr; }

	RwUInt8		GetMainWeaponAttr(void) { return m_ucMainWeaponBattleAttr; }
	RwUInt8		GetSubWeaponAttr(void) { return m_ucSubWeaponBattleAttr; }
	RwUInt8		GetArmorWeaponAttr(void) { return m_ucArmorBattleAttr; }

	void		SetPcTbl(const sPC_TBLDAT* pPcTbl);
	sPC_TBLDAT* GetPcTbl(void) const;


	void						SetInvisibleTitleNameFlag(bool bFlag);
	bool						GetInvisibleTitleNameFlag();

	void						SetTitleName(const WCHAR* pwcTitleName);
	const WCHAR* GetTitleName();

	void						SetIsInFreePvpZone(bool bFlag);
	bool						GetIsInFreePvpZone();

public:

	sPC_TBLDAT* m_pPcTbl;	/** pc table data pointer */

	RwUInt8		m_ucMainWeaponBattleAttr;
	RwUInt8		m_ucSubWeaponBattleAttr;
	RwUInt8		m_ucArmorBattleAttr;

	//////////////////////////////////////////////////////////////////////////

	bool			m_bInvisibleTitleNameFlag;
	std::wstring	m_wszTitleName;

	bool			m_bIsInFreePvpZone;
};

inline void CNtlSobPlayerAttr::SetPcTbl(const sPC_TBLDAT* pPcTbl)
{
	m_pPcTbl = const_cast<sPC_TBLDAT*>(pPcTbl);
}

inline sPC_TBLDAT* CNtlSobPlayerAttr::GetPcTbl(void) const
{
	return m_pPcTbl;
}

inline void CNtlSobPlayerAttr::SetInvisibleTitleNameFlag(bool bFlag)
{
	m_bInvisibleTitleNameFlag = bFlag;
}

inline bool CNtlSobPlayerAttr::GetInvisibleTitleNameFlag()
{
	return m_bInvisibleTitleNameFlag;
}

inline const WCHAR* CNtlSobPlayerAttr::GetTitleName()
{
	if (m_wszTitleName.length() == 0)
		return NULL;

	return m_wszTitleName.c_str();
}

inline void CNtlSobPlayerAttr::SetIsInFreePvpZone(bool bFlag)
{
	m_bIsInFreePvpZone = bFlag;
}

inline bool CNtlSobPlayerAttr::GetIsInFreePvpZone()
{
	return m_bIsInFreePvpZone;
}


#endif
